#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>

#include "mem.h"

#include <windows.h>

DWORD WINAPI thread(LPVOID lpParam) {
  mem_t *mem = NULL;
  const char *path = getenv("SHMEM_ID");
  // Value for DOSBox 0.74-3.
  // Name: DOSBox.exe
  // Size: 3745792 bytes (3658 KiB)
  // SHA1: 5C5C465B48D09C0586EE465DFE5DF0CB0C1C1028
  short **videomem = (short **)0x01D074D4;

  printf("Thread started.\n");

  HANDLE handle = CreateFileMapping(INVALID_HANDLE_VALUE,    // use paging file
                                    NULL,                    // default security
                                    PAGE_READWRITE,          // read/write access
                                    0,                       // maximum object size (high-order DWORD)
                                    MEMSIZE,                 // maximum object size (low-order DWORD)
                                    path);                   // name of mapping object

  if (handle == NULL) {
    printf("Handle == NULL (%d)\n", (int)GetLastError());
    return 0;
  }

  mem = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

  if (mem == NULL) {
    printf("configuration == NULL (%d)\n", (int)GetLastError());
    return 0;
  }

  printf("Thread sleeping.\n");
  sleep(2);

  memset(mem->content_buffer[0], ' ', 80 * 25);
  //memset(mem->foreground_color_buffer[0], 0xaa, 80 * 25 * 4);  // Please ignore Alpha value ;-)
  //memset(mem->background_color_buffer[0], 0, 80 * 25 * 4);
  mem->ready = 1;

  printf("Thread ready.\n");

//  int vga_color_map[] = {
//    0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
//    0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
//  };

  // Big endian. Because Java...
  int vga_color_map[] = {
    0x00000000, 0xAA000000, 0x00AA0000, 0xAAAA0000, 0x0000AA00, 0xAA00AA00, 0x0055AA00, 0xAAAAAA00,
    0x55555500, 0xFF555500, 0x55FF5500, 0xFFFF5500, 0x5555FF00, 0xFF55FF00, 0x55FFFF00, 0xFFFFFF00
  };

  while (1) {
    if (*videomem != NULL) {
      for (int i = 0; i < 80 * 25; i++) {
        mem->content_buffer[0][i] = (*videomem)[i] & 0xff;
        // If we can load in the VGA palette into OC, we could pass nibbles directly or even the full buffer as is.
        // Would still need to have a call to set TextBuffer raw with text and color at the same time.
        //mem->foreground_color_buffer[0][i] = __builtin_bswap32(((*videomem)[i] & 0x0f00) >> 8);
        //mem->background_color_buffer[0][i] = __builtin_bswap32((*videomem)[i] >> 12);
        mem->foreground_color_buffer[0][i] = vga_color_map[((*videomem)[i] & 0x0f00) >> 8];
        mem->background_color_buffer[0][i] = vga_color_map[(*videomem)[i] >> 12];
      }
    }
    // Sleep to target 60 hz.
  }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  static int initialized = 0;

  if (fdwReason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hinstDLL);
  }

  if (initialized) {
    return TRUE;
  }

  DWORD dwThreadId;
  CreateThread(NULL, 0, thread, NULL, 0, &dwThreadId);
  initialized = 1;

  return TRUE;
}
