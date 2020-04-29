#include "ut.h"


static struct ut_thread threads[UT_MAX_THREADS];
struct ut_thread *utCurrentThread;
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
		
		
		// Сбрасываем маску просыпания от задачи к задаче
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
		
		
		// Получаем внешние маски просыпания
		ut_wake|=utExtWake();
		
		
		// Обновляем состояния задач и получаем допустимое время сна и ожидания
		ut_time_t t_sleep, t_wait;
		t_sleep=t_wait=(ut_time_t)-1;	// максимальное число для беззнакового типа
		for (i=0; i<UT_MAX_THREADS; i++)
		{
			if (threads[i].code)
			{
				utCurrentThread=&threads[i];
				
				// Будим нитку, если надо
				if (ut_wake & utCurrentThread->wait)
				{
					utCurrentThread->state=UT_RUNNING;
					utCurrentThread->wait&=~ut_wake;
				}
				
				// Обрабатываем состояние нитки
				switch (utCurrentThread->state)
				{
					case UT_RUNNING:
						// Работает - спать нельзя
						t_sleep=t_wait=0;
						break;
					
					case UT_SLEEPING:
						// Спит
						if (utCurrentThread->T < t_sleep)
							t_sleep=utCurrentThread->T;
						break;
					
					case UT_WAITING:
						// Ждет
						if (t_sleep < t_wait)
							t_wait=t_sleep;
						t_sleep=0;	// запрещаем сон
						if (utCurrentThread->T < t_wait)
							t_wait=utCurrentThread->T;
						break;
				}
			}
		}
		
		
		// Засыпаем, если надо
		if (t_sleep > 0) utDoSleep(t_sleep); else
		if (t_wait > 0) utDoWait(t_wait);
	}
}
