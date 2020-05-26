#ifndef MICROTHREADS_UT_H
#define MICROTHREADS_UT_H


// Файл конфигурации
#ifdef UT_CONFIG_H
	#include UT_CONFIG_H
#else
	#include <stdint.h>
#endif


// Максимальное количество задач
#ifndef UT_MAX_THREADS
	#define UT_MAX_THREADS		8
#endif


#ifdef __cplusplus
extern "C" {
#endif


// Состояния задачи
enum
{
	UT_RUNNING=0,	// Задача работает
	UT_SLEEPING,	// Задача спит
	UT_WAITING,		// Задача ждет
	UT_FINISHED,	// Задача завершена
};


// Тип для хранения семафоров. Один бит - один семафор.
#ifndef ut_sem_t
	typedef uint8_t ut_sem_t;
#endif

// Тип для хранения времени (должен быть беззнаковым).
#ifndef ut_time_t
	typedef uint16_t ut_time_t;
#endif

// Тип для хранения номер строки файла (для продолжения выполнения).
typedef uint16_t ut_line_t;

// Прототип функции-задачи
typedef uint8_t  (*ut_thread_t)(ut_line_t*, void*);


// Структура задачи
typedef struct ut_thread
{
	uint8_t		state;	// Состояние - UT_*
	ut_sem_t	wait;	// Ожидаемые семафоры (задача перейдет в UT_RUNNING по любому из них)
	ut_line_t	line;	// Номер строки для продолжения работы
	ut_time_t	T;		// Оставшееся время сна/ожидания
	ut_thread_t	code;	// Код задачи
	void*		arg;	// Аргумент для задачи
} ut_thread;


// Указатель на текущую выполняемую задачу
extern ut_thread *utCurrentThread;

// Биты пробуждения (чтобы одна задача могла разбудить другую, см. UT_WAKE)
extern ut_sem_t ut_wake;


// Объявить задачу
#define UT(name)			uint8_t name(ut_line_t *__cont, void *arg)


// Начало кода задачи/подпрограммы
#define UT_BEGIN()			static ut_line_t __sub; (void)__sub; (void)arg; switch (*__cont) { case 0:

// Конец кода задачи/подпрограммы
#define UT_END()			default: ; } return UT_FINISHED;

// Спать с ожиданием семафора (сон - режим экономии энергии, время сна - довольно приблизительное)
#define UT_SLEEP(ms, sem)	do { utCurrentThread->T=(ms); utCurrentThread->wait=(sem); \
								(*__cont)=__LINE__; return UT_SLEEPING; case __LINE__: ; \
								} while(0)

// Ждать семафора указанное время (ожидание семафора указанное время, время ожидания - точное)
#define UT_WAIT(ms, sem)	do { utCurrentThread->T=(ms); utCurrentThread->wait=(sem); \
								(*__cont)=__LINE__; return UT_WAITING; case __LINE__: ; \
								} while(0)

// Освободить процессорное время
#define UT_YIELD()			do { \
								(*__cont)=__LINE__; return UT_RUNNING; case __LINE__: ; \
								} while(0)

// Разбудить задачи, ожидающие указанные семафоры
#define UT_WAKE(sem)		do { ut_wake|=(sem); } while(0)

// Проверить - была ли задача разбужена по указанным семафорам (если указано несколько семафоров - вернет TRUE, если сработали сразу все семафоры)
#define UT_WOKEN_BY(sem)	((utCurrentThread->wait & sem)==0)

// Выйти из задачи
#define UT_EXIT()			do { return UT_FINISHED; } while(0)

// Запустить подпрограмму
#define UT_SUB(fn, arg)		do { uint8_t rv; __sub=0; \
								(*__cont)=__LINE__; case __LINE__: rv=fn(&__sub, arg); if (rv != UT_FINISHED) return rv; \
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
void utThread(ut_thread_t code, void *arg);

/**
 * Запустить планировщик
 */
void utStart(void);



// Все, что ниже, должно быть определено извне


/**
 * Получить текущую метку времени.
 * Время должно продолжать идти даже во время сна и ожидания.
 */
#ifndef utTime
	extern ut_time_t utTime(void);
#endif

/*
 * Получить семафоры пробуждения.
 * Должна возвращать только те семафоры, которые выставились с момента прошлого запуска utWakeFlags()
 */
#ifndef utWakeFlags
	extern ut_sem_t utWakeFlags(void);
#endif

/**
 * Спать указанное время.
 * Время сна - приблизительное (можно переходить в режим глубокого сна)
 * Функция может вернуться раньше, если имеются внешние семафоры.
 */
#ifndef utDoSleep
	extern void utDoSleep(ut_time_t T);
#endif

/**
 * Ожидать указанное время.
 * Время ожидания - точное.
 * Функция может вернуться раньше, если имеются внешние семафоры.
 */
#ifndef utDoWait
	extern void utDoWait(ut_time_t T);
#endif


#ifdef __cplusplus
}
#endif


#endif
