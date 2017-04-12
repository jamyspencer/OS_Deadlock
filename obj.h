/* Written by Jamy Spencer 01 Apr 2017 */
#ifndef OBJ_H
#define OBJ_H

#include <semaphore.h>
#define MAX_USERS 18
#define DEFAULT_TIME_NEXT_RSRC_CHECK 1000
#define MAX_SPAWN_DELAY 2000000
#define BILLION 1000000000
#define CHANCE_OF_TERMINATION 30
#define QUANTUM 4000000
#define MAX_OVERHEAD 1000
#define GO 1
#define STOP 0
#define FALSE 0
#define TRUE !FALSE

#ifndef SET_BIT
#define SET_BIT(var,pos)     ( var[(pos/32)] |= (1 << (pos%32)) )
#endif
#ifndef CLEAR_BIT 
#define CLEAR_BIT(var,pos)   ( var[(pos/32)] &= ~(1 << (pos%32)) )            
#endif
#ifndef CHECK_BIT
#define CHECK_BIT(var,pos) ( var[(pos/32)] >> (pos%32) & 1)
#endif

typedef struct user{
	int rsrc_req[20];
	int rsrc_alloc[20];
}user_t;

typedef struct allocation_table{
	user_t users[MAX_USERS];
	int rsrc_max[20];
	int rsrc_available[20]
}alloc_table_t;

typedef struct queue_msg{
	long int mtype;
	char mtext[1];
} msg_t;


void shrMemMakeAttach(int* shmid, alloc_table_t** a_table, struct timespec** clock);
void initializeSemaphore(sem_t* clk_sem, sem_t* rsrc_sem);
long pwr(long n, long p);
int digit_quan(long num);

#endif
