#define __int64 long long
#define WIN

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
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

typedef struct _node_t {
  int index;
  uint64_t last_frame;
  mem_t *mem;
  jobjectArray content;
  jobjectArray foreground_color;
  jobjectArray background_color;
#ifdef WIN
  HANDLE handle;
#else  // !WIN (POSIX)
#endif  // WIN
  struct _node_t *next;
} node_t;

node_t *node_list = NULL;

void add_node(node_t *node) {
  // FIXME: THREAD UNSAFE! Switch to CAS? Or Mutex this one? This is internal sync, so mutex or spinlock makes most sense.
  node->next = node_list;
  node_list = node;
}

node_t *find_node(int index) {
  // FIXME: THREAD UNSAFE! Switch to CAS? Or Mutex this one? This is internal sync, so mutex or spinlock makes most sense.
  node_t *node = node_list;

  while (node) {
    if (node->index == index) {
      break;
    }
  }

  if (!node) {
    printf("node: %d not found\n", index);
    return NULL;
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

  node_t *node = NULL;
  if ((node = malloc(sizeof(node_t))) == NULL) {
    printf("node == NULL (%d)\n", (int)GetLastError());
    return -1;
  }

  memset(node, 0, sizeof(node_t));

  node->index = index;
  node->last_frame = 0;
  node->mem = buffer;
  node->content = NULL;
  node->foreground_color = NULL;
  node->background_color = NULL;

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

  if (!CreateProcess(NULL,                                                 // No module name (use command line)
                     "cmd /c D:\\Desktop\\OCFramebuffer\\boxwrap.exe",     // Command line
                     NULL,                                                 // Process handle not inheritable
                     NULL,                                                 // Thread handle not inheritable
                     FALSE,                                                // Set handle inheritance to FALSE
                     0,                                                    // No creation flags
                     NULL,                                                 // Use parent's environment block
                     NULL,                                                 // Use parent's starting directory 
                     &si,                                                  // Pointer to STARTUPINFO structure
                     &pi)) {                                               // Pointer to PROCESS_INFORMATION structure
    printf("CreateProcess failed (%d).\n", (int)GetLastError());
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

JNIEXPORT jboolean JNICALL Java_shmem_get_1next_1frame(JNIEnv *env, jobject thisObj, jlong handle) {
  node_t *node = find_node((int)handle);

  if (!node || !node->mem->ready)
    return JNI_FALSE;

  struct timeval time_struct;
  gettimeofday(&time_struct, NULL);
  uint64_t time = ((time_struct.tv_sec * 1000000) + time_struct.tv_usec);

  if (time < node->last_frame + 50000)    // Max 20 Hz frame rate.
    return JNI_FALSE;
  
  node->last_frame = time;

  // The number 3. Too little to macro, just enough to be irritating in code duplication.
  if (node->content) {
    (*env)->DeleteLocalRef(env, node->content);
    node->content = NULL;
  }

  if (node->foreground_color) {
    (*env)->DeleteLocalRef(env, node->foreground_color);
    node->foreground_color = NULL;
  }

  if (node->background_color) {
    (*env)->DeleteLocalRef(env, node->background_color);
    node->background_color = NULL;
  }

  // FIXME: Switch active buffer with atomics.
  int active_buffer = 0;

  jclass bytebuffer_class = (*env)->FindClass(env, "java/nio/ByteBuffer");

  node->content = (*env)->NewObjectArray(env, 25, bytebuffer_class, NULL);
  node->foreground_color = (*env)->NewObjectArray(env, 25, bytebuffer_class, NULL);
  node->background_color = (*env)->NewObjectArray(env, 25, bytebuffer_class, NULL);

  for (int row = 0; row < 25; row++) {
    (*env)->SetObjectArrayElement(env, node->content, row, (*env)->NewDirectByteBuffer(env, &node->mem->content_buffer[active_buffer][row * 80], 80));
    (*env)->SetObjectArrayElement(env, node->foreground_color, row, (*env)->NewDirectByteBuffer(env, &node->mem->foreground_color_buffer[active_buffer][row * 80], 80 * 4));
    (*env)->SetObjectArrayElement(env, node->background_color, row, (*env)->NewDirectByteBuffer(env, &node->mem->background_color_buffer[active_buffer][row * 80], 80 * 4));
  }

  return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_v_ocframebuffer_shmem_get_1next_1frame(JNIEnv *env, jobject thisObj, jlong handle) {
  return Java_shmem_get_1next_1frame(env, thisObj, handle);
}

JNIEXPORT jobjectArray JNICALL Java_shmem_get_1content_1buffer(JNIEnv *env, jobject thisObj, jlong handle) {
  node_t *node = find_node((int)handle);
  if (!node || !node->content) return NULL;
  jobjectArray content = node->content;
  node->content = NULL;
  return content;
}

JNIEXPORT jobjectArray JNICALL Java_v_ocframebuffer_shmem_get_1content_1buffer(JNIEnv *env, jobject thisObj, jlong handle) {
  return Java_shmem_get_1content_1buffer(env, thisObj, handle);
}

JNIEXPORT jobjectArray JNICALL Java_shmem_get_1foreground_1color_1buffer(JNIEnv *env, jobject thisObj, jlong handle) {
  node_t *node = find_node((int)handle);
  if (!node || !node->foreground_color) return NULL;
  jobjectArray foreground_color = node->foreground_color;
  node->foreground_color = NULL;
  return foreground_color;
}

JNIEXPORT jobjectArray JNICALL Java_v_ocframebuffer_shmem_get_1foreground_1color_1buffer(JNIEnv *env, jobject thisObj, jlong handle) {
  return Java_shmem_get_1foreground_1color_1buffer(env, thisObj, handle);
}

JNIEXPORT jobjectArray JNICALL Java_shmem_get_1background_1color_1buffer(JNIEnv *env, jobject thisObj, jlong handle) {
  node_t *node = find_node((int)handle);
  if (!node || !node->background_color) return NULL;
  jobjectArray background_color = node->background_color;
  node->background_color = NULL;
  return background_color;
}

JNIEXPORT jobjectArray JNICALL Java_v_ocframebuffer_shmem_get_1background_1color_1buffer(JNIEnv *env, jobject thisObj, jlong handle) {
  return Java_shmem_get_1background_1color_1buffer(env, thisObj, handle);
}
