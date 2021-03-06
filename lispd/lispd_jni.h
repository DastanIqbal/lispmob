#include <jni.h>
/* Header for class org_lispmob_noroot_LISPmob_JNI */

#ifndef _Included_org_lispmob_noroot_LISPmob_JNI
#define _Included_org_lispmob_noroot_LISPmob_JNI
#ifdef __cplusplus
extern "C" {
#endif

/*
* Class: org_lispmob_noroot_LISPmob_JNI
* Method: startLispd
* Signature: (I)[I
*/
JNIEXPORT jintArray JNICALL Java_org_lispmob_noroot_LISPmob_1JNI_startLispd
  (JNIEnv * env, jclass cl, jint vpn_tun_fd, jstring storage_path);

/*
* Class: org_lispmob_noroot_LISPmob_JNI
* Method: lispd_loop
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_lispmob_noroot_LISPmob_1JNI_lispd_1loop
  (JNIEnv * env, jclass cl);

/*
* Class: org_lispmob_noroot_LISPmob_JNI
* Method: lispd_exit
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_lispmob_noroot_LISPmob_1JNI_lispd_1exit
   (JNIEnv * env, jclass cl);


#ifdef __cplusplus
}
#endif
#endif

