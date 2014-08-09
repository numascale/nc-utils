#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
  void *(*start_routine) (void *), void *arg)
{
	return 0;
}

pid_t fork(void)
{
	return 0;
}

pid_t vfork(void)
{
	return 0;
}

int clone(int (*fn)(void *), void *child_stack, int flags, void *arg, ...)
{
	return 0;
}
