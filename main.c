/* Written by Jamy Spencer 01 Apr 2017 */
#include <errno.h>
#include <semaphore.h>
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
#include <fcntl.h>
#include "forkerlib.h"
#include "obj.h"
#include "timespeclib.h"


void AbortProc();
void AlarmHandler();
void ReturnProcessResources(alloc_table_t sys_resources, pid_t pid);

static sem_t *clk_sem, *rsrc_sem;
static struct list* user_list;
static struct timespec* sys_clock;
static consumer_t* a_table;
static int shmid[2];

int main ( int argc, char *argv[] ){

	struct stats mystats;

	mystats.child_count = 0;
	mystats.total_spawned = 0;

	char* file_name = "test.out";
	int c, i, j, check;

	int max_users = 5;
	int max_time_next_rsrc_check = DEFAULT_TIME_NEXT_RSRC_CHECK;
	char* child_arg = (char*) malloc(digit_quan(DEFAULT_TIME_NEXT_RSRC_CHECK) + 1);
    char chr_buf[50];
	sprintf(child_arg, "%lu", DEFAULT_TIME_NEXT_RSRC_CHECK);
	int max_run_time = 20;
	int quantum = QUANTUM;
	int returning_child;
	struct timespec when_next_fork = zeroTimeSpec();


	signal(2, AbortProc);
	signal(SIGALRM, AlarmHandler);
	user_list = NULL;

	while ( (c = getopt(argc, argv, "hl:m:q:r:t:")) != -1) {
		switch(c){
		case 'h':
			printf("-h\tHelp Menu\n");
			printf("-i\tChanges the chance that an interrupt will occur(default is 50)\n");
			printf("-l\tSet log file name(default is test.out)\n-m\tChanges the number of consumer processes(default is 18)\n");
			printf("-o\tChanges the maximum scheduling overhead in nanoseconds(default is 1000)\n");
			printf("-q\tChanges the base quantum used by processes in nanoseconds(default is 4000000)\n");
			printf("-t\tChanges the number of seconds to wait until the oss terminates all users and itself(default is 20)\n");
			return 0;
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
		case 'q':
			quantum = atoi(optarg);
			if (quantum < 0 || quantum > BILLION){
				printf("Error: quantum cannot be a negative number");
				exit(1);
			}
			break;
		case 'r':
			max_time_next_rsrc_check = atoi(optarg);
			if (max_time_next_rsrc_check > 10000 || max_time_next_rsrc_check < 0){
				printf("Error: Time between resource checks must be between 0 and 10000\n");
				exit(1);
			}
			else{
				child_arg = optarg;
			}
			break;
		case 't':
			max_run_time = atoi(optarg);
			if (max_run_time < 0){
				printf("Error: maximum run-time cannot be a negative number\n");
				exit(1);
			}
			break;
		case '?':
			return 1;
			break;
		}
	}
	
	alarm(max_run_time);

	//clear log file
	FILE* file_write = fopen(file_name, "w+");
	fclose(file_write);

	//init random numbers
	srand(time(0));

	//initialize semaphores
	
	if((clk_sem = sem_open("/my_clock_name", O_CREAT, 0666, 1)) == (void*)-1){
		perror("clock semaphore creation failed");
	}
	if ((rsrc_sem = sem_open("/my_resource_name", O_CREAT, 0666, 1)) == (void*)-1){
		perror("resource semaphore creation failed");
	}


	//initialize user_clock and array of rsrc_table in shared memory
	shrMemMakeAttach(shmid, &a_table, &sys_clock);
	alloc_table_t rsrc_table;
	sys_clock->tv_sec = 0;
	sys_clock->tv_nsec = 0;
	for (i = 0; i < MAX_USERS; i++){
		a_table[i].pid = 0;
		for (j = 0; j < 20; j++){
			a_table[i].rsrc_req[j] = 1;//i=consumer j=resource -requested
			a_table[i].rsrc_alloc[j] = 1;//i=consumer j=resource -allocated
		}
	}
	int num_shared = (rand() % 2 + 1)+3;	
	for (i = 0; i < num_shared; i++){
		j = rand() % 10 + 1; //pick a number of instances of this resource
		rsrc_table.rsrc_max[i] = j;
		rsrc_table.rsrc_available[i] = j;
	}
	for (; i < 20; i++){
		rsrc_table.rsrc_max[i] = 1;
		rsrc_table.rsrc_available[i] = 1;
	}

	do{
		//advance clock
		sem_wait(clk_sem);
		*sys_clock = addLongToTimespec(rand() % 1200 + 100, sys_clock);
		check = cmp_timespecs(*sys_clock, when_next_fork);
 //       printf("MAIN: CLOCK: %02lu%09lu\n",user_clock->tv_sec, user_clock->tv_nsec);
		sem_post(clk_sem);

		//Create new user if it is time.
		if (check >= 0 && mystats.total_spawned < 100 && sys_clock->tv_sec < 2 && mystats.child_count < max_users){
			if (MakeChild(&user_list, *sys_clock, child_arg) == NULL){
				perror("MakeChild failed");
				AbortProc();			
			}
			mystats.total_spawned++;
			mystats.child_count++;
			addLongToTimespec(rand() % MAX_SPAWN_DELAY + 1, &when_next_fork);
		}
        //logging
        for (i = 0; i < 20; i++){
            sprintf(chr_buf, "RS:%2d AV:%d MX%d\n", i, rsrc_table.rsrc_available[i], rsrc_table.rsrc_max[i]);
            Log(file_name, chr_buf);
        }


            //Check resource requests and grant/reclaim them if possible
		for (i = 0; i < MAX_USERS; i++){
			for (j = 0; j < 20; j++){
				while (a_table[i].rsrc_req[j] > a_table[i].rsrc_alloc[j]){
					if (rsrc_table.rsrc_available[j] > 0){
						(a_table[i].rsrc_alloc[j])++;
						(rsrc_table.rsrc_available[j])--;
					}
					else break;
				}
				while (a_table[i].rsrc_req[j] < a_table[i].rsrc_alloc[j]){
					(a_table[i].rsrc_alloc[j])--;
					(rsrc_table.rsrc_available[j])++;
				}
			}
		}
	
		//Wait for returning users
		if ((returning_child = waitpid(-1, NULL, WNOHANG)) != 0){

			if (returning_child != -1){
				user_list = destroyNode(user_list, returning_child);
//				printf("Child %d returned/removed\n", returning_child);
				(mystats.child_count)--;
			}
	}

	}while(mystats.child_count > 0 || (sys_clock->tv_sec < 2 && mystats.total_spawned < 100));

    printf("exit conditions: child count:%d total:%d\n ", mystats.child_count, mystats.total_spawned);

	shmdt(sys_clock);
	shmdt(a_table);
	shmctl(shmid[0], IPC_RMID, NULL);
	shmctl(shmid[1], IPC_RMID, NULL);
	sem_unlink("/my_clock_name");
	sem_unlink("/my_resource_name");
	sem_close(clk_sem);
	sem_close(rsrc_sem);
	return 0;
}

void ReturnAllProcessResources(alloc_table_t sys_resources, pid_t pid){
	int i,j;
	for (i = 0; i < MAX_USERS; i++){
		sem_wait(rsrc_sem);
		if (a_table[i].pid == pid) {
			for (j = 0; j < 20; j++) {
				while (a_table[i].rsrc_alloc[j] > 0) {
					(a_table[i].rsrc_alloc[j])--;
					(sys_resources.rsrc_available[j])++;
				}
			}
			break;
		}
		sem_post(rsrc_sem);
	}
}

void AlarmHandler(){
	perror("Time ran out");
	AbortProc();
}

void AbortProc(){
	KillUsers(user_list);
	sem_unlink("/my_clock_name");
	sem_unlink("/my_resource_name");
	sem_close(clk_sem);
	sem_close(rsrc_sem);
	shmdt(a_table);
	shmdt(sys_clock);
	shmctl(shmid[1], IPC_RMID, NULL);
	shmctl(shmid[0], IPC_RMID, NULL);
	kill(0, 2);
	exit(1);
}