/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class shmem */

#ifndef _Included_shmem
#define _Included_shmem
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     shmem
 * Method:    get_handle
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_shmem_get_1handle
  (JNIEnv *, jobject);

/*
 * Class:     shmem
 * Method:    get_next_frame
 * Signature: (J)Ljava/nio/ByteBuffer;
 */
JNIEXPORT jobject JNICALL Java_shmem_get_1next_1frame
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif