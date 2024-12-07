#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

//! LAB 4 S
/* UPL utility */
struct user_pl * init_UPL(bool, tid_t);
void add_to_UPL(struct user_pl *, tid_t);
void remove_from_UPL(struct user_pl *, tid_t);
void show_UPL(struct user_pl *);
bool in_UPL(struct user_pl *, tid_t);

/* Utilities */
char *copy_string(const char*);
int count_args(const char*);
void parse_args_to_stack(void **, const char *);
//! LAB 4 E

#endif /* userprog/process.h */
