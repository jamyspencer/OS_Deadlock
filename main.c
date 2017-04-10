/* Written by Jamy Spencer 23 Feb 2017 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/msg.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "forkerlib.h"
#include "obj.h"


void AbortProc();
void AlarmHandler();

int sem_key, sem_id;


int main ( int argc, char *argv[] ){

	struct stats mystats;

	mystats.child_count = 0;
	mystats.total_spawned = 0;

	file_name = "test.out";
	int c, i, j;

	int max_users = MAX_USERS;
	int percent_interrupt = CHANCE_OF_INTERRUPT;
	int max_run_time = 20;
	int quantum = QUANTUM;
	int max_overhead = MAX_OVERHEAD;



	struct timespec overhead;

	signal(2, AbortProc);
	signal(SIGALRM, AlarmHandler);
	user_list = NULL;

	while ( (c = getopt(argc, argv, "hi:l:m:o:q:t:")) != -1) {
		switch(c){
		case 'h':
			printf("-h\tHelp Menu\n");
			printf("-i\tChanges the chance that an interrupt will occur(default is 50)\n");
			printf("-l\tSet log file name(default is test.out)\n-m\tChanges the number of user processes(default is 18)\n");
			printf("-o\tChanges the maximum scheduling overhead in nanoseconds(default is 1000)\n");
			printf("-q\tChanges the base quantum used by processes in nanoseconds(default is 4000000)\n");
			printf("-t\tChanges the number of seconds to wait until the oss terminates all users and itself(default is 20)\n");
			return 0;
			break;
		case 'i':
			percent_interrupt = atoi(optarg);
			if (percent_interrupt > 100 || percent_interrupt < 0){
				printf("Error: Chance of interrupt must be between 0 and 100\n");
				exit(1);
			}
			break;
		case 'l':
			file_name = optarg;
			break;
		case 'm':
			max_users = atoi(optarg);
			if (max_users > MAX_USERS || max_users < 1){
				printf("Error: -s is out of acceptable range, set to %d\n", MAX_USERS);
				max_users = MAX_USERS;
			}
			break;
		case 'o':
			max_overhead = atoi(optarg);
			if (max_overhead < 0 || max_overhead > 5000000){
				printf("Error: max overhead must be between 0-50000000");
				exit(1);
			}
			break;
		case 'q':
			quantum = atoi(optarg);
			if (quantum < 0 || quantum > BILLION){
				printf("Error: quantum cannot be a negative number");
				exit(1);
			}
			break;
		case 't':
			max_run_time = atoi(optarg);
			if (max_run_time < 0){
				printf("Error: maximum run-time cannot be a negative number");
				exit(1);
			}
			break;
		case '?':
			return 1;
			break;
		}
	}
	alarm(max_run_time);

	//clear log file_name
	FILE* file_write = fopen(file_name, "w+");
	fclose(file_write);

	//init random numbers
	srand(time(0));

	//initialize semaphores
	int clk_sem_key, rsrc_sem_key;
	int clk_sem_id, rsrc_sem_id;
	initializeSemaphore(&clk_sem_key, &rsrc_sem_key, &clk_sem_id, &rsrc_sem_id);

	//initialize clock and array of resources in shared memory
	int shmid[2];
	struct timespec* clock;
	resource_t* resources;
	shrMemMakeAttach(shmid[], &resources, &clock);
	*clock = zeroTimeSpec();
	for (i = 0; i < 20; i++){
		for (j = 0; j < MAX_USERS; j++) (resources + i)->users_requesting[j] = 0;
		for (j = 0; j < MAX_USERS; j++) (resources + i)->users_granted[j] = 0;
	}
	int num_shared = (rand() % 2 + 1)+3;	
	for (i = 0; i < num_shared; i++){
		j = rand() % 10 + 1; //pick a number of instances of this resource
		(resources + i)->quan = j;
		(resources + i)->>num_available = j;

	}
	for (i; i < 20; i++){
		(resources + i)->quan = 1;//these resources have only one instance(unshareable)
		(resources + i)->>num_available = 1;
	}

	do{


		//Create new user if it is time.
		if ((cmp_timespecs(*clock, when_next_fork) >= 0) && mystats.total_spawned < 100 && clock->tv_sec < 2 && mystats.child_count < max_users){

			if (pcb_loc != -1){
				user_list = MakeChild(user_list, control_blocks + pcb_loc, pcb_loc, *clock);
				if (user_list == NULL){
					perror("MakeChild failed");
					AbortProc();			
				}
				mystats.total_spawned++;
				mystats.child_count++;
				SaveLog(file_name, (control_blocks + pcb_loc)->pid, *clock, 0, "create");
				addLongToTimespec(rand() % MAX_SPAWN_DELAY + 1, &when_next_fork);
			}
		}

		sem_wait(&clk_sem_id);

		sem_post(&clk_sem_id)
	}while(mystats.child_count > 0 || (clock->tv_sec < 2 && mystats.total_spawned < 100));

	LogStats(file_name, mystats);

	free(os_msg);
	free(unlock);
	free(user_msg);
	msgctl(messenger, IPC_RMID, NULL);
	shmdt(clock);
	shmdt(control_blocks);
	shmctl(shmid[0], IPC_RMID, NULL);
	shmctl(shmid[1], IPC_RMID, NULL);
	return 0;
}

void AlarmHandler(){
	perror("Time ran out");
	AbortProc();
}

void AbortProc(){


	kill(0, 2);
	msgctl(messenger, IPC_RMID, NULL);
	shmdt(control_blocks);
	shmdt(clock);
	shmctl(shmid[1], IPC_RMID, NULL);
	shmctl(shmid[0], IPC_RMID, NULL);
	msgctl(messenger, IPC_RMID, NULL);
	exit(1);
}


