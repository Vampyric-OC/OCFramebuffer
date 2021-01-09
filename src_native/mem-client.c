#define WIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN
#include <windows.h>
#else  // !WIN (POSIX)
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#endif  // WIN

#include "mem.h"

int main(int argc, char *argv[]) {
  mem_t *mem = NULL;
  const char *path = getenv("SHMEM_ID");

#ifdef WIN
  HANDLE handle;

  handle = CreateFileMapping(INVALID_HANDLE_VALUE,    // use paging file
                             NULL,                    // default security
                             PAGE_READWRITE,          // read/write access
                             0,                       // maximum object size (high-order DWORD)
                             MEMSIZE,                    // maximum object size (low-order DWORD)
                             path);                   // name of mapping object

  if (handle == NULL) {
    printf("Handle == NULL (%d)\n", (int)GetLastError());
    return -1;
  }

  mem = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

  if (mem == NULL) {
    printf("configuration == NULL (%d)\n", (int)GetLastError());
    return -1;
  }
#else  // !WIN (POSIX)
  // TBD.
#endif  // WIN

  memset(mem->content_buffer[0], ' ', sizeof(mem->content_buffer[0]));
  mem->ready = 1;

  while (1) {
    for (int i = 0; i < 80 * 25; i++) {
      memset(mem->content_buffer[0], ' ', sizeof(mem->content_buffer[0]));
      mem->content_buffer[0][i] = '.';
    }
  }

  // So lazy again....
  system("cmd /c pause");
  return 0;
}
