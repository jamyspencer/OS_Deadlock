/* Written by Jamy Spencer 23 Feb 2017 */
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

#define MAX_TIME_NEXT_TERM_CHECK 2500

While the user processes are not actually doing anything, they will be asking for resources at random times.
You should have a parameter giving a bound for when a process should request/let go of a resource. 

Each process when it starts
should roll a random number from 0 to that bound and when it occurs it should try and either claim a new resource or release
a resource that it already has. It should make this request by putting a request in shared memory. It will continue looping and
checking to see if it is granted that resource.

At random times (between 0 and 250ms), the process checks if it should terminate successfully. If so, it should deallocate all
the resources allocated to it by communicating to master that it is releasing all those resources. Make sure that this probability
to terminate is low enough that churn happens in your system.


int main ( int argc, char *argv[] ){

	//handle args or shutdown with error msg
	if (argc < 2){
		perror("Error: User process received too few arguments");
		return 1;
	}else{
		int max_time_next_rsrc_op = atoi(argv[1]);
	}

	int doing_it = GO;
	struct timespec next_terminate_check = randTime(0, MAX_TIME_NEXT_TERM_CHECK);
	struct timespec next_resource_change = randTime(0, max_time_next_rsrc_op);

	//get shared memory
	int shmid[2];
	struct timespec* clock;
	resource_t* resources;
	shrMemMakeAttach(shmid[], &resources, &clock);

	//initialize semaphores
	int clk_sem_key, rsrc_sem_key;
	int clk_sem_id, rsrc_sem_id;
	initializeSemaphore(&clk_sem_key, &rsrc_sem_key, &clk_sem_id, &rsrc_sem_id);

	//initiallize rand generator
	srand(my_pid);



	while (doing_it){
	//Critical Section--------------------------------------------------------
		sem_wait(&clk_sem_id);
		if (cmp_timespecs(*clock, next_terminate_check) >= 0){
		sem_post(&clk_sem_id);
			if (is_terminating()){
			
			/* DO stuff here to shut down*/
				return 0;
			}
			else{
				plusEqualsTimeSpecs(&next_terminate_check, randTime(0, MAX_TIME_NEXT_TERM_CHECK));
			}
		}
		else if (cmp_timespecs(*clock, next_resource_change) >= 0){
		sem_post(&clk_sem_id);

			/*Do stuff here to request resources*/

		}
		else(//spin wait
		sem_post(&clk_sem_id);
		)
		



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

