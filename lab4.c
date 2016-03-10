#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <string.h>

// #define _POSIX_C_SOURCE >= 199309L

static long long counter;
static int opt_yield;
static pthread_mutex_t lock;
static volatile int lock_m;
static int sync_m; 
static int sync_s;
static int sync_c;
static int iterations;
static int threads;


void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;
}

void add_sync_m(long long *pointer, long long value) {
	pthread_mutex_lock(&lock);
	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;
	pthread_mutex_unlock(&lock);
}

void add_sync_s(long long *pointer, long long value) {
	while (__sync_lock_test_and_set(&lock_m,1)) {
		continue;
	}
	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;
	__sync_lock_release(&lock_m);
}

void add_sync_c(long long *pointer, long long value) {

	// long long sum = *pointer + value;
	// if (opt_yield)
	// 	pthread_yield();
	// *pointer = sum;
	int orig;
	int sum;
	do {
		orig = *pointer; 
		sum = orig +value;
	} while(__sync_val_compare_and_swap(pointer,orig,sum) != orig);

}

/*no-sync wrapper*/
void *wrapper(void *arg) {
	int i;
	for (i = 0; i < iterations;++i) {
		add(&counter, 1);
	}
	for (i = 0; i < iterations;++i) {
		add(&counter, -1);
	}	
	return (void *)arg;
}

/*mutex wrapper*/
void *wrapper_sync_m(void *arg) {
	int i;
	for (i = 0; i < iterations;++i) {
		add_sync_m(&counter, 1);
	}
	for (i = 0; i < iterations;++i) {
		add_sync_m(&counter, -1);
	}	
	return (void *) arg;
}

/*spin-lock wrapper*/
void *wrapper_sync_s(void *arg) {
	int i;
	for (i = 0; i < iterations;++i) {
		add_sync_s(&counter, 1);
	}
	for (i = 0; i < iterations;++i) {
		add_sync_s(&counter, -1);
	}
	return (void *)arg;	
}

/*compare-and-swap wrapper*/
void *wrapper_sync_c(void *arg) {
	int i;
	for (i = 0; i < iterations; ++i) {
		add_sync_s(&counter, 1);
	}
	for (i = 0; i < iterations; ++i) {
		add_sync_s(&counter, -1);
	}
	return (void *)arg;	
}



int main(int argc, char *argv[]) 
{
	/*Initialize global flags and counters*/
	opt_yield = 0;
	iterations = 1;
	threads = 1;
	counter = 0;
	sync_m = 0;
	sync_s = 0;

	/*Parse input to addtest*/
	while(1) {
		static struct option long_options[] =
		{
			{"threads",		required_argument,	0,	't'},
			{"iterations", 	required_argument,	0,	'i'},
			{"yield", 		required_argument,	0,	'y'},
			{"sync", 		required_argument,	0,	's'},
			{0, 0, 0, 0}
		};
		int option_index;
		int c;

		option_index = 0;
		c = getopt_long(argc, argv, "", long_options, &option_index);
		if (c == -1)
			break;
		switch(c) {
			case 't':
				threads = atoi(optarg);
				break;

			case 'i':
				iterations = atoi(optarg);
				break;

			case 'y':
				opt_yield = atoi(optarg);
				break;

			case 's':
				if (strcmp(optarg, "m") == 0)
					sync_m = 1;
				if (strcmp(optarg, "s") == 0)
					sync_s = 1;
				if (strcmp(optarg, "c") == 0)
					sync_c = 1;
				break;
		}
	}

	/*Declare helper variable and thread storage*/
	int i;
	pthread_t tid[threads];

	/*Declare time helper struct and fetch time*/
	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	/*Initialize thread storage, run threads, and join threads based on synchronization type*/
	//goto results to skip over conditional statement overhead
	if (sync_m) {
		for (i = 0; i < threads; i++) {
			pthread_create(&tid[i], NULL, wrapper_sync_m, NULL);
		}
		for (i = 0; i < threads; i++) {
			pthread_join(tid[i], NULL);
		}
		goto results;
	}
	else if (sync_s) {
		for (i = 0;i < threads; i++) {
			pthread_create(&tid[i], NULL, wrapper_sync_s, NULL);
		}
		for (i = 0; i < threads; i++) {
			pthread_join(tid[i], NULL);
		}
		goto results;
	}
	else if (sync_c) {
		for (i = 0; i < threads; i++) {
			pthread_create(&tid[i], NULL, wrapper_sync_c, NULL);
		}
		for (i = 0;i < threads; i++) {
			pthread_join(tid[i], NULL);
		}
		goto results;
	}
	else {
		for (i = 0; i < threads; i++) {
			pthread_create(&tid[i], NULL, wrapper, NULL);
		}
		for (i = 0; i < threads; i++) {
			pthread_join(tid[i], NULL);
		}
		goto results;
	}

	/*Calculate timing results*/
	results:
	clock_gettime(CLOCK_REALTIME, &end);
	double elapsed_time;
	elapsed_time = ((double)end.tv_sec*pow(10,9) + (double)end.tv_nsec) - 
					((double)start.tv_sec*pow(10,9) + (double)start.tv_nsec);
	char error[10];
	int exit_status;
	exit_status = 0;
	if (counter != 0) {
		fprintf(stderr, "ERROR: counter not 0\n"); //May not be formatted the same as specification's suggestions
		strcpy(error, "ERROR: ");
		exit_status = 1;
	}
	else 
		strcpy(error,"");

	printf("%d threads x %d iterations x (add + subtract) = %d operations\n", threads, iterations, threads*iterations*2);
	printf("%sfinal count = %lld\n", error, counter);
	printf("elapsed time: %f ns\n", elapsed_time);
	printf("per operation: %f ns\n", elapsed_time/(threads*iterations*2));
	exit(exit_status);
}
