#ifndef TIMESPECLIB_H
#define TIMESPECLIB_H

#define BILLION 1000000000

struct timespec divTimeSpecByInt(struct timespec dividend, int divisor);
void plusEqualsTimeSpecs(struct timespec* t1, struct timespec t2);
void minusEqualsTimeSpecs(struct timespec* t1, struct timespec* t2);
void addLongToTimespec(long l, struct timespec* t1);
int cmp_timespecs(struct timespec t1, struct timespec t2);
int isTimeZero(struct timespec t1);
struct timespec zeroTimeSpec();
struct timespec randTime(long min, long max)

#endif
