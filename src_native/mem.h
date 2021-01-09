#ifndef _mem_h
#define _mem_h

#include <stdint.h>

#define PAGE 4096

// Define exact variable sizes and pack the struct. This code is used in both 32 and 64 bit.
typedef struct {
  int64_t size;
  int8_t ready;
  uint8_t content_buffer[3][80 * 25];
  int32_t foreground_color_buffer[3][80 * 25];
  int32_t background_color_buffer[3][80 * 25];
} __attribute__((packed)) mem_t;


// Memory block size rounded up to page boundry.
#define MEMSIZE ((sizeof(mem_t) + PAGE - 1) / PAGE) * PAGE

#endif
