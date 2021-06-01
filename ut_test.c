#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "ut.h"


ut_time_t utTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	
    unsigned long long v;
	
    v=ts.tv_sec;
    v*=1000;
    v+=ts.tv_nsec/1000000;
	
    return (ut_time_t)(v & 0xffffffff);
}


ut_sem_t utWakeFlags(void)
{
	return 0;
}


void utDoSleep(ut_time_t T)
{
	printf("utDoSleep(%d)\n", T);
	usleep(T*1000);
}


void utDoWait(ut_time_t T)
{
	printf("utDoWait(%d)\n", T);
	usleep(T*1000);
}


UT(my_sub, int a, int b)
{
	UT_BEGIN();
		printf("my_sub a=%d b=%d\n", a, b--);
		UT_SLEEP(200, 0);
	UT_END();
}


UT_THREAD(my_thread1)
{
	//printf("cont=%d\n", *__cont);
	UT_BEGIN();
		while (1)
		{
			printf("Alive1\n");
			UT_SUB(my_sub, 1, 10);
			UT_SLEEP(1000, 0x01);
		}
	UT_END();
}


UT_THREAD(my_thread2)
{
	//printf("cont=%d\n", *__cont);
	UT_BEGIN();
		while (1)
		{
			printf("Alive2\n");
			UT_SUB(my_sub, 2, 20);
			UT_SLEEP(300, 0);
			UT_WAKE(0x01);
		}
	UT_END();
}


UT_THREAD(my_thread3)
{
	//printf("cont=%d\n", *__cont);
	UT_BEGIN();
		while (1)
		{
			printf("Alive3.1\n");
			UT_SLEEP(500, 0);
			printf("Alive3.2\n");
			UT_WAIT(500, 0);
		}
	UT_END();
}


UT_THREAD(my_thread4)
{
	UT_BEGIN();
		while (1)
		{
			printf("Alive 4\n");
			UT_SLEEP(1000, 0);
		}
	UT_END();
}


UT_THREAD(my_thread5)
{
	UT_BEGIN();
		while (1)
		{
			printf("Alive 5.1\n");
			UT_SLEEP(500, 0);
			printf("Alive 5.2\n");
			UT_WAIT(500, 0);
		}
	UT_END();
}



int main()
{
	utInit();
	
	utThread(my_thread1, 0);
	utThread(my_thread2, 0);
	utThread(my_thread3, 0);
	//utThread(my_thread4, 0);
	//utThread(my_thread5, 0);
	
	utStart();
}
