#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "userprog/exception.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"

#include "userprog/process.h"

#define STDOUT 1
#define STDIN 0

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  obj_init ();
}

//! LAB 4-5 S
int exit_with_code(int code) {
  thread_current()->exit_code = code;
  thread_exit();

  return code;
}

int validate_user_pointer(void *pointer) {
  if (!(is_user_vaddr(pointer) && 
        pagedir_get_page(thread_current()->pagedir, pointer))) {
    return exit_with_code(-1);
  }
}

int compare_fd(struct list_elem *f_elem, int fd) {
  return (list_entry(f_elem, struct thread_fd, elem)->fd == fd) ? true : false;
}

int get_file(int fd) {
  struct list * fd_list = &(thread_current()->thread_fd);
  struct list_elem *thread_fd_elem = list_begin(fd_list);

  if (compare_fd(thread_fd_elem, fd)) {
    return list_entry(thread_fd_elem, struct thread_fd, elem);
  } else if (thread_fd_elem->next == NULL) {
    return NULL;
  } else {
    thread_fd_elem = list_exact(fd_list, &compare_fd, fd);
  }

  if (thread_fd_elem == NULL) {
    return -1;
  }

  return list_entry(thread_fd_elem, struct thread_fd, elem);
}

static void
syscall_handler (struct intr_frame *f) 
{
  validate_user_pointer((const void*) f->esp);

  int SYS_CALL = *(int*)f->esp;
  int* args = (int*)f->esp + 1;

  switch (SYS_CALL) {
    case SYS_HALT: //* void halt (void);
      handle_halt(); //! +
      break;

    case SYS_EXIT: //* void exit (int status);
      handle_exit(args[0]); //! +
      break;

    case SYS_EXEC: //* pid_t exec (const char *cmd_line);
      f->eax = handle_exec(args[0]); //! +
      break;

    case SYS_WAIT: //* int wait (pid_t pid);
      f->eax = handle_wait(args[0]); //! +
      break;

    case SYS_CREATE: //* bool create (const char *file, unsigned initial_size);
      f->eax = handle_create(args[0], args[1]); //! +
      break;

    case SYS_REMOVE: //* bool remove (const char *file);
      f->eax = handle_remove(args[0]); //~
      break;

    case SYS_OPEN: //* int open (const char *file);
      f->eax = handle_open(args[0]); //! +
      break;

    case SYS_FILESIZE: //* int filesize (int fd);
      f->eax = handle_filesize(args[0]); //~
      break;

    case SYS_READ: //* int read (int fd, void* buffer, unsigned size);
      f->eax = handle_read(args[0], args[1], args[2]); //! +
      break;

    case SYS_WRITE: //? int write (int fd, const void *buffer, unsigned size);
      f->eax = handle_write(args[0], args[1], args[2]); //! +
      break;

    case SYS_CLOSE: //* void close (int fd);
      handle_close(args[0]); //! +
      break;

    default:
      printf ("unhandled system call! code(%d)\n", SYS_CALL);
      break;
  }
}

//? void halt (void);
void handle_halt() {
  shutdown();
}

//? void exit (int status);
void handle_exit(int status) {
  thread_current() -> exit_code = status;
  thread_exit();
}

//? pid_t exec (const char *cmd_line);
int handle_exec(const char *cmd_line) {
  validate_user_pointer(cmd_line);

  printf("NAME: %s\n", cmd_line);

  tid_t tid = process_execute(cmd_line);

  return tid;
}

//? int wait (pid_t pid);
int handle_wait(int pid) {
  return process_wait(pid);
}

//? bool create (const char *file, unsigned initial_size);
bool handle_create(const char *file, unsigned initial_size) {
  validate_user_pointer(file);
  return filesys_create(file, initial_size);
}

//? bool remove (const char *file);
bool handle_remove(const char *file) {  
  validate_user_pointer(file);
  return filesys_remove(file);
}

//? int open (const char *file);
int handle_open(const char *filename) {
  validate_user_pointer(filename);
  struct file* thread_file = filesys_open(filename);
  if (thread_file == NULL) {
    return -1;
  }
  struct thread_fd *thread_file_elem = calloc(1, sizeof(struct thread_fd));

  thread_file_elem->file = thread_file;
  thread_file_elem->fd = thread_current()->fd++;
  list_push_back(&thread_current()->thread_fd, &thread_file_elem->elem);

  return thread_file_elem->fd;
}

//? int filesize (int fd);
int handle_filesize(int fd) {
  struct thread_fd *thread_file_elem = get_file(fd);
  if (thread_file_elem == NULL) {
    return -1;
  }

  return file_length(thread_file_elem->file);
}

//? int read (int fd, void* buffer, unsigned size);
int handle_read(int fd, uint8_t* buffer, unsigned size) {
  validate_user_pointer(buffer);

  if (fd == STDIN) {
    while (size > 0) {
      *buffer = input_getc();

      *buffer++;
      size--;
    }
  } else {
    struct thread_fd* thread_file_elem = get_file(fd);
    if (thread_file_elem == NULL) {
      return -1;
    }
    size = file_read(thread_file_elem->file, (void *)buffer, size);
  }

  return size;
}

//? int write (int fd, const void *buffer, unsigned size);
int handle_write(int fd, const void *buffer, unsigned size) {
  validate_user_pointer(buffer);

  if (fd == STDOUT) {
    putbuf((const char*)buffer, size);
  } else {
    struct thread_fd* thread_fd_elem = get_file(fd);
    if (thread_fd_elem == NULL) {
      return -1;
    }
    size = file_write(thread_fd_elem->file, buffer, size);
  }
  
  return size;
}

//? void close (int fd);
void handle_close(int fd) {
  if (fd < 2) {
    exit_with_code(-1);
  }
  struct thread_fd *thread_file_elem = get_file(fd);

  if (thread_file_elem == NULL) {
    return;
  }

  list_remove(&thread_file_elem->elem);
  file_close(thread_file_elem->file);
  free(thread_file_elem);
}

//! LAB 4-5 E