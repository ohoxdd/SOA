/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

void itoa(int a, char *b);

extern int errno;
int write(int fd, char *buffer, int size);
int gettime(void);
void perror(void);
int getpid(void);
int fork(void);
void exit(void);
void block(void);
int unblock(int pid);
int read(char* b, int maxchar);

int strlen(char *a);

#endif  /* __LIBC_H__ */
