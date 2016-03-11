/*This file is a C module holding implementations of functions in SortedList.h*/

void SortedList_insert(SortedList_t * list, SortedListElement_t * element) {
	SortedListElement_t *p = list;
	SortedListElement_t *n = list->next;
	while (n != list) {
		if (strcmp(element->key, n->key) < 0) //keep looping till you find an element greater than it
			break;
		n = n->next;
	}
	if (opt_yield && insert_yield) 
		pthread_yield();
	p=n->prev;
	element->prev=p;
	element->next=n;
	p->next=element;
	n->prev=element;
}

int SortedList_delete( SortedListElement_t *element) {

	SortedListElement_t *n = element->next;
	SortedListElement_t *p = element->prev;
	if (n->prev != element){
		return 1;
	}
	if (p->next != element){
		return 1;
	}

	if (opt_yield && delete_yield) 
		pthread_yield();
	n->prev = p;
	p->next = n;
	element->next = NULL;
	element->prev = NULL;

	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	SortedListElement_t *n = list->next;
	while (n != list) {
		if (opt_yield && search_yield) 
			pthread_yield();
		if (strcmp(n->key, key) == 0) {
			return n;
		}
		n = n->next;
	}
	return NULL;
}

int SortedList_length(SortedList_t *list) {
	int length = 0;
	SortedList_t * target_list;
	pthread_mutex_t *target_lock;
	volatile int *target_lock_m;
	if (sublists == 0) {
		target_list = list;
		target_lock = &lock;
		target_lock_m = &lock_m;

		if (sync_m)
			pthread_mutex_lock(target_lock);
		if (sync_s)
			while(__sync_lock_test_and_set(target_lock_m,1));

		SortedListElement_t *n = list->next;
		while (n != list) {
			if (opt_yield && search_yield)
				pthread_yield();
			SortedListElement_t * next = n->next;
			SortedListElement_t * prev = n->prev;
			length++;
			if (next->prev != n)
				return -1;
			if (prev->next != n)
				return -1;
			n = n->next;
		}

		if (sync_m)
			pthread_mutex_unlock(target_lock);
		if (sync_s)	
			__sync_lock_release(target_lock_m);

	}
	else {
		int i;
		for (i = 0;i < sublists;++i) {
			target_list = list_array[i]->list;
			target_lock = &list_array[i]->lock;
			target_lock_m = &list_array[i]->lock_m;

			if (sync_m)
				pthread_mutex_lock(target_lock);
			if (sync_s)
				while(__sync_lock_test_and_set(target_lock_m,1));

			SortedList_t * l = list_array[i]->list;
			SortedListElement_t * n = l->next;
			while (n != l) {
				if (opt_yield && search_yield)
					pthread_yield();
				SortedListElement_t * next = n->next;
				SortedListElement_t * prev = n->prev;
				length++;
				if (next->prev != n)
					return -1;
				if (prev->next != n)
					return -1;
				n = n->next;
			}

			if (sync_m)
				pthread_mutex_unlock(target_lock);
			if (sync_s)	
				__sync_lock_release(target_lock_m);
		}
	}
	return length;
}
