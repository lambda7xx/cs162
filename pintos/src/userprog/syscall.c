#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "devices/shutdown.h"
#include "userprog/pagedir.h"
#include "filesys/directory.h"
#include "filesys/filesys.h"
#include "lib/string.h"
static void syscall_handler (struct intr_frame *);
static struct lock filesys_lock;
void
syscall_init (void)
{ lock_init(&filesys_lock); 
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void check_valid_esp(void * vaddr);
static int SYS_Write(int fd,const void * buffer, unsigned size);
static bool right_stack(void * vaddr);
static void SYS_Halt(void);/*halt syscall call to terminates pintos */
/* exec system call */
 tid_t SYS_Exec(const char *cmd_line);

 void SYS_Exit(int status);

typedef  int pid_t;

int SYS_Wait(pid_t pid);
int SYS_Practice(int i);
bool SYS_Create(const char *,unsigned );
int SYS_Open(const char * file);

struct file_table{
 struct list_elem  file_elem;/*to insert into the thread's file_list */
 int fd;
 struct file *file;
};

int insert_file_to_thread(struct file * file);

static void
syscall_handler (struct intr_frame *f UNUSED)
{ check_valid_esp(f->esp);
  if(pagedir_get_page (thread_current ()->pagedir, f->esp) == NULL)
	SYS_Exit(-1);
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
		SYS_Halt();
		break;
	case SYS_EXIT:
		f->eax = args[1];
		SYS_Exit(args[1]);
		//printf("%s: exit(%d)\n", &thread_current ()->name, args[1]);
    		//thread_exit();
		break;
	
	case SYS_WRITE:
		 f->eax = SYS_Write(args[1],(void*)args[2],(unsigned )args[3]);
		 break;
	case SYS_EXEC:
		 f->eax = SYS_Exec(( char *)args[1]);
		 break;
	case SYS_WAIT:
		 f->eax = SYS_Wait((int)args[1]);
		 break;
	case SYS_PRACTICE:
		f->eax = SYS_Practice((int)args[1]);
		break;
	case SYS_CREATE:
		f->eax = SYS_Create((const char*)args[1],(off_t)args[2]);
		break;
	case SYS_OPEN:
		f->eax = SYS_Open((const char*)args[1]);
		break;
}
}


int SYS_Wait(pid_t pid){
	return process_wait(pid);
}

static bool right_stack(void * vaddr){
	return vaddr <= (void*)0xbffffff4;
}
static void check_valid_esp(void * vaddr){
	if(!is_user_vaddr(vaddr) || vaddr == NULL || !is_not_in_user_process_address_space(vaddr) || !right_stack(vaddr)){
	//exit(-1);
	SYS_Exit(-1);
	//printf("%s: exit(%d)\n", &thread_current ()->name, -1);
	//thread_exit();
}
}
static int SYS_Write(int fd, const void *buf, unsigned size)
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

static void SYS_Halt(void){
	shutdown_power_off();
}

tid_t SYS_Exec(const char * cmd_line){
      if(cmd_line  == NULL || pagedir_get_page (thread_current ()->pagedir, cmd_line) == NULL)
		return -1;
      tid_t pid = process_execute(cmd_line);
      
      return pid;
}	
 
void SYS_Exit(int status){
	 if(thread_current()->parent != NULL){
		thread_current()->parent->exit_code = status;
}
	 printf("%s: exit(%d)\n", &thread_current ()->name, status);
         thread_exit();
}

int SYS_Practice(int i){
	return i +1 ;
}

bool SYS_Create(const char * file,unsigned initial_size){
 	lock_acquire(&filesys_lock);
	if( pagedir_get_page (thread_current ()->pagedir,file) == NULL){
		lock_release(&filesys_lock);
		SYS_Exit(-1);
}
        if(file == NULL ){
		lock_release(&filesys_lock);
		return false;
}
 	if(strlen(file) >NAME_MAX){
		lock_release(&filesys_lock);
		return false;
}
        
    	bool result =  filesys_create(file,initial_size);
	lock_release(&filesys_lock);
	return result;
}

int SYS_Open(const char * file){
  if(file == NULL)
	return -1;
  if(pagedir_get_page (thread_current ()->pagedir,file) == NULL)
		//return -1;
                SYS_Exit(-1);
  if(strcmp(file,"") == 0) /* empty file */
        return -1;
  struct file * open_file = filesys_open(file);
  if(open_file == NULL)
	return -1;
  int fd =  insert_file_to_thread(open_file);
  return fd;
}


int insert_file_to_thread(struct file *file){
  int fd = thread_current()->fd;
  thread_current()->fd++;
  struct file_table * file_table;
  file_table = palloc_get_page(0);
  file_table->fd = fd;
  file_table->file = file;
  list_push_back(&thread_current()->file_list,&file_table->file_elem);
  return fd;
}
