#include <MU_Hazard_Pointers.h>
#include <MU_Arg_Check.h>
#include <stdlib.h>

typedef struct {
	MU_Hazard_Pointer_t *head;
	_Atomic size_t size;
	void (*destructor)(void *);
} MU_Hazard_Pointer_List_t;

MU_Hazard_Pointer_List_t *hazard_table = NULL;

static MU_Logger_t *logger = NULL;

static const int max_hazard_pointers = MU_HAZARD_POINTERS_MAX_THREADS * MU_HAZARD_POINTERS_PER_THREAD;

__attribute__((constructor)) static void init_logger(void){
	logger = MU_Logger_create("./Misc_Utils/Logs/MU_Hazard_Pointers.log", "w", MU_ALL);
}

__attribute__((constructor)) static void init_hazard_table(void){
	hazard_table = calloc(1, sizeof(*hazard_table));
	if(!hazard_table){
		MU_DEBUG("Was unable to allocate the hazard pointer table!");
		return;
	}
	hazard_table->destructor = free;
	hazard_table->size = ATOMIC_VAR_INIT(0);
}

__attribute__((destructor)) static void destroy_logger(void){
	MU_Logger_destroy(logger);
}

/*
	TODO: Major, please optimize this thing, I can't even tell it's complexity. I think
	O(N^3)? Regardless, it's awfully inefficient, and a hash table would at least make it O(N).
*/
__attribute__((destructor)) static void destroy_hazard_table(void){
	MU_Hazard_Pointer_t *prev_hp = NULL;
	DS_List_t *del_list = DS_List_create(false);
	for(MU_Hazard_Pointer_t *hp = hazard_table->head; hp; hp = hp->next){
		free(prev_hp);
		for(int i = 0; i < MU_HAZARD_POINTERS_PER_THREAD; i++){
			if(hp->owned[i] && !DS_List_contains(del_list, hp->owned[i])){
				DS_List_add(del_list, hp->owned[i], NULL);
			}
		}
		for(void *ptr = DS_List_head(del_list); ptr; ptr = DS_List_next(del_list)){
			if(!DS_List_contains(del_list, ptr)){
				DS_List_add(del_list, ptr, NULL);
			}
		}
		prev_hp = hp;
		DS_List_destroy(hp->retired, NULL);
	}
	free(prev_hp);
	DS_List_destroy(del_list, hazard_table->destructor);
}

static void scan(MU_Hazard_Pointer_t *hp){
	DS_List_t *private_list = DS_List_create(false);
	/*
		Loop through the hazard pointers owned by each thread. Add them to the private if they
		are non-NULL, hence in use.
	*/
	for(MU_Hazard_Pointer_t *tmp_hp = hazard_table->head; tmp_hp; tmp_hp = tmp_hp->next){
		for(int i = 0; i < MU_HAZARD_POINTERS_PER_THREAD; i++){
			if(tmp_hp->owned[i]){
				DS_List_add(private_list, tmp_hp->owned[i], NULL);
			}
		}
	}
	size_t arr_size = 0;
	void **tmp_arr = DS_List_to_array(hp->retired, &arr_size);
	DS_List_clear(hp->retired, NULL);
	/*
		Since each thread has it's own list of retired pointers, we check to see
		if this thread's "retired" is no longer in the list of hazard pointers, hence
		no longer in use and can be freed.
	*/
	for(int i = 0; i < arr_size; i++){
		if(DS_List_contains(private_list, tmp_arr[i])){
			DS_List_add(hp->retired, tmp_arr[i], NULL);
		} else {
			hazard_table->destructor(tmp_arr[i]);
		}
		free(tmp_arr);
	}
}

static void help_scan(MU_Hazard_Pointer_t *hp){
	for(MU_Hazard_Pointer_t *tmp_hp = hazard_table->head; tmp_hp; tmp_hp = tmp_hp->next){
		// If we fail to mark the hazard pointer as active, then it's already in use.
		bool expected = false;
		if(atomic_load(&tmp_hp->in_use) || atomic_compare_exchange_strong(&tmp_hp->in_use, &expected, true)) continue; 
		void *data;
		while((data = DS_List_remove_at(tmp_hp->retired, 0, NULL))){
			DS_List_add(hp->retired, data, NULL);
			if(hp->retired->size >= max_hazard_pointers) scan(hp);
		}
		atomic_store(&tmp_hp->in_use, false);
	}
}

static MU_Hazard_Pointer_t *create(){
	MU_Hazard_Pointer_t *hp = calloc(1, sizeof(*hp));
	if(!hp){
		MU_LOG_ASSERT(logger, "calloc: '%s'", strerror(errno));
		return NULL;
	}
	atomic_store(&hp->in_use, true);
	hp->retired = DS_List_create(false);
	if(!hp->retired){
		MU_LOG_ERROR(logger, "DS_List_create: '%s'", strerror(errno));
		free(hp);
		return NULL;
	}
	return hp;
}

MU_Hazard_Pointer_t *MU_Hazard_Pointer_acquire(void){
	for(MU_Hazard_Pointer_t *tmp_hp = hazard_table->head; tmp_hp; tmp_hp = tmp_hp->next){
		bool expected = false;
		if(atomic_load(&tmp_hp->in_use) || atomic_compare_exchange_strong(&tmp_hp->in_use, &expected, true)) continue;
		else return tmp_hp;
	}
	atomic_fetch_add(&hazard_table->size, MU_HAZARD_POINTERS_PER_THREAD);
	MU_Hazard_Pointer_t *hp = create();
	if(!hp){
		MU_LOG_ERROR(logger, "create_hp: 'Was unable to allocate a Hazard Pointer!");
		return NULL;
	}
	MU_Hazard_Pointer_t *old_head;
	do {
		old_head = hazard_table->head;
		hp->next = old_head;
	} while(!__sync_bool_compare_and_swap(&hazard_table->head, old_head, hp));
	return hp;
}

bool MU_Hazard_Pointer_reset(MU_Hazard_Pointer_t *hp){
	MU_ARG_CHECK(logger, false, hp);
	for(int i = 0; i < MU_HAZARD_POINTERS_PER_THREAD; i++){
		if(hp->owned[i]){
			DS_List_add(hp->retired, hp->owned[i], NULL);
			hp->owned[i] = NULL;
			if(hp->retired->size >= max_hazard_pointers){
				scan(hp);
				help_scan(hp);
			}
		}
	}
	return true;
}

bool MU_Hazard_Pointer_release(MU_Hazard_Pointer_t *hp){
	MU_ARG_CHECK(logger, false, hp);
	MU_Hazard_Pointer_reset(hp);
	atomic_store(&hp->in_use, false);
	return true;
}

bool MU_Hazard_Pointer_register_destructor(void (*destructor)(void *)){
	MU_ARG_CHECK(logger, false, destructor);
	hazard_table->destructor = destructor;
	return true;
}