#ifndef MICROTHREADS_H
#define MICROTHREADS_H


#include <stdint.h>


#define MAX_THREADS		8


enum
{
	THREAD_RUNNING=0,
	THREAD_SLEEPING,
	THREAD_WAITING,
	THREAD_FINISHED,
};


typedef uint8_t  sem_t;
typedef uint16_t line_t;
typedef uint8_t  (*thread_t)(line_t*, void*);


struct thread
{
	uint8_t		state;
	sem_t		wait;
	line_t		line;
	uint16_t	T;
	thread_t	code;
	void*		arg;
};


// Указатель на текущую выполняемую нитку
extern struct thread *currentThread;

extern sem_t ut_wake;


#define UT(name)			uint8_t name(line_t *__cont, void *arg)

#define UT_BEGIN()			static line_t __sub; (void)__sub; switch (*__cont) { case 0:
#define UT_END()			default: ; } return THREAD_FINISHED;
#define UT_SLEEP(ms, sem)	do { currentThread->T=(ms); currentThread->wait=(sem); \
								(*__cont)=__LINE__; return THREAD_SLEEPING; case __LINE__: ; \
								} while(0)
#define UT_WAIT(ms, sem)	do { currentThread->T=(ms); currentThread->wait=(sem); \
								(*__cont)=__LINE__; return THREAD_WAITING; case __LINE__: ; \
								} while(0)
#define UT_YIELD()			do { \
								(*__cont)=__LINE__; return THREAD_RUNNING; case __LINE__: ; \
								} while(0)
#define UT_WAKE(sem)		do { ut_wake|=(sem); } while(0)
#define UT_EXIT()			do { return THREAD_FINISHED; } while(0)
#define UT_SUB(fn, arg)		do { uint8_t rv; __sub=0; \
								(*__cont)=__LINE__; case __LINE__: rv=fn(&__sub, arg); if (rv != THREAD_FINISHED) return rv; \
								} while(0)


void utInit(void);
void utThread(thread_t code, void *arg);
void utStart(void);

// Должно быть определено извне
extern uint16_t utTime(void);
extern sem_t utExtWake(void);
extern void utDoSleep(uint16_t T);
extern void utDoWait(uint16_t T);


#endif
