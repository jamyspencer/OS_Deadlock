#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int isTimeZero(struct timespec t1){
	if (t1.tv_sec == 0 && t1.tv_nsec == 0) return 1;
	return 0;
}

struct timespec zeroTimeSpec(){
	struct timespec temp;
	temp->tv_sec = 0;
	temp->tv_nsec = 0;
	return temp;
}

struct timespec randTime(long min, long max){
	struct timespec temp = zeroTimeSpec();
	srand(time(0));
	addLongToTimespec((rand() % max + min), &temp);
	return temp;
}

struct timespec divTimeSpecByInt(struct timespec dividend, int divisor){
	struct timespec result;

	result.tv_sec = (dividend.tv_sec) / divisor; 
	result.tv_nsec = (unsigned long) (((dividend.tv_sec) % divisor) * BILLION /divisor); 
	dividend.tv_nsec = dividend.tv_nsec / divisor;
	(result.tv_nsec) += (dividend.tv_nsec);
	if(result.tv_nsec >= BILLION){
		result.tv_nsec -= BILLION;
		(result.tv_sec)++;
	}
	return result;	
}

void plusEqualsTimeSpecs(struct timespec* t1, struct timespec t2){

	t1->tv_sec = t1->tv_sec + t2->tv_sec;
	t1->tv_nsec = t1->tv_nsec + t2->tv_nsec;

	if(t1->tv_nsec >= BILLION){
		t1->tv_nsec -= BILLION;
		(t1->tv_sec)++;
	}
	return;
}

void minusEqualsTimeSpecs(struct timespec* t1, struct timespec* t2){

	t1->tv_sec = t1->tv_sec - t2->tv_sec;
	t1->tv_nsec = t1->tv_nsec - t2->tv_nsec;

	if(t1->tv_nsec < 0){
		t1->tv_nsec += BILLION;
		(t1->tv_sec)--;
	}
	return;
}

void addLongToTimespec(long l, struct timespec* t1){
	t1->tv_nsec = t1->tv_nsec + l;

	if(t1->tv_nsec >= BILLION){
		t1->tv_nsec -= BILLION;
		(t1->tv_sec)++;
	}
	return;
}

int cmp_timespecs(struct timespec t1, struct timespec t2){
	if (t1.tv_sec > t2.tv_sec) return 1;
	else if (t1.tv_sec < t2.tv_sec) return -1;
	else if (t1.tv_nsec > t2.tv_nsec) return 1;
	else if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
} 
