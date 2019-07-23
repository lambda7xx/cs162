#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "filesys/file.h"
#include <list.h>
void syscall_init (void);
void SYS_Exit(int);
int SYS_Open(const char * file);/*Open a file */
struct file_table{
 struct list_elem  file_elem;//to insert into the thread's file_list 
 int fd;
 struct file *file;
};


int insert_file_to_thread(struct file * file);
//ssize_t sys_write(int fd,const void * buf,size_t count);
#endif /* userprog/syscall.h */
