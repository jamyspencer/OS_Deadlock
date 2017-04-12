/* Written by Jamy Spencer 01 Apr 2017 */
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "obj.h"
#include "timespeclib.h"

#define MAX_TIME_NEXT_TERM_CHECK 2500

/*While the user processes are not actually doing anything, they will be asking for rsrc_table at random times.
You should have a parameter giving a bound for when a process should request/let go of a resource. 

Each process when it starts
should roll a random number from 0 to that bound and when it occurs it should try and either claim a new resource or release
a resource that it already has. It should make this request by putting a request in shared memory. It will continue looping and
checking to see if it is granted that resource.

At random times (between 0 and 250ms), the process checks if it should terminate successfully. If so, it should deallocate all
the rsrc_table allocated to it by communicating to master that it is releasing all those rsrc_table. Make sure that this probability
to terminate is low enough that churn happens in your system.
*/
int is_terminating ();

int main ( int argc, char *argv[] ){
	int max_time_next_rsrc_op;
	int i, r, success;

	pid_t my_pid= getpid();
	int check;
	//handle args or shutdown with error msg
	if (argc < 2){
		perror("Error: User process received too few arguments");
		return 1;
	}else{
		max_time_next_rsrc_op = atoi(argv[1]);
	}

	int doing_it = GO;
	struct timespec next_terminate_check = randTime(0, MAX_TIME_NEXT_TERM_CHECK);
	struct timespec next_resource_change = randTime(0, max_time_next_rsrc_op);

	//get shared memory
	int shmid[2];
	struct timespec* clock;
	struct alloc_table_t* a_table;
	shrMemMakeAttach(shmid, &a_table, &clock);
	for (i = 0; i < MAX_USERS; i++){
		if (a_table->)
	}

	//get the vector for this pid
	for (i = 0; i < MAX_USERS; i++){
		if (a_table->table[i])
	}

	//initialize semaphores
	sem_t *clk_sem = sem_open("/my_clock_name", 0);
	sem_t *rsrc_sem = sem_open("/my_resource_name", 0);

	//initiallize rand generator
	srand(my_pid);



	while (doing_it){
	//Critical Section--------------------------------------------------------
		sem_wait(clk_sem);
		check = cmp_timespecs(*clock, next_terminate_check);
		sem_post(clk_sem);
		if (check >= 0){
		
			if (is_terminating()){
			
			/* DO stuff here to shut down*/
				return 0;
			}
			else{
				plusEqualsTimeSpecs(&next_terminate_check, randTime(0, MAX_TIME_NEXT_TERM_CHECK));
			}
		}
		if (cmp_timespecs(*clock, next_resource_change) >= 0){
			//request or release
			if ((rand() % 2) == 0){//request
				success = 0;
				r = rand() % 20; //which resource to request
				for (i = 0; i < MAX_USERS; i++){
					if (a_table[r].users_requesting[i][0] == my_pid){
						(a_table[r].users_requesting[i][1])++;
						success = 1;
						break;
					}
				}
				if (!success){
					for (i = 0; i < MAX_USERS; i++){
						if (a_table[r].users_requesting[i][0] == 0){
							(a_table[r].users_requesting[i][1])++;
							success = 1;
							break;
						}
					}
				}

			}
			else{//release

			}

			/*Do stuff here to request rsrc_table*/

		}
		
		



	//End Critical Section---------------------------------------------------			
	}

	return 1;
}

void shut_down(){

}

int is_terminating (){
	int cmpr = rand() % 100 + 1;
	if (cmpr > CHANCE_OF_TERMINATION){
		return 1;//return true
	}
	return 0;//return false
}

