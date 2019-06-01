/* Verifies that lowering a thread's priority so that it is no
   longer the highest-priority thread in the system causes it to
   yield immediately. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/thread.h"

static thread_func changing_thread;

void
test_priority_change (void)
{
  /* This test does not work with the MLFQS. */
  ASSERT (!thread_mlfqs);

  msg ("Creating a high-priority thread 2.");//1
  thread_create ("thread 2", PRI_DEFAULT + 1, changing_thread, NULL);//2
  msg ("Thread 2 should have just lowered its priority.");//3
  thread_set_priority (PRI_DEFAULT - 2);//4
  msg ("Thread 2 should have just exited.");//5
}

static void
changing_thread (void *aux UNUSED)
{
  msg ("Thread 2 now lowering priority.");//6
  thread_set_priority (PRI_DEFAULT - 1);//7
  msg ("Thread 2 exiting.");//8
}
