#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/vaddr.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h" /* for power_off */
#include "process.h" /* for exec, wait*/
#include "filesys/filesys.h"
#include "filesys/file.h" /* for create, remove, open*/
#include "lib/kernel/list.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  init_synch();
}

void
check_ptr (void *pointer)
{
  if (!(is_user_vaddr(pointer) && pagedir_get_page(thread_current()->pagedir, pointer))) 
    {
      thread_current()->exit = -1;
      thread_exit();
    }
}

int 
com_fd (struct list_elem *fd_elem, int fd) 
{
  return (list_entry(fd_elem, struct files, elem)->fd == fd) ? true : false;
}

int 
file_req (int fd) 
{
  struct list *fd_list = &thread_current()->files;
  struct list_elem *fd_elem = list_begin(fd_list);

  if (com_fd(fd_elem, fd)) 
    {
      return list_entry(fd_elem, struct files, elem);
    } 
  else if (fd_elem->next == NULL) return NULL;
  else
    {
      fd_elem = list_exact(fd_list, &com_fd, &fd);

      if (fd_elem == NULL) return -1; 

      return list_entry(fd_elem, struct files, elem);
    }
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // //! HANDLER (lab4)
  // int *args = (int*)f->esp + 1;
  // if (*(int*)f->esp == SYS_EXIT)
  //   {
  //     thread_current()->exit = args[0];
  //     thread_exit();
  //     return;
  //   }
  // else if (*(int*)f->esp == SYS_WRITE)
  //   {
  //     putbuf(args[1], args[2]);
  //     return;
  //   }
  // else
  //   {
  //     printf ("system call!\n");
  //     thread_exit ();
  //   }


  //! NEW handler (lab5)
  int sys_inter_code = *(int *)f->esp;
  int *args = (int *)f->esp + 1;
  struct files *files_elem;

  switch (sys_inter_code)
    {
      case SYS_HALT:
        shutdown();
        break;
      case SYS_EXIT:
      check_ptr(args[0]);
        thread_current()->exit = args[0];
        thread_exit();
        break;
      case SYS_EXEC:
        check_ptr(args[0]);
        f->eax = process_execute(args[0]);
        break;
      case SYS_WAIT:
        check_ptr(args[0]);
        f->eax = process_wait(args[0]);
        break;
      case SYS_CREATE:
        check_ptr(args[0]);
        check_ptr(args[1]);
        f->eax = filesys_create(args[0], args[1]);
        break;
      case SYS_REMOVE:
        check_ptr(args[0]);
        f->eax = filesys_remove(args[0]);
        break;
      case SYS_OPEN:
        check_ptr(args[0]);
        struct file *opened_file = filesys_open(args[0]);
        if (opened_file == NULL) f->eax = -1;
        else
          {
            files_elem = (struct files *) malloc(sizeof(struct files));
            files_elem->opened = opened_file;
            files_elem->fd = thread_current()-> tid;
            list_push_back(&thread_current()->fd, &files_elem->elem);
            f->eax = files_elem->fd;
          }
        break;
      case SYS_FILESIZE:
        files_elem = file_req(args[0]);
        if (files_elem == NULL) f->eax = -1;
        else f->eax = file_length(files_elem->opened);
        break;
      case SYS_CLOSE:
        if (args[0] < 2)
          {
            thread_current()->exit = -1;
            thread_exit();
          }
        files_elem = file_req(args[0]);
        if (files_elem == NULL) break;
        list_remove(&files_elem->elem);
        file_close(files_elem->opened);
        free(files_elem);
        break;
      case SYS_READ:
        check_ptr(args[1]);
        if (args[0] == 0)
          {
            args[1] = input_getc();
            args[1]++;
            args[2]--;
          }
        else
          {
            files_elem = file_req(args[0]);
            if (files_elem == NULL) f->eax = -1;
            else f->eax = file_read(files_elem->opened, (void *)args[1], args[2]);
            break;
          }
        f->eax = args[2];
        break;
      case SYS_WRITE:
        check_ptr(args[1]);
        if (args[0] == 1) putbuf((const char *)args[1], args[2]);
        else
          {
            files_elem = file_req(args[0]);
            if (files_elem == NULL) f->eax = -1;
            else f->eax = file_write(files_elem->opened, (void *)args[1], args[2]);
            break;
          }
        f->eax = args[2];
        break;
    }
}
