/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_elastos_spvcore_CoreMerkleBlock */

#ifndef _Included_com_elastos_spvcore_CoreMerkleBlock
#define _Included_com_elastos_spvcore_CoreMerkleBlock
#ifdef __cplusplus
extern "C" {
#endif
	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    createJniCoreMerkleBlock
	 * Signature: ([BI)J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_createJniCoreMerkleBlock
		(JNIEnv *, jclass, jbyteArray, jint);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    createJniCoreMerkleBlockEmpty
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_createJniCoreMerkleBlockEmpty
		(JNIEnv *, jclass);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getBlockHash
	 * Signature: ()[B
	 */
	JNIEXPORT jbyteArray JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getBlockHash
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getVersion
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getVersion
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getPrevBlockHash
	 * Signature: ()[B
	 */
	JNIEXPORT jbyteArray JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getPrevBlockHash
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getRootBlockHash
	 * Signature: ()[B
	 */
	JNIEXPORT jbyteArray JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getRootBlockHash
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getTimestamp
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getTimestamp
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getTarget
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getTarget
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getNonce
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getNonce
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getTransactionCount
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getTransactionCount
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    getHeight
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_getHeight
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    serialize
	 * Signature: ()[B
	 */
	JNIEXPORT jbyteArray JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_serialize
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    isValid
	 * Signature: (J)Z
	 */
	JNIEXPORT jboolean JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_isValid
		(JNIEnv *, jobject, jlong);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    containsTransactionHash
	 * Signature: ([B)Z
	 */
	JNIEXPORT jboolean JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_containsTransactionHash
		(JNIEnv *, jobject, jbyteArray);

	/*
	 * Class:     com_elastos_spvcore_CoreMerkleBlock
	 * Method:    disposeNative
	 * Signature: ()V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreMerkleBlock_disposeNative
		(JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif