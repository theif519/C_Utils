#include <DS_Stack.h>
#include <MU_Logger.h>
#include <TP_Pool.h>
#include <unistd.h>

static const int num_threads = 4;

static DS_Stack_t *stack = NULL;

static MU_Logger_t *logger = NULL;

volatile int counter = 0;

volatile bool running = true;

static void *push_to_stack(void *args){
	while(running){
		int *i = malloc(sizeof(int));
		*i = __sync_add_and_fetch(&counter, 1);
		DS_Stack_push(stack, i);
		//pthread_yield();
	}
	return NULL;
}

static void *pop_from_stack(void *args){
	while(running){
		int *i = DS_Stack_pop(stack);
		if(!i){
			//pthread_yield();
			continue;
		}
		MU_DEBUG("Popped Val: %d", *(int *)i);
		free(i);
	}
	return NULL;
}

int main(void){
	logger = MU_Logger_create("./Data_Structures/Logs/DS_Stack_Test.log", "w", MU_ALL);
	stack = DS_Stack_create();
	TP_Pool_t *tp = TP_Pool_create(num_threads);
	int i = 0;
	for(; i < num_threads; i++){
		TP_Pool_add(tp, (i % 2 == 0) ? push_to_stack : pop_from_stack, NULL, TP_NO_RESULT);
	}
	TP_Pool_wait(tp, 300);
	running = 0;
	TP_Pool_wait(tp, -1);
	TP_Pool_destroy(tp);
	MU_Logger_destroy(logger);
	DS_Stack_destroy(stack, free);
	return 0;
}