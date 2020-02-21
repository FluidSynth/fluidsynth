#ifndef _FLUID_THREADING_H
#define _FLUID_THREADING_H
#ifdef __cplusplus
extern "C"{
#endif
typedef int (*_thread_func_t)(void* data);
typedef struct __mtx{
	void* mutex;
}_mutex;
typedef struct __recmtx{
	void* recmutex;
}_rec_mutex;
typedef struct __cnd{
	void* cond;
}_cond;
typedef struct __prvt{
	int id;
}_private;
typedef struct __thrd{
	void* thrd;
}_thread;
void _mutex_init(_mutex *mutex);
void _mutex_clear(_mutex *mutex);
void _mutex_lock(_mutex *mutex);
void _mutex_unlock(_mutex *mutex);

void _rec_mutex_init(_rec_mutex *recmutex);
void _rec_mutex_clear(_rec_mutex *recmutex);
void _rec_mutex_lock(_rec_mutex *recmutex);
void _rec_mutex_unlock(_rec_mutex *recmutex);

void _cond_init(_cond *cond);
void _cond_clear(_cond *cond);
void _cond_signal(_cond *cond);
void _cond_broadcast(_cond *cond);
void _cond_wait(_cond *cond,_mutex *mutex);

void* _private_get(_private *priv);
void _private_set(_private *priv,void *val);

unsigned _thread_get_id(void);
void _thread_create(_thread *th,_thread_func_t func,void* data);
void _thread_detach(_thread *th);
void _thread_join(_thread *th);
void _thread_sleep(unsigned long us);
double _monotonic_time(void);
#ifdef __cplusplus
}
#endif
#endif
