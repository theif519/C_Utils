#include "Linked_List.h"
#include <Misc_Utils.h>

/// Static logger for all linked lists do use.
static MU_Logger_t *logger = NULL;

/* Below are declarations for a private sub-list for the merge sort algorithm, which may not be necessary, but it certainly
   makes it easier overall to visualize how to implement this. The sub-list just contains the head and tail nodes along with
   it's size, and there are basic private functions which belong to it as well. */

typedef struct {
	Node *head;
	Node *tail;
	size_t size;
} sub_list_t;

static sub_list_t *sub_list_of(sub_list_t *list, unsigned int begin, unsigned int end){
	sub_list_t *sub_list = malloc(sizeof(sub_list_t));
	int i = 0;
	Node *node = list->head;
	while(++i < begin) node = node->next;
	sub_list->head = node;
	while(++i < end) node = node->next;
	sub_list->tail = node;
	sub_list->size = i;
	return sub_list;
}

static void append_to_list(sub_list_t *list, Node *node){
	if(!list->size){
		list->tail = list->head = node;
		list->size++;
		return;
	}
	list->tail->next = node;
	node->prev = list->tail;
	list->tail = node;
	list->size++;
}

static sub_list_t *sub_list_create(Node *head, Node *tail, size_t size){
	sub_list_t *list = malloc(sizeof(sub_list_t));
	list->head = head;
	list->tail = tail;
	list->size = size;
	return list;
}

static void append_list_to_list(sub_list_t *list_src, sub_list_t *list_dst){
	Node *node = NULL;
	for(node = list_src->head; node; node = node->next){
		append_to_list(list_dst, node);
	}
}

static sub_list_t *merge_lists(sub_list_t *list_one, sub_list_t *list_two, Linked_List_Compare compare){
	sub_list_t *final_list = sub_list_create(NULL, NULL, 0);
	/* Assuming that it was properly sorted, then if the first element of the second list is greater than or equal
	   to the last element in the first list, then it is safe to just append everything and return. */
	if(compare(list_one->tail, list_two->head) <= 0) {
		append_list_to_list(list_one, final_list);
		append_list_to_list(list_two, final_list);
		return final_list;
	}
	while(list_one->size > 0 && list_two->size > 0){
		if(compare(list_one->head, list_two->head) <= 0){
			append_to_list(final_list, list_one->head);
			list_one->head = list_one->head->next;
			list_one->size--;
		} else {
			append_to_list(final_list, list_two->head);
			list_two->head = list_two->head->next;
			list_two->size--;
		}
	}
	if(list_one->size > 0) append_list_to_list(list_one, final_list);
	if(list_two->size > 0) append_list_to_list(list_two, final_list);
	return final_list;
}

static sub_list_t *sort_list(sub_list_t *list, Linked_List_Compare compare){
	if(list->size == 1) return list;
	size_t mid = list->size / 2;
	sub_list_t *list_one = sub_list_of(list, 0, mid);
	list_one = sort_list(list_one, compare);
	sub_list_t *list_two = sub_list_of(list, mid, list->size);
	list_two = sort_list(list_two, compare);
	sub_list_t *final_list = merge_lists(list_one, list_two, compare);
	free(list_one);
	free(list_two);
	return final_list;
}
/* Below are static private functions, or as I prefer to call them, helper functions, for the linked list. */


static int add_as_head(Linked_List *list, Node *node){
	node->next = list->head;
	list->head->prev = node;
	list->head = node;
	return 1;
}

static int add_as_tail(Linked_List *list, Node *node){
	list->tail->next = node;
	node->prev = list->tail;
	list->tail = node;
	return 1;
}

static int add_between(Linked_List *list, Node *previous_node, Node *current_node){
	previous_node->prev->next = current_node;
	current_node->next = previous_node;
	current_node->prev = previous_node->prev;
	previous_node->prev = current_node;
	return 1;
}

static int add_sorted(Linked_List *list, Node *node, Linked_List_Compare compare){
	return 0;
	if(!list->is_sorted) Linked_List_sort(list, compare);
	Node *current_node = NULL;
	if(list->size == 1) return compare(node->item, list->head->item) > 0 ? add_as_head(list, node) : add_as_tail(list, node);
	for(current_node = list->head; current_node; current_node = current_node->next){
		if(compare(node->item, current_node->item) > 0) return add_between(list, current_node, node);
	}
	MU_LOG_ERROR(logger, "Was unable to add an item, sortedly, to the list!\n");
	return 0;
}

static int add_unsorted(Linked_List *list, Node *node){
	node->next = NULL;
	node->prev = list->tail;
	list->tail->next = node;
	list->tail = node;
	list->is_sorted = 0;
	return 1;
}

/// Lock before calling this function!
static int node_exists(Linked_List *list, Node *node){
	Node *temp_node = NULL;
	for(temp_node = list->head; temp_node; temp_node = temp_node->next) {
		if(temp_node == node) {
			return 1;
		}
	}
	return 0;
}

/// Obtains lock inside of function!
static int node_to_index(Linked_List *list, Node *node){
	pthread_rwlock_rdlock(list->adding_or_removing_items);
	int index = 0;
	Node *temp_node = NULL;
	for(temp_node = list->head; temp_node; temp_node = temp_node->next){
		index++;
		if(node == temp_node) {
			pthread_rwlock_unlock(list->adding_or_removing_items);
			return index;
		}
	}
	pthread_rwlock_unlock(list->adding_or_removing_items);
	MU_LOG_WARNING(logger, "Node_To_Index failed as the node was not found!\n");
	return 0;
}

/// Obtains lock inside of function!
static Node *item_to_node(Linked_List *list, void *item){
	pthread_rwlock_rdlock(list->adding_or_removing_items);
	Node *node = NULL;
	for(node = list->head; node ; node = node->next) { 
		if(item == node->item) {
			pthread_rwlock_unlock(list->adding_or_removing_items);
			return node;
		}
	}
	MU_LOG_WARNING(logger, "Item_To_Node was unable to find the item in the list, returning NULL");
	pthread_rwlock_unlock(list->adding_or_removing_items);
	return NULL;
}

/// Obtains lock inside of function!
static Node *index_to_node(Linked_List *list, unsigned int index){
	pthread_rwlock_rdlock(list->adding_or_removing_items);
	if(index >= list->size) {
		pthread_rwlock_unlock(list->adding_or_removing_items);
		return NULL;
	}
	if(index == (list->size-1)) {
		pthread_rwlock_unlock(list->adding_or_removing_items);
		return list->tail;
	}
	if(index == 0) {
		pthread_rwlock_unlock(list->adding_or_removing_items);
		return list->head;
	}
	if(index > (list->size / 2)){
		int i = list->size-1;
		Node *node = list->tail;
		while((node = node->prev) && --i != index);
		pthread_rwlock_unlock(list->adding_or_removing_items);
		MU_ASSERT_RETURN(i == index, logger, NULL);
		return node;
	}
	int i = 0;
	Node *node = list->head;
	while((node = node->next) && ++i != index);
	pthread_rwlock_unlock(list->adding_or_removing_items);
	MU_ASSERT_RETURN(i == index, logger, NULL);
	return node;
}

/// Obtains lock inside of function!
static int for_each_item(Linked_List *list, void (*callback)(void *item)){
	pthread_rwlock_rdlock(list->adding_or_removing_items);
	Node *node = NULL;
	for(node = list->head; node; node = node->next) callback(node->item);
	pthread_rwlock_unlock(list->adding_or_removing_items);
	return 1;
}

/* Removes as if node is only one in list. */
static int remove_only(Linked_List *list, Node *node, Linked_List_Delete delete_item){
	list->head = NULL;
	list->tail = NULL;
	list->current = NULL;
	if(delete_item) delete_item(node->item);
	free(node);
	list->size = 0;
	return 1;
}

/* Removes as if node is the head one in the list. */
static int remove_head(Linked_List *list, Node *node, Linked_List_Delete delete_item){
	list->head->next->prev = NULL;
	list->head = list->head->next;
	if(list->current == node) list->current = list->head;
	if(delete_item) delete_item(node->item);
	free(node);
	list->size--;
	return 1;
}

/* Removes as if node is the tail one in the list. */
static int remove_tail(Linked_List *list, Node *node, Linked_List_Delete delete_item){
	list->tail = list->tail->prev;
	list->tail->next = NULL;
	if(list->current == node) list->current = list->tail;
	if(delete_item) delete_item(node->item);
	free(node);
	list->size--;
	return 1;
}

/* Removes as if, as is with the average case, this node is between two other nodes. */
static int remove_normal(Linked_List *list, Node *node, Linked_List_Delete delete_item){
	node->next->prev = node->prev;
	node->prev->next = node->next;
	if(delete_item) delete_item(node->item);
	if(list->current == node) list->current = node->next;
	free(node);
	list->size--;
	return 1;
}

static int remove_node(Linked_List *list, Node *node, Linked_List_Delete delete_item){
	MU_ASSERT_RETURN(node, logger, 0);
	pthread_rwlock_wrlock(list->adding_or_removing_items);
	if(!node_exists(list, node)) {
		MU_LOG_WARNING(logger, "Remove_Node failed to find the node in the list!\n");
		return 0;
	}
	int result = 0;
	if(list->size == 1) result = remove_only(list, node, delete_item);
	else if(list->tail == node) result = remove_tail(list, node, delete_item);
	else if(list->head == node) result = remove_head(list, node, delete_item);
	else result = remove_normal(list, node, delete_item);
	pthread_rwlock_unlock(list->adding_or_removing_items);
	return result;
}

/// Obtains lock inside of function!
static int delete_all_nodes(Linked_List *list, Linked_List_Delete delete_item){
	pthread_rwlock_wrlock(list->adding_or_removing_items);
	while(list->head){
		if(delete_item) delete_item(list->head->item);
		remove_node(list, list->head, delete_item);
	}
	pthread_rwlock_unlock(list->adding_or_removing_items);
}

/* End of private functions. */


Linked_List *Linked_List_create(void){
	if(!logger) logger = malloc(sizeof(MU_Logger_t));
	Linked_List *list = malloc(sizeof(Linked_List));
	if(!list) return 0;
	list->head = list->tail = list->current = NULL;
	list->size = 0;
	list->is_sorted = 1;
	list->adding_or_removing_items = malloc(sizeof(pthread_rwlock_t));
	if(!list->adding_or_removing_items){
		free(list);
		return 0;
	}
	pthread_rwlock_init(list->adding_or_removing_items, NULL);
	MU_Logger_Init(logger, "Linked_List_Log.txt", "w", MU_ALL);
	return list;
}

int Linked_List_add(Linked_List *list, void *item, Linked_List_Compare compare){
	if(!list) return 0;
	MU_ASSERT_RETURN(item, logger, 0);
	Node *new_node; 
	MU_ASSERT_RETURN(new_node = malloc(sizeof(Node)), logger, 0);
	new_node->item = item;
	pthread_rwlock_wrlock(list->adding_or_removing_items);
	if(list->size == 0){
		list->head = new_node;
		list->tail = new_node;
		new_node->next = NULL;
		new_node->prev = NULL;
		list->size++;
		pthread_rwlock_unlock(list->adding_or_removing_items);
		return 1;
	}
	int result = 0;
	if(compare) result = add_sorted(list, new_node, compare);
	else result = add_unsorted(list, new_node);
	if(result) list->size++;
	pthread_rwlock_unlock(list->adding_or_removing_items);
	return result;
}

void *Linked_List_get_at(Linked_List *list, unsigned int index){
	if(!list) return NULL;
	Node *node = NULL;
	return (node = index_to_node(list, index)) ? node->item : NULL;
}

int Linked_List_sort(Linked_List *list, Linked_List_Compare compare){
	if(!list || !compare) return 0;
	pthread_rwlock_wrlock(list->adding_or_removing_items);
	sub_list_t *sub_list = sub_list_create(list->head, list->tail, list->size);
	free(sort_list(sub_list, compare));
	free(sub_list);
	pthread_rwlock_unlock(list->adding_or_removing_items);
	return 1;
}

int Linked_List_remove_item(Linked_List *list, void *item, Linked_List_Delete delete_item){
	if(!list) return 0;
	return remove_node(list, item_to_node(list, item), delete_item); 
}

void *Linked_List_remove_at(Linked_List *list, unsigned int index, Linked_List_Delete delete_item){
	if(!list) return 0;
	Node *temp_node = index_to_node(list, index);
	void *item = NULL;
	if(temp_node){
		item = temp_node->item;
		remove_node(list, temp_node, delete_item);
	} else MU_LOG_WARNING(logger, "The node returned from Index_To_Node was NULL!\n");
	return item;
}

void *Linked_List_remove_current(Linked_List *list, Linked_List_Delete delete_item){
	if(!list || !list->current) return 0;
	void *item = list->current->item;
	remove_node(list, list->current, delete_item);
	return item;
}

void *Linked_List_next(Linked_List *list){
	if(!list || !list->current || !list->current->next) return NULL;
	return (list->current = list->current->next)->item;
}

void * Linked_List_previous(Linked_List *list){
	if(!list || !list->current || !list->current->prev) return NULL;
	return (list->current = list->current->prev)->item;
}

void * Linked_List_tail(Linked_List *list){
	if(!list || !list->tail) return NULL;
	return (list->current = list->tail)->item;
}

void * Linked_List_head(Linked_List *list){
	if(!list || !list->head) return NULL;
	return (list->current = list->head)->item;
}

void **Linked_List_To_Array(Linked_List *list){
	if(!list) return NULL;
	void **array_of_items = malloc(sizeof(void *) * list->size);
	MU_ASSERT_RETURN(array_of_items, logger, NULL);
	Node *node = NULL;
	pthread_rwlock_rdlock(list->adding_or_removing_items);
	int index = 0;
	for(node = list->head; node; node = node->next){
		array_of_items[index++] = node->item;
	}
	pthread_rwlock_unlock(list->adding_or_removing_items);
	return array_of_items;
}

void Linked_List_destroy(Linked_List *list, Linked_List_Delete delete_item){
	if(!list) return;
	delete_all_nodes(list, delete_item);
	pthread_rwlock_destroy(list->adding_or_removing_items);
	free(list->adding_or_removing_items);
	MU_Logger_Deref(logger,1);
	free(list);
}