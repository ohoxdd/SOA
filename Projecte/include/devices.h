#ifndef DEVICES_H__
#define  DEVICES_H__

int sys_write_console(char *buffer, int size);

struct circular_buffer {
char buffer[1024];
char *wr;
char *rd;
int count;
};

extern struct circular_buffer cbuffer;

int cbuffer_init(void);
int cbuffer_write(char c);
char cbuffer_read(void);

#endif /* DEVICES_H__*/
