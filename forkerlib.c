/* Written by Jamy Spencer 01 Apr 2017 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h> 
#include <time.h>
#include <unistd.h>
#include "forkerlib.h"
#include "obj.h"

struct list *MakeChild(struct list** head_ptr, struct timespec clock, char* arg){
	struct list* new_node;
	pid_t pid = fork();
	if (pid < 0){
		perror("Fork failed");
		return NULL;
	}
	else if (pid == 0){
		execl("./user", "user", arg, (char*) NULL);
	}
	else if (pid > 0){
		new_node = addNode(head_ptr);
		new_node->item.process_id = pid;
		new_node->item.t_zero = clock;

	}
	else{	
		perror("undefined behavior in MakeChild");
		return NULL;
	}
	return new_node;
}

struct list *addNode(struct list **head_ptr){
	struct list *temp = malloc (sizeof(struct list));

	temp->next = NULL;
	if ((*head_ptr) == NULL){
		temp->prev = NULL;
		(*head_ptr) = temp;
	}
	else{
		struct list *tail = returnTail(*head_ptr);
		tail->next = temp;
		temp->prev = tail;
	}
	return temp;
}

struct list *returnTail(struct list *head_ptr){
	struct list *temp = head_ptr;
	while(temp != NULL && temp->next){
		temp = temp->next;	
	}
	return temp;
}

struct list* findNodeByPid(struct list *head_ptr, pid_t pid){
	struct list *temp = head_ptr;
	while(temp != NULL){
		if (temp->item.process_id == pid){
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

//returns the head_ptr address of the list that now has the node containing the pid passed removed
struct list* destroyNode(struct list *head_ptr, pid_t pid, char* file_name){
	struct list *temp = findNodeByPid(head_ptr, pid);
	struct list *new_head;


	if (temp == NULL){
		printf("Couldn\'t find %d ", pid);
		return head_ptr;
	}
	else if (temp == head_ptr){//delete head
		if(temp){
			new_head = temp->next;//set new head to next
			if (new_head){//set the new heads prev to NULL
				new_head->prev = NULL;
			}
			free(temp);
			temp = NULL;		
		}
		else{perror("attempted to destroy null head");}
		return new_head;
	}
	else if (temp->next == NULL){//delete tail
		(temp->prev)->next = NULL;
		free(temp);
		return head_ptr;
	}
	else{//delete a mid

		(temp->prev)->next = temp->next;
		(temp->next)->prev = temp->prev;
		free(temp);
		return head_ptr;
	}
	return NULL;
}

void KillSlaves(struct list *head_ptr, char* file_name){
	while (head_ptr != NULL){	
		kill(head_ptr->item.process_id, SIGKILL);
		destroyNode(head_ptr, (head_ptr->item).process_id, file_name);	
	}
}
void log(char* file_name, char* str){
	FILE* file_write = fopen(file_name, "a");
	fprintf(file_write,"%s", str);
	fclose(file_write);
	return;
}
