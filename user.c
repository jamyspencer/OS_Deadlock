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
#define IS_LOGGING 1

int is_terminating ();
static struct timespec* user_clock;
static consumer_t* user_table;

int main ( int argc, char *argv[] ){
	int max_time_next_rsrc_op;
	int i, r;
	pid_t my_pid= getpid();
	int my_vector;
	int check;
	int is_waiting = FALSE;
	int is_doing = TRUE;
	char chr_buf[30];

	//handle args or shutdown with error msg
	if (argc < 2){
		perror("Error: User process received too few arguments");
		return 1;
	}else{
		max_time_next_rsrc_op = atoi(argv[1]);
	}
    srand(my_pid);
    struct timespec next_terminate_check = randTime(0, MAX_TIME_NEXT_TERM_CHECK);
	struct timespec next_resource_change = randTime(0, max_time_next_rsrc_op);

	//initialize semaphores
	sem_t *clk_sem = sem_open("/my_clock_name", 0);
	sem_t *rsrc_sem = sem_open("/my_resource_name", 0);

sem_wait(rsrc_sem);
sem_wait(clk_sem);
    printf("time b4 attach:%02lu:%09lu\n", user_clock->tv_sec, user_clock->tv_nsec);
	//get shared memory
	int shmid[2];

	if (shrMemMakeAttach(shmid, &user_table, &user_clock) == -1){
        perror("Failed to attach to shared memory");
        return 1;
    }
    printf("time after attach:%02lu:%09lu\n", user_clock->tv_sec, user_clock->tv_nsec);
sem_post(rsrc_sem);
sem_post(clk_sem);
	//initiallize my_vector
	sem_wait(rsrc_sem);
	for (i = 0; i < MAX_USERS; i++){	//get the vector for this pid
		if (user_table[i].pid == 0) {
			my_vector = i;
            user_table[i].pid = my_pid;
		}
		break;
	}
	sem_post(rsrc_sem);


	while (is_doing){
	//Critical Section--------------------------------------------------------
		//check if waiting on resources
        printf("time on iterate:%02lu:%09lu\n", user_clock->tv_sec, user_clock->tv_nsec);

        is_waiting = FALSE;
		sem_post(rsrc_sem);
		for (i = 0; i < 20; i++){
			if (user_table[my_vector].rsrc_alloc[i] < user_table[my_vector].rsrc_req[i]){
				is_waiting = TRUE;
			}
			if(IS_LOGGING) {
				sprintf(chr_buf, "PID:%d RSRC:%2d RQ:%d AV:%d\n", my_pid, i, user_table[my_vector].rsrc_req[i],
                        user_table[my_vector].rsrc_alloc[i]);
				Log(chr_buf);
			}
		}
		sem_post(rsrc_sem);


//		printf("is_waiting:%d !is_waiting:%d check:%d\n", is_waiting, !is_waiting, check);

		if (!is_waiting) {
			sem_wait(clk_sem);
			check = cmp_timespecs(*user_clock, next_resource_change);
			sem_post(clk_sem);

			if (TRUE) {
 //               printf("clock-before:%02lu%09lu vs change:%02lu%09lu\n", user_clock->tv_sec, user_clock->tv_sec, next_resource_change.tv_sec, next_resource_change.tv_nsec);

                plusEqualsTimeSpecs(&next_resource_change, randTime(0, MAX_TIME_NEXT_TERM_CHECK));

				r = rand() % 20; //which resource to request or release
				//request or release
                sem_wait(rsrc_sem);
				if ((rand() % 3) < 2) {//66% chance to request

					(user_table[my_vector].rsrc_req[r])++;
				} else {//release

					if (user_table[my_vector].rsrc_req[r] > 0) {
						(user_table[my_vector].rsrc_req[r])--;
					}
					else {
						for (i = 0; i < 20; i++){
							if (user_table[my_vector].rsrc_req[i] > 0) {
								(user_table[my_vector].rsrc_req[i])--;
							}
						}
					}
				}
                sem_post(rsrc_sem);
			}

            sem_wait(clk_sem);
            check = cmp_timespecs(*user_clock, next_terminate_check);
            sem_post(clk_sem);
			if (is_terminating()) {
				is_doing = FALSE;
			}
            else{
                plusEqualsTimeSpecs(&next_terminate_check, randTime(0, MAX_TIME_NEXT_TERM_CHECK));
            }
		}


	//End Critical Section---------------------------------------------------			
	}
	//set all resource requests to 0
	for (i = 0; i < 20; i++){
        user_table[my_vector].rsrc_req[i] = 0;
	}
	shmdt(user_table);
	shmdt(user_clock);
	return 0;
}

int is_terminating (){
	int cmpr = rand() % 100 + 1;
	if (cmpr > CHANCE_OF_TERMINATION){
		return TRUE;
	}
	return FALSE;
}

void Log(char* str){
	FILE* file_write = fopen("userLog.out", "a");
	fprintf(file_write,"%s", str);
	fclose(file_write);
	return;
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