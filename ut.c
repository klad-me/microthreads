#include "ut.h"


static struct thread threads[MAX_THREADS];
struct thread *currentThread;
sem_t ut_wake;


void utInit(void)
{
	uint8_t i;
	
	// Очищаем список ниток
	for (i=0; i<MAX_THREADS; i++)
		threads[i].code=0;
}


void utThread(thread_t code, void *arg)
{
	uint8_t i;
	
	// Помещаем в свободный слот
	for (i=0; i<MAX_THREADS; i++)
	{
		if (! threads[i].code)
		{
			threads[i].state=THREAD_RUNNING;
			threads[i].wait=0;
			threads[i].line=0;
			threads[i].T=0;
			threads[i].code=code;
			threads[i].arg=arg;
			return;
		}
	}
}


void utStart(void)
{
	uint16_t prevT=utTime();
	
	while (1)
	{
		uint8_t i;
		
		
		// Получаем время, прошедшее с момента последней итерации
		uint16_t curT=utTime();
		uint16_t dT=curT - prevT;
		prevT=curT;
		
		
		// Сбрасываем маску просыпания от задачи к задаче
		ut_wake=0;
		
		
		// Запускаем все нитки
		for (i=0; i<MAX_THREADS; i++)
		{
			currentThread=&threads[i];
			
			if (currentThread->code)
			{
				// Считаем таймер сна, и будим нитку, если таймер кончился
				if (currentThread->T > dT)
					currentThread->T-=dT; else
					currentThread->state=THREAD_RUNNING;
				
				//printf("cp %d state=%d T=%d\n", i, currentThread->state, currentThread->T);
				
				// Запускаем код, если нитка в нужном состоянии
				if (currentThread->state == THREAD_RUNNING)
				{
					//printf("run thread %d\n", i);
					if ( (currentThread->state = currentThread->code( &currentThread->line, currentThread->arg )) == THREAD_FINISHED )
						threads[i].code=0;
				}
			}
		}
		
		
		// Получаем внешние маски просыпания
		ut_wake|=utExtWake();
		
		
		// Обновляем состояния задач и получаем допустимое время сна и ожидания
		uint16_t t_sleep, t_wait;
		t_sleep=t_wait=0xffff;
		for (i=0; i<MAX_THREADS; i++)
		{
			if (threads[i].code)
			{
				currentThread=&threads[i];
				
				// Будим нитку, если надо
				if (ut_wake & currentThread->wait)
				{
					currentThread->state=THREAD_RUNNING;
					currentThread->wait&=~ut_wake;
				}
				
				// Обрабатываем состояние нитки
				switch (currentThread->state)
				{
					case THREAD_RUNNING:
						// Работает - спать нельзя
						t_sleep=t_wait=0;
						break;
					
					case THREAD_SLEEPING:
						// Спит
						if (currentThread->T < t_sleep)
							t_sleep=currentThread->T;
						break;
					
					case THREAD_WAITING:
						// Ждет
						if (t_sleep < t_wait)
							t_wait=t_sleep;
						t_sleep=0;	// запрещаем сон
						if (currentThread->T < t_wait)
							t_wait=currentThread->T;
						break;
				}
			}
		}
		
		
		// Засыпаем, если надо
		if (t_sleep > 0) utDoSleep(t_sleep); else
		if (t_wait > 0) utDoWait(t_wait);
	}
}
