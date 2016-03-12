/*This file contains a C program that drives threads*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "SortedList.h"

// #define _POSIX_C_SOURCE >= 199309L
int opt_yield;
int insert_yield, delete_yield, search_yield;
int sync_m; 
int sync_s;

int iterations;
int threads;

//argument to option --lists=#
//initialized as 0
int sublists;

//if no lists option then use a single list and set of locks
SortedList_t *list;
pthread_mutex_t lock;
volatile int lock_m;

list_struct_t **list_array;
SortedListElement_t **elements_array;


static char *rand_key(char *str, size_t size) {
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (size) {
		--size;
		size_t n;
		for (n = 0; n < size; n++) {
			int key = rand() % (int) (sizeof(charset) -1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
}

char *rand_key_wrapper(size_t size) {
	char *s = malloc(size+1);
	if (s) {
		rand_key(s,size);
	}
	return s;
}

void *wrapper(void *arg) {
	SortedList_t *target_list;
	pthread_mutex_t *target_lock;
	volatile int *target_lock_m;
	struct args_struct *args;
	args = arg;
	int offset = args->offset;

	SortedListElement_t **pointer = elements_array;
	
	int i;

	//add the list elements 
	for(i = offset; i < offset+iterations; ++i) {
		if (sublists == 0) {
			target_list = list;
			target_lock = &lock;
			target_lock_m = &lock_m;
		}
		else {
			//pick a list_struct by hasing the key of the list element we weant to add
			//grab the list and locks from the list_struct
			int hash = atoi(pointer[i]->key) % sublists;
			target_list = list_array[hash]->list;
			target_lock = &list_array[hash]->lock;
			target_lock_m = &list_array[hash]->lock_m;
		}

		if (sync_m)
			pthread_mutex_lock(target_lock);
		if (sync_s)
			while(__sync_lock_test_and_set(target_lock_m,1));

		SortedList_insert(target_list, pointer[i]);

		if (sync_m)
			pthread_mutex_unlock(target_lock);
		if (sync_s)	
			__sync_lock_release(target_lock_m);
		
	}
	//get the length
	int length = SortedList_length(target_list);
	// printf("length: %d\n",length);
	for (i = offset; i < offset+iterations;++i) {
		if (sublists == 0) {
			target_list = list;
			target_lock = &lock;
			target_lock_m = &lock_m;
		}
		else {
			int hash = atoi(pointer[i]->key) % sublists;
			target_list = list_array[hash]->list;
			target_lock = &list_array[hash]->lock;
			target_lock_m = &list_array[hash]->lock_m;
		}

		if (sync_m)
			pthread_mutex_lock(target_lock);
		if (sync_s)
			while(__sync_lock_test_and_set(target_lock_m,1));

		SortedListElement_t *temp = SortedList_lookup(target_list, pointer[i]->key);
		//if lookup found an element with the key in the list then delete it
		if (temp != NULL)
			SortedList_delete(temp);
		
		if (sync_m)
			pthread_mutex_unlock(target_lock);
		if (sync_s)	
			__sync_lock_release(target_lock_m);
	}	
	return (void*)arg;
}


int main(int argc, char *argv[]) 
{
	sync_m = 0;
	sync_s = 0;
	insert_yield = 0;
	delete_yield = 0;
	search_yield = 0;
	iterations = 1;
	threads = 1;
	sublists = 0;
	srand(time(0));


	while(1)
	{
		static struct option long_options[] =
		{
			{"threads",required_argument,0,'t'},
			{"iterations",required_argument,0,'i'},
			{"yield", required_argument,0,'y'},
			{"sync", required_argument,0,'s'},
			{"lists",required_argument,0,'l'},
			{0, 0, 0, 0}
		};
		int option_index;
		option_index = 0;
		int c;
		c= getopt_long(argc,argv, "", long_options, &option_index);
		if (c==-1)
			break;
		switch(c)
		{
			case 't':
				threads = atoi(optarg);
				break;

			case 'i':
				iterations = atoi(optarg);
				break;

			case 'l':
				sublists = atoi(optarg);
				break;
			case 'y':
				// if (strcmp(optarg,"1"))//im lazy for now
				opt_yield = 1;
				unsigned int j;
				for (j = 0; j < strlen(optarg); j++) {
					if (optarg[j] == 'i') {
						insert_yield = 1;
					}
					if (optarg[j] == 'd'){
						delete_yield = 1;
					}
					if (optarg[j] == 's'){
						search_yield = 1;
					}
				}
				break;

			case 's':
				if (strcmp(optarg, "m")==0) {
					sync_m = 1;
				}
				if (strcmp(optarg, "s")==0)
					sync_s = 1;
				break;



		}
	}

	//either initialize the single lists or an array of list_structs
	//each list_struct has a list and locks
	if (sublists == 0) {
		list = malloc(sizeof(SortedList_t));
		list->key = NULL;
		list->prev = list;
		list->next = list;
	}
	else {
		list_array = malloc(sizeof(list_struct_t*)*sublists);
		int i;
		for (i = 0; i < sublists;i++) {
			list_array[i] = malloc(sizeof(list_struct_t));
			list_array[i]->list = malloc(sizeof(SortedList_t));
			list_array[i]->list->key = NULL;
			list_array[i]->list->prev = list_array[i]->list;
			list_array[i]->list->next = list_array[i]->list;
		}
	}

	int i;
	elements_array = malloc(sizeof(SortedListElement_t*)*threads*iterations);
	for (i = 0; i < threads*iterations;++i) {
		elements_array[i] = malloc(sizeof(SortedListElement_t));
		elements_array[i]->key = rand_key_wrapper(10);
	}

	pthread_t tid[threads];
	struct timespec start,end;
	clock_gettime(CLOCK_MONOTONIC, &start);

		for (i=0;i <threads; i++) {
			struct args_struct *pass = malloc(sizeof(struct args_struct));
			pass->offset = i*iterations;
			pthread_create(&tid[i], NULL, wrapper, (void*)pass);
		}
		for (i=0;i<threads;i++) {
			pthread_join(tid[i],NULL);
		}

	clock_gettime(CLOCK_MONOTONIC,&end);
	double elapsed_time;
	elapsed_time = ((double)end.tv_sec*pow(10,9) + (double)end.tv_nsec) - 
					((double)start.tv_sec*pow(10,9) + (double)start.tv_nsec);

	int length = SortedList_length(list);
	for (i=0;i < threads*iterations;++i) {
		free(elements_array[i]);
	}

	char error[10];
	int exit_status;
	exit_status = 0;
	if (length != 0) {
		fprintf(stderr,"ERROR: length not 0\n");
		strcpy(error,"ERROR: ");
		exit_status = 1;
	}
	else 
		strcpy(error,"");
	printf("%d threads x %d iterations x (insert + delete) = %d operations\n",threads,iterations,threads*iterations*2);
	printf("%sfinal length = %d\n",error, length);
	printf("elapsed time: %f ns\n",elapsed_time);
	printf("per operation: %f ns\n",elapsed_time/(threads*iterations*2));
	free(elements_array);
	free(list);

	return 0;
}
