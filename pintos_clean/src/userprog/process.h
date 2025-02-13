#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "threads/synch.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

/* Utilities */
struct waiting_obj {
  tid_t tid;
  int exit_code;
  struct condition obj_cond;
  struct list_elem elem;
};

void obj_init(void);
char * copy_string(const char*);
int count_args(const char*);
void parse_args_to_stack(void **, const char *);

#endif /* userprog/process.h */
