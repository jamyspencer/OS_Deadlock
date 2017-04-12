/* Written by Jamy Spencer 01 Apr 2017 */
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "obj.h"
#include "timespeclib.h"

#define MAX_TIME_NEXT_TERM_CHECK 2500

int is_terminating ();

int main ( int argc, char *argv[] ){
	int max_time_next_rsrc_op;
	int i, r;
	pid_t my_pid= getpid();
	int my_vector;
	int check;
	int is_waiting = FALSE;
	int is_doing = TRUE;

	//handle args or shutdown with error msg
	if (argc < 2){
		perror("Error: User process received too few arguments");
		return 1;
	}else{
		max_time_next_rsrc_op = atoi(argv[1]);
	}

	struct timespec next_terminate_check = randTime(0, MAX_TIME_NEXT_TERM_CHECK);
	struct timespec next_resource_change = randTime(0, max_time_next_rsrc_op);

	//initialize semaphores
	sem_t *clk_sem = sem_open("/my_clock_name", 0);
	sem_t *rsrc_sem = sem_open("/my_resource_name", 0);

	//get shared memory
	int shmid[2];
	struct timespec* sys_clock;
	consumer_t* a_table;
	shrMemMakeAttach(shmid, &a_table, &sys_clock);
	//initiallize my_vector
	sem_wait(rsrc_sem);
	for (i = 0; i < MAX_USERS; i++){	//get the vector for this pid
		if (a_table[i].pid == 0) {
			my_vector = i;
			a_table[i].pid = my_pid;
		}
		break;
	}
	sem_post(rsrc_sem);

	//initiallize rand generator
	srand(my_pid);

	while (is_doing){
	//Critical Section--------------------------------------------------------
		sem_wait(clk_sem);
		check = cmp_timespecs(*sys_clock, next_terminate_check);
		sem_post(clk_sem);

		//check if waiting on resources
		is_waiting = FALSE;
		sem_post(rsrc_sem);
		for (i = 0; i < 20; i++){
			if (a_table[my_vector].rsrc_alloc[i] < a_table[my_vector].rsrc_req[i]){
				is_waiting = TRUE;
			}
		}
		sem_post(rsrc_sem);

		if (!is_waiting && check < 0) {
			plusEqualsTimeSpecs(&next_terminate_check, randTime(0, MAX_TIME_NEXT_TERM_CHECK));

			sem_wait(clk_sem);
			check = cmp_timespecs(*sys_clock, next_resource_change);
			sem_post(clk_sem);

			if (check >= 0) {
				r = rand() % 20; //which resource to request or release
				//request or release
				if ((rand() % 3) < 2) {//66% chance to request
					(a_table[my_vector].rsrc_req[r])++;
				} else {//release

					if (a_table[my_vector].rsrc_req[r] > 0) {
						(a_table[my_vector].rsrc_req[r])--;
					}
					else {
						for (i = 0; i < 20; i++){
							if (a_table[my_vector].rsrc_req[i] > 0) {
								(a_table[my_vector].rsrc_req[i])--;
							}
						}
					}
				}
			}
		}
		else if (is_terminating()) {
			is_doing = FALSE;
		}

	//End Critical Section---------------------------------------------------			
	}
	//set all resource requests to 0
	for (i = 0; i < 20; i++){
		a_table[my_vector].rsrc_req[i] = 0;
	}
	shmdt(a_table);
	shmdt(sys_clock);
	return 0;
}

int is_terminating (){
	int cmpr = rand() % 100 + 1;
	if (cmpr > CHANCE_OF_TERMINATION){
		return TRUE;
	}
	return FALSE;
}

/*
 * typedef struct consumer{
 * 	pid_t pid;
	int rsrc_req[20];
	int rsrc_alloc[20];
}user_t;

typedef struct allocation_table{
	user_t users[MAX_USERS];
	int rsrc_max[20];
	int rsrc_available[20];
}alloc_table_t;
 */