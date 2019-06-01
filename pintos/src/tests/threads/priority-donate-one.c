/* The main thread acquires a lock.  Then it creates two
   higher-priority threads that block acquiring the lock, causing
   them to donate their priorities to the main thread.  When the
   main thread releases the lock, the other threads should
   acquire it in priority order.

   Based on a test originally submitted for Stanford's CS 140 in
   winter 1999 by Matt Franklin <startled@leland.stanford.edu>,
   Greg Hutchins <gmh@leland.stanford.edu>, Yu Ping Hu
   <yph@cs.stanford.edu>.  Modified by arens. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func acquire1_thread_func;
static thread_func acquire2_thread_func;

void
test_priority_donate_one (void)
{
  struct lock lock;

  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  lock_init (&lock);
  lock_acquire (&lock);
  thread_create ("acquire1", PRI_DEFAULT + 1, acquire1_thread_func, &lock);//1
  msg ("This thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 1, thread_get_priority ());//20190601在我没有实现这个共能之前
	//thread_get_priority返回的是main线程的优先级//2
//printf("current thread's name is %s\n", thread_current()->name);
//但是按我之前的思路，thread_create函数创造线程acquire1，这时候acquire1获得CPU
  thread_create ("acquire2", PRI_DEFAULT + 2, acquire2_thread_func, &lock);//3
  msg ("This thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 2, thread_get_priority ());//4
  //加了lock后，thread_current()->name 为main
 //printf("current thread's name is %s\n", thread_current()->name);
  lock_release (&lock);//5
  msg ("acquire2, acquire1 must already have finished, in that order.");//6
  msg ("This should be the last line before finishing this test.");//7
}
/*我要是实现的功能: 执行完1后，线程acquire1的优先级为32，main原来的优先级为31，1之后，main优先级变为32；
   同理，3之后，线程acquire1的优先级为33，main由优先级32变为33，5之后
   ，释放锁，然后acquire1和acquire2争夺CPU，acquire2优先级更大， 抢到CPU，
   执行acquire2_thread_func函数，然后输出12和13 ，接着执行8.输出标记9和标记10
  ，所以，我要做的是，
  */
static void
acquire1_thread_func (void *lock_)//8
{
  struct lock *lock = lock_;

  lock_acquire (lock);
  msg ("acquire1: got the lock");//9
  lock_release (lock);
  msg ("acquire1: done");//10
}

static void
acquire2_thread_func (void *lock_)//11
{
  struct lock *lock = lock_;

  lock_acquire (lock);
  msg ("acquire2: got the lock");//12
  lock_release (lock);
  msg ("acquire2: done");//13
}
