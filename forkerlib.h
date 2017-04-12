/* Written by Jamy Spencer 01 Apr 2017 */
#ifndef FORKERLIB_H
#define FORKERLIB_H

#include "obj.h"

struct stats{
	struct timespec tot_user_wait;
	struct timespec tot_user_runtime;
	struct timespec tot_user_lifetime;
	struct timespec cpu_idle_time;
	struct timespec temp;	
	int child_count;	
	int total_spawned;
};

struct info{
	pid_t process_id;
	struct timespec t_zero;
	struct timespec t_final;
}; 

struct list{
	struct info item;
	struct list* next;
	struct list* prev;
};


struct list* PopProcess(struct list **queue_head);
struct list* PushProcess(struct list* queue_head, struct list* process);
struct list *MakeChild(struct list** head_ptr, struct timespec clock, char* arg);
struct list *addNode(struct list** head_ptr);
void KillSlaves(struct list *hd_ptr, char* file_name);
struct list *returnTail(struct list *head_ptr);
struct list* destroyNode(struct list *head_ptr, pid_t pid, char* file_name);
struct list* findNodeByPid(struct list *head_ptr, pid_t pid);
void log(char* file_name, char* str);

#endif
