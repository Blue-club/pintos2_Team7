#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>
#include <debug.h>
#include <stddef.h>

/* Project 2. */
#include <threads/thread.h>
#include <userprog/process.h>
/* Project 2. */

void syscall_init (void);

/* Project 2. */
void check_address (void *);
void halt (void);
void exit (int);
pid_t fork (char *, struct intr_frame *);
int exec (const char *);
int wait (pid_t);
bool create (const char *, unsigned);
bool remove (const char *);
int open (const char *);
int filesize (int);
int read (int, void *, unsigned);
int write (int, const void *, unsigned);
void seek (int, unsigned);
unsigned tell (int);
void close (int);

//p3
struct lock filesys_lock;
void *mmap(void *addr, size_t length, int writable, int fd, off_t offset);
void munmap(void *addr);

#endif /* userprog/syscall.h */
