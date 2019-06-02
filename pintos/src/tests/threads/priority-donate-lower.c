/* The main thread acquires a lock.  Then it creates a
   higher-priority thread that blocks acquiring the lock, causing
   it to donate their priorities to the main thread.  The main
   thread attempts to lower its priority, which should not take
   effect until the donation is released. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func acquire_thread_func;

void
test_priority_donate_lower (void)
{
  struct lock lock;

  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  lock_init (&lock);//
  lock_acquire (&lock);//1
  thread_create ("acquire", PRI_DEFAULT + 10, acquire_thread_func, &lock);//2
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 10, thread_get_priority ());//3

  msg ("Lowering base priority...");//4
  thread_set_priority (PRI_DEFAULT - 10);//5
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 10, thread_get_priority ());//6
  lock_release (&lock);//7
  msg ("acquire must already have finished.");//8
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT - 10, thread_get_priority ());//9
}

static void
acquire_thread_func (void *lock_)
{
  struct lock *lock = lock_;

  lock_acquire (lock);//10
  msg ("acquire: got the lock");//11
  lock_release (lock);//12
  msg ("acquire: done");//13
}
