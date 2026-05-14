#include <devices.h>
#include <io.h>
#include <utils.h>
#include <list.h>

// Queue for blocked processes in I/O 
struct circular_buffer cbuffer;

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}

int cbuffer_init()
{
  cbuffer.wr = cbuffer.buffer;
  cbuffer.rd = cbuffer.buffer;
  cbuffer.count = 0;
  return 0;
}

int cbuffer_write(char c) {
  if (cbuffer.count == 1024) 
    return -1; 

  *cbuffer.wr = c;
  cbuffer.wr++;
  if (cbuffer.wr == cbuffer.buffer + 1024)
    cbuffer.wr = cbuffer.buffer;
  cbuffer.count++;
  return 0;
}

char cbuffer_read()
{
  {
    if (cbuffer.count == 0)
      return (char)-1;

    char c = *cbuffer.rd;
    cbuffer.count--;

    cbuffer.rd++;
    if (cbuffer.rd == cbuffer.buffer + 1024)
      cbuffer.rd = cbuffer.buffer;

    return c;
  }
}