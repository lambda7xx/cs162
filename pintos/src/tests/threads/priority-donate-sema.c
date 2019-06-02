/* Low priority thread L acquires a lock, then blocks downing a
   semaphore.  Medium priority thread M then blocks waiting on
   the same semaphore.  Next, high priority thread H attempts to
   acquire the lock, donating its priority to L.

   Next, the main thread ups the semaphore, waking up L.  L
   releases the lock, which wakes up H.  H "up"s the semaphore,
   waking up M.  H terminates, then M, then L, and finally the
   main thread.

   Written by Godmar Back <gback@cs.vt.edu>. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

struct lock_and_sema
  {
    struct lock lock;
    struct semaphore sema;
  };

static thread_func l_thread_func;
static thread_func m_thread_func;
static thread_func h_thread_func;

void
test_priority_donate_sema (void)
{
  struct lock_and_sema ls;

  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);//-1

  lock_init (&ls.lock);//0
  sema_init (&ls.sema, 0);//0.5
  thread_create ("low", PRI_DEFAULT + 1, l_thread_func, &ls);//1
  thread_create ("med", PRI_DEFAULT + 3, m_thread_func, &ls);//2
  thread_create ("high", PRI_DEFAULT + 5, h_thread_func, &ls);//3
  sema_up (&ls.sema);//4
  msg ("Main thread finished.");//5
}

static void
l_thread_func (void *ls_)
{
  struct lock_and_sema *ls = ls_;

  lock_acquire (&ls->lock);//6
  msg ("Thread L acquired lock.");//7
  sema_down (&ls->sema);//8
  msg ("Thread L downed semaphore.");//9
  lock_release (&ls->lock);//10
  msg ("Thread L finished.");//11
}

static void
m_thread_func (void *ls_)
{
  struct lock_and_sema *ls = ls_;

  sema_down (&ls->sema);//12
  msg ("Thread M finished.");//13
}

static void
h_thread_func (void *ls_)
{
  struct lock_and_sema *ls = ls_;

  lock_acquire (&ls->lock);//14
  msg ("Thread H acquired lock.");//15

  sema_up (&ls->sema);//16
  lock_release (&ls->lock);//17
  msg ("Thread H finished.");//18
}
