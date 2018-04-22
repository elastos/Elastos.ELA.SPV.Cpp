/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_elastos_spvcore_CoreTransaction */

#ifndef _Included_com_elastos_spvcore_CoreTransaction
#define _Included_com_elastos_spvcore_CoreTransaction
#ifdef __cplusplus
extern "C" {
#endif
#undef com_elastos_spvcore_CoreTransaction_JNI_COPIES_TRANSACTIONS
#define com_elastos_spvcore_CoreTransaction_JNI_COPIES_TRANSACTIONS 1L
	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getHash
	 * Signature: ()[B
	 */
	JNIEXPORT jbyteArray JNICALL Java_com_elastos_spvcore_CoreTransaction_getHash
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getVersion
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_getVersion
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getInputs
	 * Signature: ()[Lcom/breadwallet/core/BRCoreTransactionInput;
	 */
	JNIEXPORT jobjectArray JNICALL Java_com_elastos_spvcore_CoreTransaction_getInputs
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getOutputs
	 * Signature: ()[Lcom/breadwallet/core/BRCoreTransactionOutput;
	 */
	JNIEXPORT jobjectArray JNICALL Java_com_elastos_spvcore_CoreTransaction_getOutputs
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getLockTime
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_getLockTime
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    setLockTime
	 * Signature: (J)V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreTransaction_setLockTime
		(JNIEnv *, jobject, jlong);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getBlockHeight
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_getBlockHeight
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getTimestamp
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_getTimestamp
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    setTimestamp
	 * Signature: (J)V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreTransaction_setTimestamp
		(JNIEnv *, jobject, jlong);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    serialize
	 * Signature: ()[B
	 */
	JNIEXPORT jbyteArray JNICALL Java_com_elastos_spvcore_CoreTransaction_serialize
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    addInput
	 * Signature: (Lcom/breadwallet/core/BRCoreTransactionInput;)V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreTransaction_addInput
		(JNIEnv *, jobject, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    addOutput
	 * Signature: (Lcom/breadwallet/core/BRCoreTransactionOutput;)V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreTransaction_addOutput
		(JNIEnv *, jobject, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    shuffleOutputs
	 * Signature: ()V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreTransaction_shuffleOutputs
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getSize
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_getSize
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getStandardFee
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_getStandardFee
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    isSigned
	 * Signature: ()Z
	 */
	JNIEXPORT jboolean JNICALL Java_com_elastos_spvcore_CoreTransaction_isSigned
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    sign
	 * Signature: ([Lcom/breadwallet/core/BRCoreKey;I)V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreTransaction_sign
		(JNIEnv *, jobject, jobjectArray, jint);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    isStandard
	 * Signature: ()Z
	 */
	JNIEXPORT jboolean JNICALL Java_com_elastos_spvcore_CoreTransaction_isStandard
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getReverseHash
	 * Signature: ()Ljava/lang/String;
	 */
	JNIEXPORT jstring JNICALL Java_com_elastos_spvcore_CoreTransaction_getReverseHash
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    getMinOutputAmount
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_getMinOutputAmount
		(JNIEnv *, jclass);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    disposeNative
	 * Signature: ()V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreTransaction_disposeNative
		(JNIEnv *, jobject);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    initializeNative
	 * Signature: ()V
	 */
	JNIEXPORT void JNICALL Java_com_elastos_spvcore_CoreTransaction_initializeNative
		(JNIEnv *, jclass);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    createJniCoreTransaction
	 * Signature: ([BJJ)J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_createJniCoreTransaction
		(JNIEnv *, jclass, jbyteArray, jlong, jlong);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    createJniCoreTransactionSerialized
	 * Signature: ([B)J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_createJniCoreTransactionSerialized
		(JNIEnv *, jclass, jbyteArray);

	/*
	 * Class:     com_elastos_spvcore_CoreTransaction
	 * Method:    createJniCoreTransactionEmpty
	 * Signature: ()J
	 */
	JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CoreTransaction_createJniCoreTransactionEmpty
		(JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
