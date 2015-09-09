#include <DS_List.h>
#include <MU_Logger.h>
#include <MU_Arg_Check.h>
#include <MU_Cond_Locks.h>

/// Static logger for all linked lists do use.
static MU_Logger_t *logger = NULL;

__attribute__((constructor)) static void init_logger(void){
	logger = MU_Logger_create("./Data_Structures/Logs/DS_List.log", "w", MU_ALL);
}

__attribute__((destructor)) static void destroy_logger(void){
	MU_Logger_destroy(logger);
}
/* Begin implementations of helper functions. */

/* Helper functions used for adding nodes to the list. */

static int add_as_head(DS_List_t *list, DS_Node_t *node){
	node->_double.next = list->head;
	list->head->_double.prev = node;
	list->head = node;
	node->_double.prev = NULL;
	list->size++;
	return 1;
}

static int add_as_tail(DS_List_t *list, DS_Node_t *node){
	list->tail->_double.next = node;
	node->_double.prev = list->tail;
	list->tail = node;
	node->_double.next = NULL;
	list->size++;
	return 1;
}

#if 0
static int add_between(DS_List_t *list, DS_Node_t *previous_node, DS_Node_t *current_node){
	previous_node->_double.prev->_double.next = current_node;
	current_node->_double.next = previous_node;
	current_node->_double.prev = previous_node->_double.prev;
	previous_node->_double.prev = current_node;
	list->size++;
	return 1;
}

static int add_after(DS_List_t *list, DS_Node_t *current_node, DS_Node_t *new_node){
	current_node->_double.next->_double.prev = new_node;
	new_node->_double.next = current_node->_double.next;
	new_node->_double.prev = current_node;
	current_node->_double.next = new_node;
	list->size++;
	return 1;
}
#endif

static int add_as_only(DS_List_t *list, DS_Node_t *node){
	list->head = list->tail = node;
	node->_double.next = node->_double.prev = NULL;
	list->size++;
	return 1;
}

static int add_before(DS_List_t *list, DS_Node_t *current_node, DS_Node_t *new_node){
	current_node->_double.prev->_double.next = new_node;
	new_node->_double.next = current_node;
	new_node->_double.prev = current_node->_double.prev;
	current_node->_double.prev = new_node;
	list->size++;
	return 1;
}

static int add_sorted(DS_List_t *list, DS_Node_t *node, DS_comparator_cb compare){
	if(!list->is_sorted) DS_List_sort(list, compare);
	DS_Node_t *current_node = NULL;
	if(list->size == 1) return compare(node->item, list->head->item) < 0 ? add_as_head(list, node) : add_as_tail(list, node);
	if(compare(node->item, list->head->item) <= 0) return add_as_head(list, node);
	if(compare(node->item, list->tail->item) >= 0) return add_as_tail(list, node);
	for(current_node = list->head; current_node; current_node = current_node->_double.next){
		if(compare(node->item, current_node->item) <= 0) return add_before(list, current_node, node);
		else if(!current_node->_double.next) return add_as_tail(list, node);
	}
	MU_LOG_ERROR(logger, "Was unable to add an item, sortedly, to the list!\n");
	return 0;
}

static int add_unsorted(DS_List_t *list, DS_Node_t *node){
	node->_double.next = NULL;
	node->_double.prev = list->tail;
	list->tail->_double.next = node;
	list->tail = node;
	list->is_sorted = 0;
	list->size++;
	return 1;
}

/* Helper Functions for removing items and nodes from the list. */

static int remove_only(DS_List_t *list, DS_Node_t *node, DS_delete_cb del){
	list->head = NULL;
	list->tail = NULL;
	if(del) del(node->item);
	free(node);
	list->size--;
	return 1;
}

static int remove_head(DS_List_t *list, DS_Node_t *node, DS_delete_cb del){
	list->head->_double.next->_double.prev = NULL;
	list->head = list->head->_double.next;
	if(del) del(node->item);
	free(node);
	list->size--;
	return 1;
}

static int remove_tail(DS_List_t *list, DS_Node_t *node, DS_delete_cb del){
	list->tail = list->tail->_double.prev;
	list->tail->_double.next = NULL;
	if(del) del(node->item);
	free(node);
	list->size--;
	return 1;
}

static int remove_normal(DS_List_t *list, DS_Node_t *node, DS_delete_cb del){
	node->_double.next->_double.prev = node->_double.prev;
	node->_double.prev->_double.next = node->_double.next;
	if(del) del(node->item);
	free(node);
	list->size--;
	return 1;
}

static int remove_node(DS_List_t *list, DS_Node_t *node, DS_delete_cb del){
	int result = 0;
	if(list->size == 1) result = remove_only(list, node, del);
	else if(list->tail == node) result = remove_tail(list, node, del);
	else if(list->head == node) result = remove_head(list, node, del);
	else result = remove_normal(list, node, del);
	return result;
}

/* Helper functions for sorting the linked list */

static void swap_node_items(DS_Node_t *node_one, DS_Node_t *node_two){
	void *item = node_one->item;
	node_one->item = node_two->item;
	node_two->item = item;
}

static void insertion_sort_list(DS_List_t *list, DS_comparator_cb compare){
	DS_Node_t *node = NULL, *sub_node = NULL;
	for(node = list->head; node; node = node->_double.next){
		void *item = node->item;
		sub_node = node->_double.prev;
		while(sub_node && compare(sub_node->item, item) > 0){
			swap_node_items(sub_node->_double.next, sub_node);
			sub_node = sub_node->_double.prev;
		}
		if(sub_node) sub_node->_double.next->item = item;
	}
}

/* Helper functions for general linked list operations. */

static void print_list(DS_List_t *list, FILE *file, DS_to_string_cb to_string){
	DS_Node_t *node = NULL;
	char *all_items_in_list;
	asprintf(&all_items_in_list, "{ ");
	for(node = list->head; node ; node = node->_double.next) {
		char *item_as_string = to_string(node->item);
		char *old_items_in_list = all_items_in_list;
		asprintf(&all_items_in_list, "%s  %s,", all_items_in_list, item_as_string);
		free(old_items_in_list);
		free(item_as_string);
	}
	char *old_items_in_list = all_items_in_list;
	asprintf(&all_items_in_list, "%s } Size: %zu\n", all_items_in_list, list->size);
	fprintf(file, "%s\n", all_items_in_list);
	fflush(file);
	free(old_items_in_list);
	free(all_items_in_list);
}

static int delete_all_nodes(DS_List_t *list, DS_delete_cb del){
	while(list->head) remove_node(list, list->head, del);
	return 1;
}

#if 0
static int node_exists(DS_List_t *list, DS_Node_t *node){
	DS_Node_t *temp_node = NULL;
	for(temp_node = list->head; temp_node; temp_node = temp_node->_double.next) if(temp_node == node) return 1;
	return 0;
}

static int node_to_index(DS_List_t *list, DS_Node_t *node){
	int index = 0;
	DS_Node_t *temp_node = NULL;
	for(temp_node = list->head; temp_node; temp_node = temp_node->_double.next){
		index++;
		if(node == temp_node) {
			return index;
		}
	}
	MU_LOG_WARNING(logger, "Node_To_Index failed as the node was not found!\n");
	return 0;
}
#endif


static DS_Node_t *item_to_node(DS_List_t *list, void *item){
	if(list->head && list->head->item == item) return list->head;
	if(list->tail && list->tail->item == item) return list->tail;
	DS_Node_t *node = NULL;
	for(node = list->head; node ; node = node->_double.next) { 
		if(item == node->item) {
			return node;
		}
	}
	return NULL;
}

static DS_Node_t *index_to_node(DS_List_t *list, unsigned int index){
	if(index >= list->size) {
		return NULL;
	}
	if(index == (list->size-1)) {
		return list->tail;
	}
	if(index == 0) {
		return list->head;
	}
	if(index > (list->size / 2)){
		int i = list->size-1;
		DS_Node_t *node = list->tail;
		while((node = node->_double.prev) && --i != index);
		MU_ASSERT(i == index, logger, "Error in Node Traversal!Expected index %u, stopped at index %d!", index, i);
		return node;
	}
	int i = 0;
	DS_Node_t *node = list->head;
	while((node = node->_double.next) && ++i != index);
	MU_ASSERT(i == index, logger, "Error in Node Traversal!Expected index %u, stopped at index %d!", index, i);
	return node;
}

static void for_each_item(DS_List_t *list, void (*callback)(void *item)){
	DS_Node_t *node = NULL;
	for(node = list->head; node; node = node->_double.next) callback(node->item);
}

/* End implementations of helper functions. */

/* Implementation of DS_Iterator callbacks */

static DS_Node_t *next(void *instance, DS_Node_t *curr, void **item_ptr){
	DS_List_t *list = instance;
	DS_Node_t *next;
	MU_COND_RWLOCK_RDLOCK(list->rwlock, logger);
	if(list->size == 0){
		MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
		*item_ptr = NULL;
		return NULL;
	}
	if(!curr){
		next = list->head;
		*item_ptr = next->item;
		MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
		return next;
	}
	/*
		If the the list is not empty and if the current node passed is not NULL, we must check if it
		is valid, since the list easily could have changed since the last iterator operation. This involves
		checking to see if it is found in the list and whether or not the previous or next nodes are valid,
		O(N). The previous and next nodes are checked because in case it is not valid, we'd rather not have
		to go back to the beginning, so instead we assign the current to be the next, if not that then the previous.
	*/
	bool next_valid = false, prev_valid = false, curr_valid = false;
	DS_Node_t *node;
	for(node = list->head; node; node = node->_double.next){
		if(node == curr){
			curr_valid = true;
			break;
		}
		if(node == curr->_double.next){
			next_valid = true;
			break;
		}
		if(node == curr->_double.prev){
			prev_valid = true;
		}
	}
	if(curr_valid || next_valid){
		next = curr->_double.next;
	} else if(prev_valid){
		next = curr->_double.prev;
		if(next){
			next = next->_double.next;
		}
	} else {
		next = list->head;
	}
	*item_ptr = next ? next->item : NULL;
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return next;
}

static DS_Node_t *prev(void *instance, DS_Node_t *curr, void **item_ptr){
	DS_List_t *list = instance;
	DS_Node_t *prev;
	MU_COND_RWLOCK_RDLOCK(list->rwlock, logger);
	if(list->size == 0){
		MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
		*item_ptr = NULL;
		return NULL;
	}
	if(!curr){
		prev = list->tail;
		*item_ptr = prev->item;
		MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
		return prev;
	}
	/*
		If the the list is not empty and if the current node passed is not NULL, we must check if it
		is valid, since the list easily could have changed since the last iterator operation. This involves
		checking to see if it is found in the list and whether or not the previous or next nodes are valid,
		O(N). The previous and next nodes are checked because in case it is not valid, we'd rather not have
		to go back to the beginning, so instead we assign the current to be the next, if not that then the previous.
	*/
	bool next_valid = false, prev_valid = false, curr_valid = false;
	DS_Node_t *node;
	for(node = list->tail; node; node = node->_double.prev){
		if(node == curr){
			curr_valid = true;
			break;
		}
		if(node == curr->_double.next){
			next_valid = true;
		}
		if(node == curr->_double.prev){
			prev_valid = true;
			break;
		}
	}
	if(curr_valid || prev_valid){
		prev = curr->_double.prev;
	} else if(next_valid){
		prev = curr->_double.next;
		if(prev){
			prev = prev->_double.next;
		}
	} else {
		prev = list->head;
	}
	*item_ptr = prev ? prev->item : NULL;
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return prev;
}

static DS_Node_t *append(void *instance, DS_Node_t *curr, void *item, bool *result){
	DS_List_t *list = instance;
	DS_Node_t *node = malloc(sizeof(*node));
	if(!node){
		MU_LOG_ASSERT(logger, "malloc: '%s'", strerror(errno));
		*result = false;
		return curr;
	}
	node->item = item;
	MU_COND_RWLOCK_WRLOCK(list->rwlock, logger);
	if(list->size == 0){
		add_as_only(list, node);
		*result = true;
		return node; 
	}
	// If current is NULL, we append it to the end.
	if(!curr){
		add_as_tail(list, node);
		*result = true;
		return node;
	}
	DS_Node_t *tmp_node;
	for(tmp_node = list->head; tmp_node; tmp_node = tmp_node->_double.next){
		// TODO
	}
}

/* Linked List Creation and Deletion functions */

DS_List_t *DS_List_create(bool synchronized){
	DS_List_t *list = calloc(1, sizeof(DS_List_t));
	if(!list) {
		MU_DEBUG("See Log!!!\n");
		MU_LOG_ERROR(logger, "Was unable to allocate list, Out of Memory\n");
		return NULL;
	}
	list->head = list->tail = NULL;
	list->size = 0;
	list->is_sorted = 1;
	if(synchronized){
		list->rwlock = malloc(sizeof(pthread_rwlock_t));
		if(!list->rwlock){
			MU_DEBUG("See Log!!!\n");
			free(list);
			MU_LOG_ERROR(logger, "Unable to allocate rwlock, Out of Memory!\n");
			return NULL;
		}
		pthread_rwlock_init(list->rwlock, NULL);
	}
	return list;
}

DS_List_t *DS_List_create_from(void **array, size_t size, DS_comparator_cb compare, bool synchronized){
	MU_ARG_CHECK(logger, NULL, array);
	DS_List_t *list = DS_List_create(synchronized);
	int i = 0;
	for(;i<size;i++) DS_List_add(list, array[i], compare);
	return list;
}

bool DS_List_clear(DS_List_t *list, DS_delete_cb del){
	MU_ARG_CHECK(logger, false, list);
	MU_COND_RWLOCK_WRLOCK(list->rwlock, logger);
	delete_all_nodes(list, del);
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return true;
}

bool DS_List_destroy(DS_List_t *list, DS_delete_cb del){
	MU_ARG_CHECK(logger, false, list);
	DS_List_clear(list, del);
	MU_COND_RWLOCK_DESTROY(list->rwlock, logger);
	free(list);
	return true;
}

/* Linked List adding functions */

bool DS_List_add(DS_List_t *list, void *item, DS_comparator_cb compare){
	MU_ARG_CHECK(logger, false, list, item);
	DS_Node_t *node = malloc(sizeof(DS_Node_t)); 
	if(!node){
		MU_LOG_ASSERT(logger, "malloc: '%s'", strerror(errno));
		return false;
	}
	node->item = item;
	MU_COND_RWLOCK_WRLOCK(list->rwlock, logger);
	if(!list->size){
		add_as_only(list, node);
		MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
		return true;
	}
	bool result = false;
	if(compare) result = add_sorted(list, node, compare);
	else result = add_unsorted(list, node);
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return result;
}

bool DS_List_add_after(DS_List_t *list, void *item){
	MU_ARG_CHECK(logger, false, list, item);
	MU_COND_RWLOCK_WRLOCK(list->rwlock, logger);
	DS_Node_t *node = malloc(sizeof(DS_Node_t));
	node->item = item;
	if(!list->size){
		add_as_only(list, node);
		MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
		return 1;
	}
	list->is_sorted = 0;
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return 1;
}

bool DS_List_add_before(DS_List_t *list, void *item){
	MU_ARG_CHECK(logger, false, list, item);
	MU_COND_RWLOCK_WRLOCK(list->rwlock, logger);
	DS_Node_t *node = malloc(sizeof(DS_Node_t));
	node->item = item;
	if(!list->size){
		add_as_only(list, node);
		MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
		return 1;
	}
	list->is_sorted = 0;
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return 1;
}

/* Linked List removal functions */

bool DS_List_remove_item(DS_List_t *list, void *item, DS_delete_cb del){
	MU_ARG_CHECK(logger, false, list, item);
	MU_COND_RWLOCK_WRLOCK(list->rwlock, logger);
	DS_Node_t *node = item_to_node(list, item);
	int result = 0;
	if(node) result = remove_node(list, node, del);
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return result; 
}

void *DS_List_remove_at(DS_List_t *list, unsigned int index, DS_delete_cb del){
	MU_ARG_CHECK(logger, NULL, list);
	MU_COND_RWLOCK_WRLOCK(list->rwlock, logger);
	DS_Node_t *temp_node = index_to_node(list, index);
	void *item = NULL;
	if(temp_node){
		item = temp_node->item;
		remove_node(list, temp_node, del);
	} else MU_LOG_WARNING(logger, "The node returned from Index_To_Node was NULL!\n");
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return item;
}

/* Linked List miscallaneous functions */

bool DS_List_for_each(DS_List_t *list, void (*callback)(void *item)){
	MU_ARG_CHECK(logger, false, list, callback);
	MU_COND_RWLOCK_RDLOCK(list->rwlock, logger);
	for_each_item(list, callback);
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return true;
}

bool DS_List_sort(DS_List_t *list, DS_comparator_cb compare){
	MU_ARG_CHECK(logger, false, list, compare);
	MU_COND_RWLOCK_WRLOCK(list->rwlock, logger);
	insertion_sort_list(list, compare);
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return true;
}

bool DS_List_contains(DS_List_t *list, void *item){
	MU_ARG_CHECK(logger, false, list, item);
	MU_COND_RWLOCK_RDLOCK(list->rwlock, logger);
	DS_Node_t *node = item_to_node(list, item);
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return node != NULL;
}

bool DS_List_print_all(DS_List_t *list, FILE *file, DS_to_string_cb to_string){
	MU_ARG_CHECK(logger, false, list, file, to_string);
	MU_COND_RWLOCK_RDLOCK(list->rwlock, logger);
	print_list(list, file, to_string);
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return true;
}

void *DS_List_get_at(DS_List_t *list, unsigned int index){
	MU_ARG_CHECK(logger, NULL, list);
	MU_COND_RWLOCK_RDLOCK(list->rwlock, logger);
	DS_Node_t *node = index_to_node(list, index);
	void *item = NULL;
	if(node) item = node->item;
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return item;
}

void **DS_List_to_array(DS_List_t *list, size_t *size){
	MU_ARG_CHECK(logger, NULL, list);
	MU_COND_RWLOCK_RDLOCK(list->rwlock, logger);
	void **array_of_items = malloc(sizeof(void *) * list->size);
	if(!array_of_items){
		MU_LOG_ASSERT(logger, "malloc: '%s'", strerror(errno));
		return NULL;
	}
	DS_Node_t *node = NULL;
	int index = 0;
	for(node = list->head; node; node = node->_double.next){
		array_of_items[index++] = node->item;
	}
	*size = index;
	MU_COND_RWLOCK_UNLOCK(list->rwlock, logger);
	return array_of_items;
}


DS_Iterator_t *DS_List_iterator(DS_List_t *list){
	DS_Iterator_t *it = calloc(1, sizeof(*it));
	if(!it){
		MU_LOG_ASSERT(logger, "calloc: '%s'", strerror(errno));
		goto error;
	}
	it->ds_handle = list;
	it->next = next;
	it->prev = prev;
	return it;

	error:
		free(it);
		return NULL;
}