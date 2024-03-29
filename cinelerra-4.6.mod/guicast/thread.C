
/*
 * CINELERRA
 * Copyright (C) 2008 Adam Williams <broadcast at earthling dot net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

#ifndef NO_GUICAST
#include "bcsignals.h"
#endif
#include <sys/wait.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "thread.h"

// track unjoined threads at termination
#ifndef NO_GUICAST
#include "arraylist.h"
#include "mutex.h"
#include <typeinfo>

class MLocker {
	static Mutex the_lock;
public:
	MLocker() { the_lock.lock(); }
	~MLocker() { the_lock.unlock(); }
};
Mutex MLocker::the_lock;

class the_dbg {
public:
	pthread_t tid;  const char *name;
	the_dbg(pthread_t t, const char *nm) { tid = t; name = nm; }
	~the_dbg() {}
};


static class the_list : public ArrayList<the_dbg*> {
public:
	static void dump_threads(FILE *fp);
	 the_list() {}
	~the_list() {
		MLocker mlkr;
		remove_all_objects();
	}
} thread_list;

static void dbg_add(pthread_t tid, const char *nm)
{
	MLocker mlkr;
	int i = thread_list.size();
	while( --i >= 0 && thread_list[i]->tid != tid );
	if( i >= 0 ) {
		printf("dbg_add, dup %016lx %s %s\n",
			(unsigned long)tid, nm, thread_list[i]->name);
		return;
	}
	thread_list.append(new the_dbg(tid, nm));
}

static void dbg_del(pthread_t tid)
{
	MLocker mlkr;
	int i = thread_list.size();
	while( --i >= 0 && thread_list[i]->tid != tid );
	if( i < 0 ) {
		printf("dbg_del, mis %016lx\n",(unsigned long)tid);
		return;
	}
	thread_list.remove_object_number(i);
}

static class the_chkr {
public:
	the_chkr() {}
	~the_chkr() {
		int i = thread_list.size();
		if( !i ) return;
		printf("unjoined tids %d\n", i);
		while( --i >= 0 ) printf("  %016lx %s\n",
			(unsigned long)thread_list[i]->tid,
			thread_list[i]->name);
	}
} the_chk;
#else
#define dbg_add(t, nm) do {} while(0)
#define dbg_del(t) do {} while(0)
#endif

Thread::Thread(int synchronous, int realtime, int autodelete)
{
	this->synchronous = synchronous != 0;
	this->realtime = realtime != 0;
	this->autodelete = autodelete != 0;
	tid = (pthread_t)-1;
	finished = false;
	cancelled = false;
	cancel_enabled = false;
}

Thread::~Thread()
{
}

void* Thread::entrypoint(void *parameters)
{
	Thread *thread = (Thread*)parameters;

// allow thread to be cancelled at any point during a region where it is enabled.
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
// Disable cancellation by default.
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	thread->cancel_enabled = false;

// Set realtime here seince it doesn't work in start
	if( thread->realtime && getuid() == 0 ) {
		struct sched_param param = { sched_priority : 1 };
		if(pthread_setschedparam(thread->tid, SCHED_RR, &param) < 0)
			perror("Thread::entrypoint pthread_attr_setschedpolicy");
	}

	thread->run();
	thread->finished = true;
	if( !thread->synchronous ) {
		if( !thread->cancelled ) dbg_del(thread->tid);
		if( thread->autodelete ) delete thread;
		else thread->tid = ((pthread_t)-1);
	}
	return NULL;
}

void Thread::start()
{
	pthread_attr_t  attr;
	struct sched_param param;

	pthread_attr_init(&attr);

// previously run, and did not join, join to clean up zombie
	if( synchronous && exists() )
		join();

	finished = false;
	cancelled = false;
	owner = get_self();

// Inherit realtime from current thread the easy way.
	if( !realtime )
		realtime = calculate_realtime();

	if( !synchronous )
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if( realtime && getuid() == 0 ) {
		if(pthread_attr_setschedpolicy(&attr, SCHED_RR) < 0)
			perror("Thread::start pthread_attr_setschedpolicy");
		param.sched_priority = 50;
		if(pthread_attr_setschedparam(&attr, &param) < 0)
			perror("Thread::start pthread_attr_setschedparam");
	}
	else {
		if(pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED) < 0)
			perror("Thread::start pthread_attr_setinheritsched");
	}

	pthread_create(&tid, &attr, Thread::entrypoint, this);

	dbg_add(tid, typeid(*this).name());
}

int Thread::cancel()
{
	if( exists() && !cancelled ) {
		LOCK_LOCKS("Thread::cancel");
		pthread_cancel(tid);
		cancelled = true;
		if( !synchronous ) dbg_del(tid);
		UNLOCK_LOCKS;
	}
	return 0;
}

int Thread::join()   // join this thread
{
	if( !exists() ) return 0;
	if( synchronous ) {
// NOTE: this does not do anything if the thread is not synchronous
		int ret = pthread_join(tid, 0);
		if( ret ) strerror(ret);
		CLEAR_LOCKS_TID(tid);
		dbg_del(tid);
		tid = ((pthread_t)-1);
// Don't execute anything after this.
		if( autodelete ) delete this;
	}
	else {
// kludge
		while( running() && !cancelled ) {
			int ret = pthread_kill(tid, 0);
			if( ret ) break;
			usleep(200000);
		}
		tid = ((pthread_t)-1);
	}
	return 0;
}

int Thread::enable_cancel()
{
	if( !cancel_enabled ) {
		cancel_enabled = true;
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
	return 0;
}

int Thread::disable_cancel()
{
	if( cancel_enabled ) {
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		cancel_enabled = false;
	}
	return 0;
}

int Thread::get_cancel_enabled()
{
	return cancel_enabled;
}

void Thread::exit_thread()
{
	finished = true;
 	pthread_exit(0);
}


int Thread::suspend_thread()
{
	if( exists() )
		pthread_kill(tid, SIGSTOP);
	return 0;
}

int Thread::continue_thread()
{
	if( exists() )
		pthread_kill(tid, SIGCONT);
	return 0;
}

bool Thread::exists()
{
	return tid != ((pthread_t)-1);
}

bool Thread::running()
{
	return exists() & ~finished;
}

int Thread::set_synchronous(int value)
{
	this->synchronous = value != 0;
	return 0;
}

int Thread::set_realtime(int value)
{
	this->realtime = value != 0;
	return 0;
}

int Thread::set_autodelete(int value)
{
	this->autodelete = value != 0;
	return 0;
}

int Thread::get_autodelete()
{
	return autodelete ? 1 : 0;
}

int Thread::get_synchronous()
{
	return synchronous ? 1 : 0;
}

bool Thread::calculate_realtime()
{
//printf("Thread::calculate_realtime %d %d\n", getpid(), sched_getscheduler(0));
	return (sched_getscheduler(0) == SCHED_RR ||
		sched_getscheduler(0) == SCHED_FIFO);
}

int Thread::get_realtime()
{
	return realtime ? 1 : 0;
}

unsigned long Thread::get_tid()
{
	return tid;
}

void Thread::dump_threads(FILE *fp)
{
	int i = thread_list.size();
	while( --i >= 0 ) {
		fprintf(fp, "thread %016lx, %s\n",
			(unsigned long)thread_list[i]->tid,
			thread_list[i]->name);
	}
}

