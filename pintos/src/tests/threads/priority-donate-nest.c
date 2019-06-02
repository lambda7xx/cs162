/* Low-priority main thread L acquires lock A.  Medium-priority
   thread M then acquires lock B then blocks on acquiring lock A.
   High-priority thread H then blocks on acquiring lock B.  Thus,
   thread H donates its priority to M, which in turn donates it
   to thread L.

   Based on a test originally submitted for Stanford's CS 140 in
   winter 1999 by Matt Franklin <startled@leland.stanford.edu>,
   Greg Hutchins <gmh@leland.stanford.edu>, Yu Ping Hu
   <yph@cs.stanford.edu>.  Modified by arens. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

struct locks
  {
    struct lock *a;
    struct lock *b;
  };

static thread_func medium_thread_func;
static thread_func high_thread_func;

void
test_priority_donate_nest (void)
{
  struct lock a, b;
  struct locks locks;

  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  lock_init (&a);
  lock_init (&b);

  lock_acquire (&a);//1

  locks.a = &a;//2
  locks.b = &b;//3
  thread_create ("medium", PRI_DEFAULT + 1, medium_thread_func, &locks);//4
  thread_yield ();//5
  msg ("Low thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 1, thread_get_priority ());//6

  thread_create ("high", PRI_DEFAULT + 2, high_thread_func, &b);//7
  //printf("before 8,the current thread's name is %s\n", thread_current()->name ); main
  thread_yield ();//8
  //printf("after 8,the current thread's name is %s\n", thread_current()->name); main
  msg ("Low thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 2, thread_get_priority ());//9

  lock_release (&a);//10
 //  printf("before 11,the current thread's name is %s\n", thread_current()->name);main

  thread_yield ();//11
   //printf("after 11,the current thread's name is %s\n", thread_current()->name);main

  msg ("Medium thread should just have finished.");//12
  msg ("Low thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT, thread_get_priority ());//13
}

static void
medium_thread_func (void *locks_)
{
  struct locks *locks = locks_;//14

  lock_acquire (locks->b);//15
  lock_acquire (locks->a);//16

  msg ("Medium thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 2, thread_get_priority ());//16
  msg ("Medium thread got the lock.");//17

  lock_release (locks->a);//18
  thread_yield ();//19

  lock_release (locks->b);//20
  thread_yield ();//21

  msg ("High thread should have just finished.");//22
  msg ("Middle thread finished.");//23
}

static void
high_thread_func (void *lock_)
{
  struct lock *lock = lock_;

  lock_acquire (lock);//24
  msg ("High thread got the lock.");//25
  lock_release (lock);//26
  msg ("High thread finished.");//27
}
