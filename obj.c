/* Written by Jamy Spencer 23 Feb 2017 */
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



void shrMemMakeAttach(int* shmid, pcb_t** resources, struct timespec** clock){
	/* make the key: */
	int key[2];

    if ((key[0] = ftok("main.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }
    if ((key[1] = ftok("slave.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }
    /* connect to (and possibly create) the shared clock segment: */
    if ((shmid[0] = shmget(key[0], sizeof(struct timespec), IPC_CREAT | 0644)) == -1) {
        perror("shmget");
        exit(1);
    }

    /* connect to (and possibly create) the resource segment: */
    if ((shmid[1] = shmget(key[0], sizeof(resource_t) * 20, IPC_CREAT | 0644)) == -1) {
        perror("shmget");
        exit(1);
    }
    /* attach to the segment to get a pointer to it: */
    *clock = shmat(shmid[0], (void*) NULL, 0);
    if (*clock == (void *)(-1)) {
        perror("shmat clock");
        exit(1);
    }
    *resources = shmat(shmid[1], (void*) NULL, 0);
    if (*resources == (void *)(-1)) {
        perror("shmat resources");
        exit(1);
    }
	return;
}

void initializeSemaphore(int* clk_sem_key, int* rsrc_sem_key, int* clk_sem_id, int* rsrc_sem_id){
    if ((*clk_sem_key = ftok("slave.c", 'C')) == -1) {
        perror("ftok");
        exit(1);
    }
    if ((*rsrc_sem_key = ftok("slave.c", 'D')) == -1) {
        perror("ftok");
        exit(1);
    }
	if ((*clk_sem_id = semget(*clk_sem_key, 0, IPC_CREAT|0666)) < 0) { 
		perror("Semaphore creation failed ");
	}
	if ((*rsrc_sem_id = semget(*rsrc_sem_key, 0, IPC_CREAT|0666)) < 0) { 
		perror("Semaphore creation failed ");
	}
}

long pwr(long n, long p){
	if (p == 0){return 1;}
	return pwr(n, p-1) * n;
}

void log_mem_loc(pcb_t* addr, char* exec){
	FILE* file_write = fopen("memlog.out", "a");
	fprintf(file_write, "PID:%6d PCB: %03d", addr->pid, addr->pcb_loc); 
	fprintf(file_write, " Memory  Address: %09x, Burst: %09lu, Executable: %s\n", &(*addr), addr->this_burst.tv_nsec, exec); 
	fclose(file_write);
	return;
}

