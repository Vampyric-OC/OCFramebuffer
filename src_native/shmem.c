#define __int64 long long
#define WIN

#include <jni.h>
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

#include "shmem.h"
#include "mem.h"

typedef struct _handle_list {
  int index;
  mem_t *mem;
#ifdef WIN
  HANDLE handle;
#else  // !WIN (POSIX)
#endif  // WIN
  struct _handle_list *next;
} handle_list_t;

handle_list_t *handle_list = NULL;

void add_node(handle_list_t *node) {
  // FIXME: THREAD UNSAFE! Switch to CAS? Or Mutex this one? This is internal sync, so mutex or spinlock makes most sense.
  node->next = handle_list;
  handle_list = node;
}

handle_list_t *find_node(int index) {
  // FIXME: THREAD UNSAFE! Switch to CAS? Or Mutex this one? This is internal sync, so mutex or spinlock makes most sense.
  handle_list_t *node = handle_list;

  while (node) {
    if (node->index == index) {
      break;
    }
  }

  return node;
}

void remove_node(int index) {
  // FIXME: TBD.
}

// FIXME: Access to the shared memory blocks is completely open, 'might' want to add some security.
JNIEXPORT jlong JNICALL Java_shmem_get_1handle(JNIEnv *env, jobject thisObj) {
  char path[128];
  static int index = 0;

#ifdef WIN
  snprintf(path, sizeof(path), "shmem-%d-%d", getpid(), index);

  HANDLE handle;

  handle = CreateFileMapping(INVALID_HANDLE_VALUE,    // use paging file
                             NULL,                    // default security
                             PAGE_READWRITE,          // read/write access
                             0,                       // maximum object size (high-order DWORD)
                             MEMSIZE,                 // maximum object size (low-order DWORD)
                             path);                   // name of mapping object

  if (handle == NULL) {
    printf("Handle == NULL (%d)\n", (int)GetLastError());
    return -1;
  }

  void *buffer = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

  if (buffer == NULL) {
    printf("buffer == NULL (%d)\n", (int)GetLastError());
    return -1;
  }
#else  // !WIN (POSIX)
  snprintf(path, sizeof(path), "shmem_%d", getpid());
  key_t key = ftok(path, index);

  shmget(key, MEMSIZE, IPC_CREAT | IPC_EXCL | 0777);
  void *buffer = shmat(key, NULL, 0);
#endif  // WIN

  ((mem_t *)buffer)->size = MEMSIZE;
  memset(buffer, 0, ((mem_t *)buffer)->size);

  handle_list_t *node = NULL;
  if ((node = malloc(sizeof(handle_list_t))) == NULL) {
    printf("node == NULL (%d)\n", (int)GetLastError());
    return -1;
  }

  memset(node, 0, sizeof(handle_list_t));

  node->index = index;
  node->mem = buffer;

#ifdef WIN
  node->handle = handle;
#else  // !WIN (POSIX)
#endif  // WIN

  add_node(node);

#ifdef WIN
  _putenv_s("SHMEM_ID", path);

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;
//  si.wShowWindow = SW_SHOWMINNOACTIVE;

  if (!CreateProcess(NULL,                       // No module name (use command line)
                     "cmd /c mem-client.exe",    // Command line
                     NULL,                       // Process handle not inheritable
                     NULL,                       // Thread handle not inheritable
                     FALSE,                      // Set handle inheritance to FALSE
                     0,                          // No creation flags
                     NULL,                       // Use parent's environment block
                     NULL,                       // Use parent's starting directory 
                     &si,                        // Pointer to STARTUPINFO structure
                     &pi)) {                     // Pointer to PROCESS_INFORMATION structure
    printf( "CreateProcess failed (%d).\n", (int)GetLastError());
  }

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
#else  // !WIN (POSIX)
  setenv("SHMEM_ID", path, 1);
  // FIXME: fork & exec.
  //        Is forking the whole minecraft process a good idea? Linux PTs are effective, but 6GB in <1ms effective?
  //        Probably better to fork once and exec a small launcher with pipe, which forks and parents everything else.
#endif  // WIN

  return index++;
}

JNIEXPORT jlong JNICALL Java_v_ocframebuffer_shmem_get_1handle(JNIEnv *env, jobject thisObj) {
  return Java_shmem_get_1handle(env, thisObj);
}

JNIEXPORT jobject JNICALL Java_shmem_get_1next_1frame(JNIEnv *env, jobject thisObj, jlong handle) {
  handle_list_t *node = find_node((int)handle);

  if (!node) {
    printf("index: %d not found\n", (int)handle);
    return NULL;
  }

  if (!node->mem->ready) {
    return NULL;
  }

  return (*env)->NewDirectByteBuffer(env, ((char *)node->mem->buffer[0]), 80 * 25);
}

JNIEXPORT jobject JNICALL Java_v_ocframebuffer_shmem_get_1next_1frame(JNIEnv *env, jobject thisObj, jlong handle) {
  return Java_shmem_get_1next_1frame(env, thisObj, handle);
}
