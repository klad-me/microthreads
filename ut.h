#ifndef MICROTHREADS_H
#define MICROTHREADS_H


#include <stdint.h>


// Максимальное количество задач
#define MAX_THREADS		8


// Состояния задачи
enum
{
	THREAD_RUNNING=0,	// Задача работает
	THREAD_SLEEPING,	// Задача спит
	THREAD_WAITING,		// Задача ждет
	THREAD_FINISHED,	// Задача завершена
};


// Тип для хранения семафоров. Один бит - один семафор.
typedef uint8_t  sem_t;

// Тип для хранения номер строки файла (для продолжения выполнения).
typedef uint16_t line_t;

// Прототип функции-задачи
typedef uint8_t  (*thread_t)(line_t*, void*);


// Структура задачи
struct thread
{
	uint8_t		state;	// Состояние - THREAD_*
	sem_t		wait;	// Ожидаемые семафоры (задача перейдет в THREAD_RUNNING по любому из них)
	line_t		line;	// Номер строки для продолжения работы
	uint16_t	T;		// Оставшееся время сна/ожидания
	thread_t	code;	// Код задачи
	void*		arg;	// Аргумент для задачи
};


// Указатель на текущую выполняемую задачу
extern struct thread *currentThread;

// Биты пробуждения (чтобы одна задача могла разбудить другую, см. UT_WAKE)
extern sem_t ut_wake;


// Объявить задачу
#define UT(name)			uint8_t name(line_t *__cont, void *arg)


// Начало кода задачи/подпрограммы
#define UT_BEGIN()			static line_t __sub; (void)__sub; switch (*__cont) { case 0:

// Конец кода задачи/подпрограммы
#define UT_END()			default: ; } return THREAD_FINISHED;

// Спать с ожиданием семафора (сон - режим экономии энергии, время сна - довольно приблизительное)
#define UT_SLEEP(ms, sem)	do { currentThread->T=(ms); currentThread->wait=(sem); \
								(*__cont)=__LINE__; return THREAD_SLEEPING; case __LINE__: ; \
								} while(0)

// Ждать семафора указанное время (ожидание семафора указанное время, время ожидания - точное)
#define UT_WAIT(ms, sem)	do { currentThread->T=(ms); currentThread->wait=(sem); \
								(*__cont)=__LINE__; return THREAD_WAITING; case __LINE__: ; \
								} while(0)

// Освободить процессорное время
#define UT_YIELD()			do { \
								(*__cont)=__LINE__; return THREAD_RUNNING; case __LINE__: ; \
								} while(0)

// Разбудить задачи, ожидающие указанные семафоры
#define UT_WAKE(sem)		do { ut_wake|=(sem); } while(0)

// Проверить - была ли задача разбужена по указанным семафорам (если указано несколько семафоров - вернет TRUE, если сработали сразу все семафоры)
#define UT_WOKEN_BY(sem)	((currentThread->wait & sem)==0)

// Выйти из задачи
#define UT_EXIT()			do { return THREAD_FINISHED; } while(0)

// Запустить подпрограмму
#define UT_SUB(fn, arg)		do { uint8_t rv; __sub=0; \
								(*__cont)=__LINE__; case __LINE__: rv=fn(&__sub, arg); if (rv != THREAD_FINISHED) return rv; \
								} while(0)


/**
 * Инициализация
 */
void utInit(void);

/**
 * Создать новую задачу
 *  code - код задачи
 *  arg  - аргумент (контекст)
 */
void utThread(thread_t code, void *arg);

/**
 * Запустить планировщик
 */
void utStart(void);



// Все, что ниже, должно быть определено извне


/**
 * Получить текущую метку времени.
 * Время должно продолжать идти даже во время сна и ожидания.
 */
extern uint16_t utTime(void);

/*
 * Получить семафоры внешнего пробуждения.
 * Должна возвращать только те семафоры, которые выставились с момента прошлого запуска utExtWake()
 */
extern sem_t utExtWake(void);

/**
 * Спать указанное время.
 * Время сна - приблизительное (можно переходить в режим глубокого сна)
 * Функция может вернуться раньше, если имеются внешние семафоры.
 */
extern void utDoSleep(uint16_t T);

/**
 * Ожидать указанное время.
 * Время ожидания - точное.
 * Функция может вернуться раньше, если имеются внешние семафоры.
 */
extern void utDoWait(uint16_t T);


#endif
