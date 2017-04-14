/* Written by Jamy Spencer 01 Apr 2017 */
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/types.h>
#include <sys/msg.h>
#include <time.h>
#include "obj.h"
#include <unistd.h>



int shrMemMakeAttach(int* shmid, consumer_t** a_table, struct timespec** clock){
	/* make the key: */
	int key[2] = {398290797, 398290798};

    /* connect to (and possibly create) the shared clock segment: */
    if ((shmid[0] = shmget(key[0], sizeof(struct timespec), IPC_CREAT | 0666)) == -1) {
        perror("shmget clock");
        return 1;
    }

    /* connect to (and possibly create) the resource segment: */
    if ((shmid[1] = shmget(key[1], sizeof(consumer_t)*20, IPC_CREAT | 0666)) == -1) {
        perror("shmget rsrc_table");
        return 1;
    }
    /* attach to the segment to get a pointer to it: */
    *clock = shmat(shmid[0], (void*) NULL, 0);
    if (clock == (void*)(-1)) {
        perror("shmat clock");
        return 1;
    }
    *a_table = shmat(shmid[1], (void*) NULL, 0);
    if (a_table == (void*)(-1)) {
        perror("shmat rsrc_table");
        return 1;
    }
	return 0;
}


long pwr(long n, long p){
	if (p == 0){return 1;}
	return pwr(n, p-1) * n;
}

int digit_quan(long num){
	int i;
	for (i = 0; i < 100; i++)	{
		if ((num - pwr(10, i)) < 0) return i;
	}
	return -1;
}

