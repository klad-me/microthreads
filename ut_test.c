#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "ut.h"


uint16_t utTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	
    unsigned long long v;
	
    v=ts.tv_sec;
    v*=1000;
    v+=ts.tv_nsec/1000000;
	
    return (uint16_t)(v & 0xffff);
}


sem_t utExtWake(void)
{
	return 0;
}


void utDoSleep(uint16_t T)
{
	printf("utDoSleep(%d)\n", T);
	usleep(T*1000);
}


void utDoWait(uint16_t T)
{
	printf("utDoWait(%d)\n", T);
	usleep(T*1000);
}


struct
{
	int x;
} ctx1;


struct
{
	int y;
} ctx2;


struct
{
	int z;
} ctx3;


struct sub_ctx
{
	int a, b;
};


UT(my_sub)
{
	struct sub_ctx *Z=(struct sub_ctx*)arg;
	UT_BEGIN();
		while (Z->b)
		{
			printf("my_sub%d=%d\n", Z->a, Z->b--);
			//UT_SLEEP(100, 0);
			UT_YIELD();
		}
	UT_END();
}


UT(my_thread1)
{
	static struct sub_ctx Z;
	
	//printf("cont=%d\n", *__cont);
	UT_BEGIN();
		while (1)
		{
			printf("Alive1\n");
			Z.a=1;
			Z.b=10;
			UT_SUB(my_sub, &Z);
			UT_SLEEP(1000, 0x01);
		}
	UT_END();
}


UT(my_thread2)
{
	static struct sub_ctx Z;
	
	//printf("cont=%d\n", *__cont);
	UT_BEGIN();
		while (1)
		{
			printf("Alive2\n");
			Z.a=2;
			Z.b=5;
			UT_SUB(my_sub, &Z);
			UT_SLEEP(300, 0);
			UT_WAKE(0x01);
		}
	UT_END();
}


UT(my_thread3)
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


int main()
{
	utInit();
	
	utThread(my_thread1, &ctx1);
	utThread(my_thread2, &ctx2);
	utThread(my_thread3, &ctx3);
	
	utStart();
}
