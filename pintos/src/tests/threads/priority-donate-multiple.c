/* The main thread acquires locks A and B, then it creates two
   higher-priority threads.  Each of these threads blocks
   acquiring one of the locks and thus donate their priority to
   the main thread.  The main thread releases the locks in turn
   and relinquishes its donated priorities.

   Based on a test originally submitted for Stanford's CS 140 in
   winter 1999 by Matt Franklin <startled@leland.stanford.edu>,
   Greg Hutchins <gmh@leland.stanford.edu>, Yu Ping Hu
   <yph@cs.stanford.edu>.  Modified by arens. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func a_thread_func;
static thread_func b_thread_func;

void
test_priority_donate_multiple (void)
{
  struct lock a, b;

  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  lock_init (&a);
  lock_init (&b);

  lock_acquire (&a);//1
  lock_acquire (&b);//2

  thread_create ("a", PRI_DEFAULT + 1, a_thread_func, &a);//3
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 1, thread_get_priority ());//4

  thread_create ("b", PRI_DEFAULT + 2, b_thread_func, &b);//5
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 2, thread_get_priority ());//6

  lock_release (&b);//7
  msg ("Thread b should have just finished.");//8
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 1, thread_get_priority ());//9

  lock_release (&a);//10
  msg ("Thread a should have just finished.");//11
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT, thread_get_priority ());//12
}

static void
a_thread_func (void *lock_)
{
  struct lock *lock = lock_;

  lock_acquire (lock);//13
  msg ("Thread a acquired lock a.");//14
  lock_release (lock);//15
  msg ("Thread a finished.");//16
}

static void
b_thread_func (void *lock_)
{
  struct lock *lock = lock_;

  lock_acquire (lock);//17
  msg ("Thread b acquired lock b.");//118
  lock_release (lock);//19
  msg ("Thread b finished.");//20
}
