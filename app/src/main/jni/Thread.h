#ifndef __Thread__
#define __Thread__

#include <pthread.h>

class Thread
{
public:
    Thread();
    virtual 	~Thread();
    void  		Start();
    void  		Join();
    void  		Detach();
    void  		Cancel();
    pthread_t   Self();
	virtual void Quit();
	bool        IsRunning();

    static void * stThreadRoutine(void*);

    virtual void  Run() = 0;

protected:
	bool			m_running;

private:
    pthread_t       m_thread;
    pthread_attr_t  m_thread_attr;
};

#endif