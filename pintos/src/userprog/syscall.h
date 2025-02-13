#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <list.h>
#include "filesys/file.h"

extern struct lock filesys_lock;
extern struct condition filesys_cond;

struct thread_fd
{
    int fd;
    struct file* file;
    struct list_elem elem;
};

int exit_with_code(int);
int validate_user_pointer(void*);
int get_file(int);
int compare_fd(struct list_elem *, int);
void syscall_init (void);

void handle_close(int);
int handle_write(int, const void *, unsigned);
int handle_read(int, uint8_t *, unsigned);
int handle_filesize(int);
int handle_open(const char *);
bool handle_remove(const char *);
bool handle_create(const char *, unsigned);
int handle_wait(int);
int handle_exec(const char *);
void handle_exit(int);
void handle_halt(void);
#endif /* userprog/syscall.h */
