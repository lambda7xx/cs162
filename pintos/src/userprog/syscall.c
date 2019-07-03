#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void check_valid_esp(void * vaddr);
static int write(int fd,const void * buffer, unsigned size);
static bool right_stack(void * vaddr);
static void halt(void);/*halt syscall call to terminates pintos */
/* exec system call */
static tid_t exec(const char *cmd_line);





static void
syscall_handler (struct intr_frame *f UNUSED)
{ check_valid_esp(f->esp);
  uint32_t* args = ((uint32_t*) f->esp);
  //printf("System call number: %d\n", args[0]);
  /*if (args[0] == SYS_EXIT) {
    f->eax = args[1];
    printf("%s: exit(%d)\n", &thread_current ()->name, args[1]);
    thread_exit();
  }
 else if(args[0] == SYS_WRITE){
   f->eax = write(args[1],(void*)args[2],(unsigned )args[3]);*/
  switch(args[0]){
	case SYS_HALT:
		halt();
		break;
	case SYS_EXIT:
		f->eax = args[1];
		printf("%s: exit(%d)\n", &thread_current ()->name, args[1]);
    		thread_exit();
		break;
	
	case SYS_WRITE:
		 f->eax = write(args[1],(void*)args[2],(unsigned )args[3]);
		 break;
	case SYS_EXEC:
		 f->eax = exec(( char *)args[1]);
		 break;
	
}
}



static bool right_stack(void * vaddr){
	return vaddr <= (void*)0xbffffff4;
}
static void check_valid_esp(void * vaddr){
	if(!is_user_vaddr(vaddr) || vaddr == NULL || !is_not_in_user_process_address_space(vaddr) || !right_stack(vaddr)){
	//exit(-1);
	printf("%s: exit(%d)\n", &thread_current ()->name, -1);
	thread_exit();
}
}
static int  write(int fd, const void *buf, unsigned size)
{ 
  int result = 0;
 switch(fd){
   case 1:
	putbuf((char *)buf,size);
	result = size;
	break;
}
  
  return result;
}

static void halt(void){
	shutdown_power_off();
}

static tid_t exec(const char * cmd_line){
      tid_t pid = 0;
      tid_t temp  = process_execute(cmd_line);
      if(temp == TID_ERROR)
		pid = -1;
      else
		pid = temp;
      return pid;
}
