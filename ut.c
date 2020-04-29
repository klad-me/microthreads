#include "ut.h"


static ut_thread threads[UT_MAX_THREADS];
ut_thread *utCurrentThread;
ut_sem_t ut_wake;


void utInit(void)
{
	uint8_t i;
	
	// Очищаем список ниток
	for (i=0; i<UT_MAX_THREADS; i++)
		threads[i].code=0;
}


void utThread(ut_thread_t code, void *arg)
{
	uint8_t i;
	
	// Помещаем в свободный слот
	for (i=0; i<UT_MAX_THREADS; i++)
	{
		if (! threads[i].code)
		{
			threads[i].state=UT_RUNNING;
			threads[i].wait=0;
			threads[i].line=0;
			threads[i].T=0;
			threads[i].code=code;
			threads[i].arg=arg;
			return;
		}
	}
}


static void utWakeThreads(ut_sem_t mask)
{
	uint8_t i;
	
	if (mask == 0) return;
	
	for (i=0; i<UT_MAX_THREADS; i++)
	{
		if (threads[i].code)
		{
			ut_thread *thr=&threads[i];
			
			// Будим нитку, если надо
			if (thr->wait & mask)
			{
				thr->state=UT_RUNNING;
				thr->wait&=~mask;
			}
		}
	}
}


void utStart(void)
{
	ut_time_t prevT=utTime();
	
	while (1)
	{
		uint8_t i;
		
		
		// Получаем время, прошедшее с момента последней итерации
		ut_time_t curT=utTime();
		ut_time_t dT=curT - prevT;
		prevT=curT;
		
		
		// Будим задачи по внешним событиям
		utWakeThreads(utExtWake());
		
		
		// Сбрасываем маску просыпания от задачи к задаче (программные семафоры)
		ut_wake=0;
		
		
		// Запускаем все нитки
		for (i=0; i<UT_MAX_THREADS; i++)
		{
			utCurrentThread=&threads[i];
			
			if (utCurrentThread->code)
			{
				// Считаем таймер сна, и будим нитку, если таймер кончился
				if (utCurrentThread->T > dT)
					utCurrentThread->T-=dT; else
					utCurrentThread->state=UT_RUNNING;
				
				//printf("cp %d state=%d T=%d\n", i, utCurrentThread->state, utCurrentThread->T);
				
				// Запускаем код, если нитка в нужном состоянии
				if (utCurrentThread->state == UT_RUNNING)
				{
					//printf("run thread %d\n", i);
					if ( (utCurrentThread->state = utCurrentThread->code( &utCurrentThread->line, utCurrentThread->arg )) == UT_FINISHED )
						threads[i].code=0;
				}
			}
		}
		
		
		// Будим задачи по программным семафорам
		utWakeThreads(ut_wake);
		
		
		// Получаем допустимое время сна или ожидания
		ut_time_t t_sleep=(ut_time_t)-1;	// максимальное число для беззнакового типа
		uint8_t can_sleep=1;
		for (i=0; i<UT_MAX_THREADS; i++)
		{
			if (threads[i].code)
			{
				ut_thread *thr=&threads[i];
				
				// Обрабатываем состояние нитки
				switch (thr->state)
				{
					case UT_RUNNING:
						// Работает - спать нельзя
						t_sleep=0;
						can_sleep=0;
						break;
					
					case UT_SLEEPING:
						// Спит
						if (thr->T < t_sleep) t_sleep=thr->T;
						break;
					
					case UT_WAITING:
						// Ждет - можно ждать, но не спать
						if (thr->T < t_sleep) t_sleep=thr->T;
						can_sleep=0;
						break;
				}
			}
		}
		
		
		// Засыпаем, если надо
		if (t_sleep > 0)
		{
			if (can_sleep)
				utDoSleep(t_sleep); else
				utDoWait(t_sleep);
		}
	}
}
