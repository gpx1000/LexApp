#include <signal.h>
#include <memory.h>
#include "Thread.h"

void thread_exit_handler(int sig)
{
    pthread_exit(0);
}

Thread::Thread()
{
    pthread_attr_init(&m_thread_attr);
    struct sigaction actions;
    memset ((void*)&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    sigaction(SIGUSR1, &actions, 0);
}

Thread::~Thread() { }

void Thread::Start() {
    m_running = true;
    pthread_create(&m_thread, &m_thread_attr, stThreadRoutine, this);
}

void Thread::Join() {
    pthread_join(m_thread, NULL);
}

void Thread::Detach() {
    pthread_detach(m_thread);
}

void Thread::Cancel() {
    pthread_kill(m_thread, SIGUSR1);
}

pthread_t Thread::Self() {
    return pthread_self();
}

void Thread::Quit() {
	m_running = false;
}

bool Thread::IsRunning() {
    return m_running;
}

void* Thread::stThreadRoutine(void* arg) {
    ((Thread*)arg)->Run();
    return 0;
}