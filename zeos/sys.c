/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int zeos_ticks;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_write(int fd, char *buffer, int size) {
	int err;
	char sys_buffer[256];
	int bytes_left;
	int ret = 0;
	
	// CHECKS -- comprobar datos del usuario
	err = check_fd(fd, ESCRIPTURA); // solo escritura
	if (err < 0) return err;	// fd invalido

	if (buffer == NULL) return -14; // -EFAULT bad address
	if (size < 0) return -22;		// -EINVAL invalid argument
	
	bytes_left = size;

	while (bytes_left > 0) {
		int to_copy = (bytes_left > sizeof(sys_buffer)) ? sizeof(sys_buffer) : bytes_left;

		err = copy_from_user(buffer + ret, sys_buffer, to_copy);
		if (err < 0) return err;
		
		err = sys_write_console(sys_buffer, to_copy);
		if (err < 0) return err;

		ret += err;
		bytes_left -= err;
	}

	return ret;
}

int sys_gettime() {
	return zeos_ticks;
}
