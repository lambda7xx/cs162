#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "userprog/pagedir.h"
#include "filesys/directory.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
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
 tid_t SYS_Exec(const char *cmd_line);/*start another process */

 void SYS_Exit(int status);/*terminate this process */

typedef  int pid_t;

static int SYS_Wait(pid_t pid);/* wait for a child process to die*/
static int SYS_Practice(int i);/*return arg increment by 1 */
static bool SYS_Create(const char *,unsigned );/*Create a file */
static int SYS_Open(const char * file);/*Open a file */
static bool SYS_Remove(const char *file);/*Delete a file */
static int SYS_Filesize(int fd);/*Obtain a file's size */
static int SYS_Read(int fd,void *buffer,unsigned size);/*Read from a file */
static void  SYS_Seek(int fd,unsigned position);/*change position in a file */
static unsigned SYS_Tell(int fd);/*report current positionin a file */
static void SYS_Close(int fd);/*close a file */


static int get_user(const uint8_t *uaddr);

static bool put_user(uint8_t *udst, uint8_t byte);


struct file_table{
 struct list_elem  file_elem;/*to insert into the thread's file_list */
 int fd;
 struct file *file;
};

int insert_file_to_thread(struct file * file);

struct file *find_file(int fd);

static void
syscall_handler (struct intr_frame *f UNUSED)
{ check_valid_esp(f->esp);
  if(pagedir_get_page (thread_current ()->pagedir, f->esp) == NULL)
	SYS_Exit(-1);
  uint32_t* args = ((uint32_t*) f->esp);
  switch(args[0]){
	case SYS_HALT:
		SYS_Halt();
		break;
	case SYS_EXIT:
		f->eax = args[1];
		SYS_Exit(args[1]);
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
	case SYS_REMOVE:
		f->eax = SYS_Remove((const char *) args[1]);
		break;
	case SYS_FILESIZE:
		f->eax = SYS_Filesize((int)args[1]);
		break;
	case SYS_READ:
		f->eax = SYS_Read((int)args[1],(void*)args[2],(unsigned)args[3]);
		break;
	case SYS_SEEK:
		SYS_Seek((int)args[1],(unsigned)args[2]);
		break;
	case SYS_TELL:
		f->eax = SYS_Tell((int)args[1]);
		break;
	case SYS_CLOSE:
		SYS_Close((int)args[1]);
		break;
}
}


static int SYS_Wait(pid_t pid){
	return process_wait(pid);
}

static bool right_stack(void * vaddr){
	return vaddr <= (void*)0xbffffff4;
}
static void check_valid_esp(void * vaddr){
	if(!is_user_vaddr(vaddr) || vaddr == NULL || !is_not_in_user_process_address_space(vaddr) || !right_stack(vaddr)){
	SYS_Exit(-1);
}
}
static int SYS_Write(int fd, const void *buf, unsigned size)
{
 if(buf > (void*)0Xc0000000)
	SYS_Exit(-1);

 uint8_t * temp =(uint8_t *) buf; 
 if(get_user(temp) == -1 || (get_user(temp +size) == -1))
	SYS_Exit(-1);
 lock_acquire(&filesys_lock);
  if(size == 0){
	lock_release(&filesys_lock);
	return 0;
}
  if(pagedir_get_page(thread_current()->pagedir,buf)==NULL){
	lock_release(&filesys_lock);
	SYS_Exit(-1);
}
   if(fd == 1){
	putbuf((char *)buf,size);
	lock_release(&filesys_lock);
	return size;
}
  else{
	struct file *file = find_file(fd);
	if(file == NULL){
		lock_release(&filesys_lock);
		SYS_Exit(-1);
	}
	int res =  file_write(file,buf,size);
	lock_release(&filesys_lock);
	return res;
}
  
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
		sema_up(&thread_current()->parent->child_sema);
		thread_current()->parent->exit_code = status;
		//thread->parent = NULL;
		list_remove(&thread_current()->child_elem);
		//thread_current()->parent = NULL;
}
	 printf("%s: exit(%d)\n", &thread_current ()->name, status);
         thread_exit();
}

static int SYS_Practice(int i){
	return i +1 ;
}

static bool SYS_Create(const char * file,unsigned initial_size){
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

static int SYS_Open(const char * file){
  lock_acquire(&filesys_lock);
  if(file == NULL){
	lock_release(&filesys_lock);
	return -1;
 }
  if(pagedir_get_page (thread_current ()->pagedir,file) == NULL){
		lock_release(&filesys_lock);
                SYS_Exit(-1);
}
  if(strcmp(file,"") == 0) /* empty file */{
	lock_release(&filesys_lock);
        return -1;
}
  struct file * open_file = filesys_open(file);
  if(open_file == NULL){
	lock_release(&filesys_lock);
	return -1;
}
  int fd =  insert_file_to_thread(open_file);
  lock_release(&filesys_lock);
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

static bool SYS_Remove(const char *file){
   lock_acquire(&filesys_lock);
   if(file == NULL){
	lock_release(&filesys_lock);
        return  false;
  }
  if(pagedir_get_page (thread_current ()->pagedir,file) == NULL){
		lock_release(&filesys_lock);
                SYS_Exit(-1);
}
  return filesys_remove(file);
}

struct file *find_file(int fd){
	struct file * file = NULL;
	struct list_elem *e;
	for(e = list_begin(&thread_current()->file_list); e != list_end(&thread_current()->file_list); e = list_next(e)){
	 struct file_table * file_t = list_entry(e,struct file_table,file_elem);
	if(file_t->fd == fd){
		file = file_t->file;
		break;	
}

}
	return file;
}


int SYS_Filesize(int fd){
	if(fd < 3)
		return 0;
	struct file *file = find_file(fd);
	if(file == NULL)
		return 0;
	return file_length(file);
}



int SYS_Read(int fd,void * buffer, unsigned size ){
	if(buffer > (void *)0xc0000000)
		SYS_Exit(-1);
	uint8_t *temp = (uint8_t*)buffer;
	unsigned i;
	for(i = 0; i < size; i++){
		if(!put_user(temp,1))
			SYS_Exit(-1);
		temp++;
	}
        lock_acquire(&filesys_lock);
	if(size == 0){
		lock_release(&filesys_lock);
		return 0;
	}
	if(fd == 0){
		int res =  input_getc();
		lock_release(&filesys_lock);	
		return res;
	}
	if(buffer > (void*)0xbffffff4){
		lock_release(&filesys_lock);
		SYS_Exit(-1);
	}

	struct file *file = find_file(fd);
	if(file == NULL){
		lock_release(&filesys_lock);
		return 0;
	}
	if(buffer == NULL){
		lock_release(&filesys_lock);
		SYS_Exit(-1);
	}
	lock_release(&filesys_lock);
	return file_read(file,buffer,size);

}


static void SYS_Seek(int fd, unsigned position){
	if(fd <=1 )
	   SYS_Exit(-1);
	lock_acquire(&filesys_lock);
	struct file *file = find_file(fd);
	if(file == NULL)/*no file whose fd eaual fd */{
		lock_release(&filesys_lock);
		SYS_Exit(-1);
		}
	lock_release(&filesys_lock);
	file_seek(file,position);
}


/*return the position of the next byte to be read */
static unsigned SYS_Tell(int fd){
	 if(fd <=1 )
           SYS_Exit(-1);
        lock_acquire(&filesys_lock);
        struct file *file = find_file(fd);
        if(file == NULL)/*no file whose fd eaual fd */{
                lock_release(&filesys_lock);
                SYS_Exit(-1);
                }
        lock_release(&filesys_lock);
        return file_tell(file);

}

static void SYS_Close(int fd){
	if(fd <= 1)
		SYS_Exit(-1);
	struct file *file = find_file(fd);
	if(file == NULL)
		SYS_Exit(-1);
	lock_acquire(&filesys_lock);
	 struct list_elem *e;
	  for(e = list_begin(&thread_current()->file_list); e != list_end(&thread_current()->file_list); e = list_next(e)){
         struct file_table * file_t = list_entry(e,struct file_table,file_elem);
        if(file_t->fd == fd){
                file = file_t->file;
		list_remove(&file_t->file_elem);
                break;
}
	}
	file_close(file);
	lock_release(&filesys_lock);
}


/*Read a byte at user virtual address uaddr,uaddr must be blow PHYS_BASE;
  returns the byte value if successfule,-1 if a segfaule occurred */
static int get_user(const uint8_t *uaddr){
	int result ;
	asm("movl $1f, %0; movzbl %1, %0;1:"
	     :"=&a"(result):"m"(*uaddr));
	return result;
	}

/*write BYTE to user address UDST .UDST
  must be blow PHYS_BASE.Returns true if successful,false if a segfault occrred */
static bool put_user(uint8_t *udst,uint8_t byte){
	int error_code;
	asm("movl $1f, %0; movb %b2 , %1;1:"
	    :"=&a"(error_code),"=m"(*udst):"q"(byte));
	return error_code != -1;
	}
