#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "devices/shutdown.h"
#include <console.h>
#include "userprog/process.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/palloc.h"
#include "filesys/off_t.h"

typedef int pid_t;

/* lock structure for sychronization */
struct lock lock_file;

struct file 
  {
    struct inode *inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
  };


static void syscall_handler (struct intr_frame *);

void user_addr_check(const void *uaddr);
void syscall_halt(void);
pid_t syscall_exec(const char *cmd_line);
int syscall_wait(pid_t pid);

int syscall_read (int fd, const void *buffer, unsigned size);
int syscall_write (int fd, const void *buffer, unsigned size);
int fibonacci (int n);
int sum_of_four_int (int a, int b, int c, int d);

/* proj 2 */

bool syscall_create (const char *file, unsigned initial_size);
bool syscall_remove (const char *file);
int syscall_open (const char *file);
int syscall_filesize (int fd);
void syscall_seek (int fd, unsigned position);
unsigned syscall_tell (int fd);
void syscall_close (int fd);


void
syscall_init (void) 
{
  lock_init(&lock_file);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int syscall = 0;
  pid_t pid = 0;
  int status = 0;
  const char *cmd_line = {0};
  int fd = 0;
  void *buffer = {0};
  unsigned size = 0;
  int n= 0, a = 0, b= 0, c = 0, d = 0;

  const char *file;
  unsigned initial_size;
  unsigned position;

  syscall = *(int*)f->esp;

  // printf("syscall is %d\n", syscall);
  // hex_dump(f->esp, f->esp, 100, 1);

  switch(syscall){

    case SYS_HALT: /* Terminates Pintos by calling shutdown_power_off() */
      syscall_halt();
      break;

    case SYS_EXIT: /* Terminates the current user program, returning status to the kernel*/
      user_addr_check(f->esp + 4);
      status = *(int*)(f->esp + 4);
      syscall_exit(status);
      break;

    case SYS_EXEC: /* Runs the executable whose name is given in cmd_line, passing any given arguments, and returns the new process's pid. */

      user_addr_check(f->esp + 4);
      cmd_line = *(const char**)(f->esp + 4);
      f->eax = syscall_exec(cmd_line);
      break;

    case SYS_WAIT: /* Waits for a child process pid and retrieves the child's exit status. */

      user_addr_check(f->esp + 4);
      pid = *(pid_t*)(f->esp + 4);
      f->eax = syscall_wait(pid);
      break;

    case SYS_READ: /* Reads size bytes from the file open as fd into buffer */
      user_addr_check(f->esp + 4);
      fd =  *(int*)(f->esp + 4);
      user_addr_check(f->esp + 8);
      buffer = *(void**)(f->esp + 8);
      user_addr_check(f->esp + 12);
      size = *(unsigned*)(f->esp + 12);
      f->eax = syscall_read(fd, buffer, size);
      break;

    case SYS_WRITE:
      user_addr_check(f->esp + 4);
      fd =  *(int*)(f->esp + 4);
      user_addr_check(f->esp + 8);
      buffer = *(void**)(f->esp + 8);
      user_addr_check(f->esp + 12);
      size = *(unsigned*)(f->esp + 12);
      f->eax = syscall_write(fd, buffer, size);

      break; 

    case SYS_FIBO:
  
      user_addr_check(f->esp + 4);
      n = *(int*)(f->esp + 4);
      f->eax = fibonacci(n);
      break;

    case SYS_SUM:
    
      user_addr_check(f->esp + 4);
      a = *(int*)(f->esp + 4);
      user_addr_check(f->esp + 8);
      b = *(int*)(f->esp + 8);
      user_addr_check(f->esp + 12);
      c = *(int*)(f->esp + 12);
      user_addr_check(f->esp + 16);
      d = *(int*)(f->esp + 16);
      f->eax = sum_of_four_int(a, b, c, d);
      break;

    case SYS_CREATE:
      user_addr_check(f->esp + 4);
      file = (const char*)*(uint32_t*)(f->esp + 4);
      user_addr_check(f->esp + 8);
      initial_size = (unsigned)*(uint32_t*)(f->esp + 8);
      f->eax = syscall_create(file, initial_size);
      break;

    case SYS_REMOVE:
      user_addr_check(f->esp + 4);
      file = *(const char **)(f->esp + 4);
      f->eax = syscall_remove(file);
      break;

    case SYS_OPEN:
      user_addr_check(f->esp + 4);
      file = *(const char **)(f->esp + 4);
      f->eax = syscall_open(file);
      break;

    case SYS_FILESIZE:
      user_addr_check(f->esp + 4);
      fd = *(int *)(f->esp + 4);
      f->eax = syscall_filesize(fd);
      break;

    case SYS_SEEK:
      user_addr_check(f->esp + 4);
      fd = *(int *)(f->esp + 4);
      user_addr_check(f->esp + 8);
      position = *(unsigned*)(f->esp + 8);
     syscall_seek(fd, position);
      break;

    case SYS_TELL:
      user_addr_check(f->esp + 4);
      fd = (int)*(uint32_t*)(f->esp + 4);
      f->eax = syscall_tell(fd);
      break;

    case SYS_CLOSE:
      user_addr_check(f->esp + 4);
      fd = *(int *)(f->esp + 4);
      syscall_close(fd);
      break;

  }
}

void
user_addr_check (const void *uaddr)
{
  /* Check if given address is NULL or not */
  if(uaddr == NULL) syscall_exit(-1);
  if(!is_user_vaddr(uaddr)) syscall_exit(-1);
  /* Check if given address is kernel virtual address or not */
  if(is_kernel_vaddr(uaddr)) syscall_exit(-1);

}

void
syscall_halt (void)
{
  shutdown_power_off();
}

pid_t syscall_exec (const char *cmd_line)
{

  return process_execute(cmd_line);
}

int
syscall_wait (pid_t pid)
{

  return process_wait(pid);
}

void syscall_exit(int status){

  struct thread *cur = thread_current();
  int i;

  /* Update the exit_status for parent and child thread to 'status' */
  cur->exit_status = status;
  cur->parent->exit_status_of_child = status;
  for (i = 3; i < 128; i++) {
      if (thread_current()->file_list[i] != NULL) {
          syscall_close(i);
      }   
  }   
  printf("%s: exit(%d)\n", thread_current()->name, status);

  thread_exit();

}

int syscall_read(int fd, const void* buffer, unsigned size){
  int i;
  int readed_byte = -1;

  if(!(0 <= fd && fd < 130)) syscall_exit(-1);
  user_addr_check(buffer);
  struct thread *cur = thread_current();
  struct file *fl = cur->file_list[fd];
  lock_acquire(&lock_file);

  if(fd == 0){
    for(i = 0; i < (int)size; ++i){
      /*Fd 0 reads from the keyboard using input_getc().*/
      *(uint8_t*)(buffer + i) = input_getc();
    }
    readed_byte = i; /* Returns the number of bytes actually read */
  }
  else if(fd > 2){ 
  
    if(fl != NULL){
      readed_byte = file_read(fl, (void*)buffer, size);
    }
    else{
      syscall_exit(-1);
    }
  }
  lock_release(&lock_file);
  return readed_byte;
}

int syscall_write(int fd, const void *buffer, unsigned size){
  struct thread *cur = thread_current();
  struct file *fl = cur->file_list[fd];
  int return_byte = -1;

  if(!(0 <= fd && fd < 130)) syscall_exit(-1);
  user_addr_check(buffer);

  /* write-bad-ptr ; Pass in the pointer to the page directory 
                      and check if this pointer has not been allocated memory for yet.*/
  if(pagedir_get_page(cur->pagedir, buffer) == NULL) syscall_exit(-1);

  lock_acquire(&lock_file);
  if(fd == 1){ 
    putbuf(buffer, size);
    return_byte = size;
  }
  else if (fd > 2){
    
    if(fl == NULL){
      lock_release(&lock_file);
      syscall_exit(-1);
    }
    if(fl->deny_write){
       file_deny_write(cur->file_list[fd]);
    }
    return_byte = file_write(fl, buffer, size);
  }
  lock_release(&lock_file);
  return return_byte;
}

int fibonacci (int n){
  
  int i;
  int x0 = 2, x1 = 1, x2 = 1;
  if(n == 1) return x1;
  if(n == 2) return x2;
  for(i = 0; i < n-2; i++){
      x0 = x1 + x2;
      x2 = x1;
      x1 = x0;
  }
  return x0;
}

int sum_of_four_int (int a, int b, int c, int d){
  
  int sum = a + b + c + d;
  return sum;
}

bool syscall_create (const char *file, unsigned initial_size) {
  if(file == NULL) {
    syscall_exit(-1);
  }
    
  user_addr_check(file);
  lock_acquire(&lock_file);

  /* Creates a new file called file initially initial_size bytes in size. 
      Returns true if successful, false otherwise.*/
  bool isCreated = filesys_create(file, initial_size);
  lock_release(&lock_file);
  return isCreated;
}

bool syscall_remove (const char *file){
  if(file == NULL) syscall_exit(-1);
  user_addr_check(file);
  lock_acquire(&lock_file);

  /* Deletes the file called file. Returns true if successful, false otherwise. */
  bool isRemoved = filesys_remove(file);
  lock_release(&lock_file);
  return isRemoved;
}

int syscall_open (const char *file){
  int fd;
  if(file == NULL) syscall_exit(-1);
  user_addr_check(file);

  lock_acquire(&lock_file);

  /* Opens the file called file. */
  struct file *fl = filesys_open(file);
  if(fl == NULL){
    lock_release(&lock_file);
    return -1;
  }
  struct thread *cur = thread_current();
  cur->file_size++;

  /* Check and prevent writing on execuatble of running thread. */
  if(!strcmp(cur->name, file)) file_deny_write(fl);
  cur->file_list[cur->file_size] = fl;
  fd = cur->file_size;
  lock_release(&lock_file);
  return fd;
}

int syscall_filesize (int fd){
  if(!(0 <= fd && fd < 130)) syscall_exit(-1);
  lock_acquire(&lock_file);
  struct thread *cur = thread_current();
  struct file *fl = cur->file_list[fd];
  if(fl == NULL)
  {
    lock_release(&lock_file);
    syscall_exit(-1);
  }

  /* Returns the size, in bytes, of the file open as fd.*/
  off_t size = file_length(fl);
  lock_release(&lock_file);
  return size;
}

void syscall_seek (int fd, unsigned position){
  
  if(!(0 <= fd && fd < 130)) syscall_exit(-1);
  lock_acquire(&lock_file);
  struct thread *cur = thread_current();
  struct file *fl = cur->file_list[fd];
  if(fl == NULL){
    lock_release(&lock_file);
    syscall_exit(-1);
  }

  /* Changes the next byte to be read or written in open file fd to position,
       expressed in bytes from the beginning of the file.
       (Thus, a position of 0 is the file’s start.) */
  file_seek(fl, position);
  lock_release(&lock_file);

}

unsigned syscall_tell (int fd){
  
  if(!(0 <= fd && fd < 130)) syscall_exit(-1);
  lock_acquire(&lock_file);
  struct thread *cur = thread_current();
  struct file *fl = cur->file_list[fd];
  if(fl == NULL){
    lock_release(&lock_file);
    syscall_exit(-1);
  }
  /* Returns the position of the next byte to be read or written in open file fd,
     expressed in bytes from the beginning of the file. */
  lock_release(&lock_file);
  return file_tell(fl);
}

void 
syscall_close (int fd){
  
  if(!(0 <= fd && fd < 130)) syscall_exit(-1);
  lock_acquire(&lock_file);
  struct thread *cur = thread_current();
  struct file *fl = cur->file_list[fd];
  if(fl == NULL){
    lock_release(&lock_file);
    syscall_exit(-1);
  }
  
  /* Closes file descriptor fd. Exiting or terminating a process implicitly
     closes all its open file descriptors, as if by calling this function for each one. */
  cur->file_list[fd] = NULL;
  lock_release(&lock_file);

  return file_close(fl);
}

