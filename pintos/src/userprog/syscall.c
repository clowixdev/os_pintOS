#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

//! LAB4 S
static void
syscall_handler (struct intr_frame *f) 
{
  int SYS_CALL = *(int*)f->esp;
  int* args = (int*)f->esp + 1;

  switch (SYS_CALL) {
    case SYS_EXIT:
      thread_current() -> exit_code = args[0];
      thread_exit();
      break;

    case SYS_WRITE:
      putbuf(args[1], args[2]);
      return;
    
    default:
      printf ("unhandled system call! code(%d)\n", SYS_CALL);
      break;
  }
}
//! LAB4 E