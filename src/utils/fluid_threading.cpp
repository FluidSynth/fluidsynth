#include "fluid_threading.h"
#include <cstdio>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <ratio>
#include <condition_variable>
thread_local void* privs[16];
bool occupied[16]={false};
void _mutex_init(_mutex *mutex)
{
	mutex->mutex=(void*)(new std::mutex);
}
void _mutex_clear(_mutex *mutex)
{
	delete (std::mutex*)(mutex->mutex);
	mutex->mutex=NULL;
}
void _mutex_lock(_mutex *mutex)
{
	if(!mutex->mutex)mutex->mutex=(void*)(new std::mutex);
	((std::mutex*)(mutex->mutex))->lock();
}
void _mutex_unlock(_mutex *mutex)
{
	if(!mutex->mutex)mutex->mutex=(void*)(new std::mutex);
	((std::mutex*)(mutex->mutex))->unlock();
}

void _rec_mutex_init(_rec_mutex *recmutex)
{
	recmutex->recmutex=(void*)(new std::recursive_mutex);
}
void _rec_mutex_clear(_rec_mutex *recmutex)
{
	delete (std::recursive_mutex*)(recmutex->recmutex);
	recmutex->recmutex=NULL;
}
void _rec_mutex_lock(_rec_mutex *recmutex)
{
	if(!recmutex->recmutex)
	recmutex->recmutex=(void*)(new std::recursive_mutex);
	((std::recursive_mutex*)(recmutex->recmutex))->lock();
}
void _rec_mutex_unlock(_rec_mutex *recmutex)
{
	if(!recmutex->recmutex)
	recmutex->recmutex=(void*)(new std::recursive_mutex);
	((std::recursive_mutex*)(recmutex->recmutex))->unlock();
}

void _cond_init(_cond *cond)
{
	cond->cond=(void*)(new std::condition_variable_any);
}
void _cond_clear(_cond *cond)
{
	delete (std::condition_variable_any*)(cond->cond);
}
void _cond_signal(_cond *cond)
{
	((std::condition_variable_any*)(cond->cond))->notify_one();
}
void _cond_broadcast(_cond *cond)
{
	((std::condition_variable_any*)(cond->cond))->notify_all();
}
void _cond_wait(_cond *cond,_mutex *mutex)
{
	((std::condition_variable_any*)(cond->cond))->wait(*(std::mutex*)(mutex->mutex));
}

void* _private_get(_private *priv)
{
	if(!priv->id)
	for(int i=0;i<16;++i)if(!occupied[i]){priv->id=i+1;break;}
	if(priv->id)return privs[priv->id-1];else return NULL;
}
void _private_set(_private *priv,void *val)
{
	if(!priv->id)
	for(int i=0;i<16;++i)if(!occupied[i]){priv->id=i+1;break;}
	if(priv->id)privs[priv->id-1]=val;
}
unsigned _thread_get_id(void)
{
	std::hash<std::thread::id>h;
	return h(std::this_thread::get_id());
}
void _thread_create(_thread *th,_thread_func_t func,void* data)
{
	th->thrd=(void*)(new std::thread(func,data));
}
void _thread_detach(_thread *th)
{
	if(!th->thrd)return;
	((std::thread*)(th->thrd))->detach();
	delete (std::thread*)(th->thrd);
	th->thrd=nullptr;
}
void _thread_join(_thread *th)
{
	if(!th->thrd)return;
	((std::thread*)(th->thrd))->join();
	delete (std::thread*)(th->thrd);
	th->thrd=nullptr;
}
void _thread_sleep(unsigned long us)
{
	std::this_thread::sleep_for(std::chrono::microseconds(us));
}
double _monotonic_time()
{
	return std::chrono::duration_cast<std::chrono::duration<double,std::micro>>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
