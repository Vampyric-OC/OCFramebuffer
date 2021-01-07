#ifndef _mem_h
#define _mem_h

#include <stddef.h>

#define PAGE 4096

typedef struct {
  size_t size;
  int ready;
  unsigned char buffer[3][80 * 25];
} mem_t;


// Memory block size rounded up to page boundry.
#define MEMSIZE ((sizeof(mem_t) + PAGE - 1) / PAGE) * PAGE

#endif
