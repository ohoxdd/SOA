/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

void itoa(int a, char *b);

extern int errno;
int write(int fd, char *buffer, int size);
void perror(void);

int strlen(char *a);

#endif  /* __LIBC_H__ */
