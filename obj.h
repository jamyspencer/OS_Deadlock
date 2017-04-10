/* Written by Jamy Spencer 23 Feb 2017 */
#ifndef OBJ_H
#define OBJ_H


#define MAX_USERS 18
#define MAX_SPAWN_DELAY 2000000
#define BILLION 1000000000
#define MAX_MSG_LEN 12
#define CHANCE_OF_TERMINATION 30
#define MAX_TOTAL_RUNTIME 100000000
#define QUANTUM 4000000
#define MAX_OVERHEAD 1000
#define GO 1
#define STOP 0

#ifndef SET_BIT
#define SET_BIT(var,pos)     ( var[(pos/32)] |= (1 << (pos%32)) )
#endif
#ifndef CLEAR_BIT 
#define CLEAR_BIT(var,pos)   ( var[(pos/32)] &= ~(1 << (pos%32)) )            
#endif
#ifndef CHECK_BIT
#define CHECK_BIT(var,pos) ( var[(pos/32)] >> (pos%32) & 1)
#endif

typedef struct resrc{
	int users_requesting[MAX_USERS];
	int users_granted[MAX_USERS];
	int quan;
	int num_available;
}resource_t;

typedef struct queue_msg{
	long int mtype;
	char mtext[1];
} msg_t;



void shrMemMakeAttach(int* shmid, pcb_t** cntl_blocks, struct timespec** clock);
void initializeSemaphore(int* clk_sem_key, int* rsrc_sem_key, int* clk_sem_id, int* rsrc_sem_id);
long pwr(long n, long p);
void log_mem_loc(pcb_t* addr, char* exec);

#endif
