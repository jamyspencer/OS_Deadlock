/* Written by Jamy Spencer 01 Apr 2017 */
#ifndef OBJ_H
#define OBJ_H

#include <semaphore.h>
#define MAX_USERS 18
#define DEFAULT_TIME_NEXT_RSRC_CHECK 1000
#define MAX_SPAWN_DELAY 200000
#define BILLION 1000000000
#define CHANCE_OF_TERMINATION 10
#define QUANTUM 4000000
#define FALSE 0
#define TRUE !FALSE

typedef struct consumer{
	pid_t pid;
	int rsrc_req[20];
	int rsrc_alloc[20];
}consumer_t;

typedef struct allocation_table{
	int rsrc_max[20];
	int rsrc_available[20];
}alloc_table_t;

typedef struct queue_msg{
	long int mtype;
	char mtext[1];
} msg_t;


int shrMemMakeAttach(int* shmid, consumer_t** a_table, struct timespec** clock);
long pwr(long n, long p);
int digit_quan(long num);

#endif
