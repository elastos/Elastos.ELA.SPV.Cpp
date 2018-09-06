//
//  BRBIP32Sequence.c
//
//  Created by Aaron Voisine on 8/19/15.
//  Copyright (c) 2015 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "BRBIP32Sequence.h"
#include "BRCrypto.h"
#include <string.h>
#include <assert.h>
#include <cmake-build-debug/ThirdParty/openssl/install/include/openssl/ec.h>
#include <cmake-build-debug/ThirdParty/openssl/install/include/openssl/obj_mac.h>

#define BIP32_SEED_KEY "Bitcoin seed"
#define BIP32_XPRV     "\x04\x88\xAD\xE4"
#define BIP32_XPUB     "\x04\x88\xB2\x1E"

static int _TweakSecret(unsigned char vchSecretOut[32], const unsigned char vchSecretIn[32],
						const unsigned char vchTweak[32], int nid) {
	BN_CTX *ctx = BN_CTX_new();
	BN_CTX_start(ctx);
	BIGNUM *bnSecret = BN_CTX_get(ctx);
	BIGNUM *bnTweak = BN_CTX_get(ctx);
	BIGNUM *bnOrder = BN_CTX_get(ctx);
	EC_GROUP *group = EC_GROUP_new_by_curve_name(nid);
	EC_GROUP_get_order(group, bnOrder,
					   ctx); // what a grossly inefficient way to get the (constant) group order...
	BN_bin2bn(vchTweak, 32, bnTweak);
	if (BN_cmp(bnTweak, bnOrder) >= 0)
		return -1; // extremely unlikely
	BN_bin2bn(vchSecretIn, 32, bnSecret);
	BN_add(bnSecret, bnSecret, bnTweak);
	BN_nnmod(bnSecret, bnSecret, bnOrder, ctx);
	if (BN_is_zero(bnSecret))
		return -1; // ridiculously unlikely
	int nBits = BN_num_bits(bnSecret);
	memset(vchSecretOut, 0, 32);
	BN_bn2bin(bnSecret, &vchSecretOut[32 - (nBits + 7) / 8]);
	EC_GROUP_free(group);
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	return 0;
}

void getPubKeyFromPrivKey(void *brecPoint, const UInt256 *k) {
	BIGNUM *privkey = BN_bin2bn((const unsigned char *) k, sizeof(*k), NULL);
	EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (NULL != privkey && NULL != key) {
		const EC_GROUP *curve = EC_KEY_get0_group(key);
		EC_POINT *_pubkey = EC_POINT_new(curve);
		if (_pubkey) {
			if (1 == EC_POINT_mul(curve, _pubkey, privkey, NULL, NULL, NULL)) {
				BIGNUM *__pubkey = EC_POINT_point2bn(curve, _pubkey, POINT_CONVERSION_COMPRESSED, NULL,
													 NULL);
				if (NULL != __pubkey) {
					uint8_t arrBN[256] = {0};
					int len = BN_bn2bin(__pubkey, arrBN);
					if (0 < len) {
						memcpy(brecPoint, arrBN, (size_t) len);
					}
					BN_free(__pubkey);
				}
			}
			EC_POINT_free(_pubkey);
		}
		BN_free(privkey);
		EC_KEY_free(key);
	}
}

size_t
ECDSA65Sign_sha256(const void *privKey, size_t privKeyLen, const UInt256 *md, void *signedData, size_t signedDataSize) {
	size_t out = 0;
	uint8_t *psignedData = signedData;
	if (0 == privKeyLen) {
		return out;
	}
	EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (key) {
		BIGNUM *privkeyIn = BN_bin2bn((const unsigned char *) privKey, (int) privKeyLen,
									  NULL);
		if (privkeyIn) {
			if (1 == EC_KEY_set_private_key(key, privkeyIn)) {
				ECDSA_SIG *sig = ECDSA_do_sign((unsigned char *) md, sizeof(*md), key);
				if (NULL != sig) {
					const BIGNUM *r = NULL;
					const BIGNUM *s = NULL;
					ECDSA_SIG_get0(sig, &r, &s);
					int nBitsR = BN_num_bits(r);
					int nBitsS = BN_num_bits(s);
					if (nBitsR <= 256 && nBitsS <= 256) {
						psignedData[0] = 64;
						uint8_t arrBIN[256] = {0};
						size_t szLen = 0;
						szLen = BN_bn2bin(r, arrBIN);
						memcpy(psignedData + 1 + (32 - szLen), arrBIN, szLen);
						memset(arrBIN, 0, sizeof(arrBIN));
						szLen = BN_bn2bin(s, arrBIN);
						memcpy(psignedData + 1 + 32 + (32 - szLen), arrBIN, szLen);
						out = szLen; //fixme
					}
					ECDSA_SIG_free(sig);
				}
			}
			BN_free(privkeyIn);
		}
		EC_KEY_free(key);
	}
	return out;
}

int ECDSA65Verify_sha256(const void *pubKey, size_t pubKeyLen, const UInt256 *md, const void *signedData,
						 size_t signedDataLen) {
	int out = 0;
	if (0 == pubKeyLen || 65 != signedDataLen) {
		return out;
	}
	//todo
//	if (PublickeyIsValid(pubKey, nid)) {
	BIGNUM *_pubkey = NULL;
	_pubkey = BN_bin2bn((const unsigned char *) (uint8_t *) pubKey, (int) pubKeyLen, NULL);
	EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (NULL != _pubkey && NULL != key) {
		const EC_GROUP *curve = EC_KEY_get0_group(key);
		EC_POINT *ec_p = EC_POINT_bn2point(curve, _pubkey, NULL, NULL);
		if (NULL != ec_p) {
			if (1 == EC_KEY_set_public_key(key, ec_p)) {
				const uint8_t *p64 = &signedData[1];
				ECDSA_SIG *sig = ECDSA_SIG_new();
				if (NULL != sig) {
					BIGNUM *r = BN_bin2bn(&p64[0], 32, NULL);
					BIGNUM *s = BN_bin2bn(&p64[32], 32, NULL);
					ECDSA_SIG_set0(sig, r, s);
					if (1 == ECDSA_do_verify((uint8_t *) md, sizeof(*md), sig, key)) {
						out = 1;
					}
					ECDSA_SIG_free(sig);
				}
			}
			EC_POINT_free(ec_p);
		}
		EC_KEY_free(key);
		BN_free(_pubkey);
	} else {
		if (NULL != _pubkey) {
			BN_free(_pubkey);
		}
		if (NULL != key) {
			EC_KEY_free(key);
		}
	}
//	}
	return out;
}

// BIP32 is a scheme for deriving chains of addresses from a seed value
// https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki

// Private parent key -> private child key
//
// CKDpriv((kpar, cpar), i) -> (ki, ci) computes a child extended private key from the parent extended private key:
//
// - Check whether i >= 2^31 (whether the child is a hardened key).
//     - If so (hardened child): let I = HMAC-SHA512(Key = cpar, Data = 0x00 || ser256(kpar) || ser32(i)).
//       (Note: The 0x00 pads the private key to make it 33 bytes long.)
//     - If not (normal child): let I = HMAC-SHA512(Key = cpar, Data = serP(point(kpar)) || ser32(i)).
// - Split I into two 32-byte sequences, IL and IR.
// - The returned child key ki is parse256(IL) + kpar (mod n).
// - The returned chain code ci is IR.
// - In case parse256(IL) >= n or ki = 0, the resulting key is invalid, and one should proceed with the next value for i
//   (Note: this has probability lower than 1 in 2^127.)
//
static void _CKDpriv(UInt256 *k, UInt256 *c, uint32_t i) {
	uint8_t buf[sizeof(BRECPoint) + sizeof(i)];
	UInt512 I;

	if (i & BIP32_HARD) {
		buf[0] = 0;
		UInt256Set(&buf[1], *k);
	} else getPubKeyFromPrivKey((BRECPoint *) buf, k);

	UInt32SetBE(&buf[sizeof(BRECPoint)], i);

	BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf, sizeof(buf)); // I = HMAC-SHA512(c, k|P(k) || i)

//    BRSecp256k1ModAdd(k, (UInt256 *)&I); // k = IL + k (mod n)
	_TweakSecret(k, k, (UInt256 *) &I, NID_X9_62_prime256v1);
	*c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR

	var_clean(&I);
	mem_clean(buf, sizeof(buf));
}

static int
_ECPointAdd(const EC_GROUP *group, EC_POINT *point, const unsigned char vchTweak[32], void *out, size_t outLen) {
	BN_CTX *ctx = BN_CTX_new();
	BN_CTX_start(ctx);
	BIGNUM *bnTweak = BN_CTX_get(ctx);
	BIGNUM *bnOrder = BN_CTX_get(ctx);
	BIGNUM *bnOne = BN_CTX_get(ctx);
	EC_GROUP_get_order(group, bnOrder,
					   ctx); // what a grossly inefficient way to get the (constant) group order...
	BN_bin2bn(vchTweak, 32, bnTweak);
	if (BN_cmp(bnTweak, bnOrder) >= 0)
		return -1; // extremely unlikely
	BN_one(bnOne);
	EC_POINT_mul(group, point, bnTweak, point, bnOne, ctx);
	if (EC_POINT_is_at_infinity(group, point))
		return -1; // ridiculously unlikely

	BIGNUM *pubkey = EC_POINT_point2bn(group, point, POINT_CONVERSION_COMPRESSED, NULL, NULL);
	if (NULL != pubkey) {
		uint8_t arrBN[256] = {0};
		int len = BN_bn2bin(pubkey, arrBN);
		if (0 < len) {
			len = (size_t) len < outLen ? len : outLen;
			memcpy(out, arrBN, (size_t) len);
		}
		BN_free(pubkey);
	}

	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	return 0;
}

static void _TweakPublic(BRECPoint *K, const unsigned char vchTweak[32], int nid) {
	EC_KEY *key = EC_KEY_new_by_curve_name(nid);
	if (key != NULL) {
		BIGNUM *pubKey = BN_bin2bn((const unsigned char *) (uint8_t *) K->p, sizeof(K->p),
								   NULL);
		if (NULL != pubKey) {
			const EC_GROUP *curve = EC_KEY_get0_group(key);
			EC_POINT *ec_p = EC_POINT_bn2point(curve, pubKey, NULL, NULL);
			if (NULL != ec_p) {
				if (1 == EC_KEY_set_public_key(key, ec_p)) {
					if (1 == EC_KEY_check_key(key)) {
						uint8_t mbDeriveKey[33];

						if (0 == _ECPointAdd(curve, ec_p, vchTweak, mbDeriveKey, sizeof(mbDeriveKey))) {
							memcpy(K->p, mbDeriveKey, 33);
						}
					}
				}
				EC_POINT_free(ec_p);
			}
			BN_free(pubKey);
		}
		EC_KEY_free(key);
	}
}

// Public parent key -> public child key
//
// CKDpub((Kpar, cpar), i) -> (Ki, ci) computes a child extended public key from the parent extended public key.
// It is only defined for non-hardened child keys.
//
// - Check whether i >= 2^31 (whether the child is a hardened key).
//     - If so (hardened child): return failure
//     - If not (normal child): let I = HMAC-SHA512(Key = cpar, Data = serP(Kpar) || ser32(i)).
// - Split I into two 32-byte sequences, IL and IR.
// - The returned child key Ki is point(parse256(IL)) + Kpar.
// - The returned chain code ci is IR.
// - In case parse256(IL) >= n or Ki is the point at infinity, the resulting key is invalid, and one should proceed with
//   the next value for i.
//
static void _CKDpub(BRECPoint *K, UInt256 *c, uint32_t i) {
	uint8_t buf[sizeof(*K) + sizeof(i)];
	UInt512 I;

	if ((i & BIP32_HARD) != BIP32_HARD) { // can't derive private child key from public parent key
		*(BRECPoint *) buf = *K;
		UInt32SetBE(&buf[sizeof(*K)], i);

		BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf, sizeof(buf)); // I = HMAC-SHA512(c, P(K) || i)

		*c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR
//        BRSecp256k1PointAdd(K, (UInt256 *)&I); // K = P(IL) + K
		_TweakPublic(K, (UInt256 *) &I, NID_X9_62_prime256v1);

		var_clean(&I);
		mem_clean(buf, sizeof(buf));
	}
}

// returns the master public key for the default BIP32 wallet layout - derivation path N(m/0H)
BRMasterPubKey BRBIP32MasterPubKey(const void *seed, size_t seedLen) {
	BRMasterPubKey mpk = BR_MASTER_PUBKEY_NONE;
	UInt512 I;
	UInt256 secret, chain;
	BRKey key;

	assert(seed != NULL || seedLen == 0);

	if (seed || seedLen == 0) {
		BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
		secret = *(UInt256 *) &I;
		chain = *(UInt256 *) &I.u8[sizeof(UInt256)];
		var_clean(&I);

		BRKeySetSecret(&key, &secret, 1);
		mpk.fingerPrint = BRKeyHash160(&key).u32[0];

		_CKDpriv(&secret, &chain, 0 | BIP32_HARD); // path m/0H

		mpk.chainCode = chain;
		BRKeySetSecret(&key, &secret, 1);
		var_clean(&secret, &chain);
		BRKeyPubKey(&key, &mpk.pubKey, sizeof(mpk.pubKey)); // path N(m/0H)
		BRKeyClean(&key);
	}

	return mpk;
}

// writes the public key for path N(m/0H/chain/index) to pubKey
// returns number of bytes written, or pubKeyLen needed if pubKey is NULL
size_t BRBIP32PubKey(uint8_t *pubKey, size_t pubKeyLen, BRMasterPubKey mpk, uint32_t chain, uint32_t index) {
	UInt256 chainCode = mpk.chainCode;

	assert(memcmp(&mpk, &BR_MASTER_PUBKEY_NONE, sizeof(mpk)) != 0);

	if (pubKey && sizeof(BRECPoint) <= pubKeyLen) {
		*(BRECPoint *) pubKey = *(BRECPoint *) mpk.pubKey;

		_CKDpub((BRECPoint *) pubKey, &chainCode, chain); // path N(m/0H/chain)
		_CKDpub((BRECPoint *) pubKey, &chainCode, index); // index'th key in chain
		var_clean(&chainCode);
	}

	return (!pubKey || sizeof(BRECPoint) <= pubKeyLen) ? sizeof(BRECPoint) : 0;
}

// sets the private key for path m/0H/chain/index to key
void BRBIP32PrivKey(BRKey *key, const void *seed, size_t seedLen, uint32_t chain, uint32_t index) {
	BRBIP32PrivKeyPath(key, seed, seedLen, 3, 0 | BIP32_HARD, chain, index);
}

// sets the private key for path m/0H/chain/index to each element in keys
void BRBIP32PrivKeyList(BRKey keys[], size_t keysCount, const void *seed, size_t seedLen, uint32_t chain,
						const uint32_t indexes[]) {
	UInt512 I;
	UInt256 secret, chainCode, s, c;

	assert(keys != NULL || keysCount == 0);
	assert(seed != NULL || seedLen == 0);
	assert(indexes != NULL || keysCount == 0);

	if (keys && keysCount > 0 && (seed || seedLen == 0) && indexes) {
		BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
		secret = *(UInt256 *) &I;
		chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];
		var_clean(&I);

		_CKDpriv(&secret, &chainCode, 0 | BIP32_HARD); // path m/0H
		_CKDpriv(&secret, &chainCode, chain); // path m/0H/chain

		for (size_t i = 0; i < keysCount; i++) {
			s = secret;
			c = chainCode;
			_CKDpriv(&s, &c, indexes[i]); // index'th key in chain
			BRKeySetSecret(&keys[i], &s, 1);
		}

		var_clean(&secret, &chainCode, &c, &s);
	}
}

void BRBIP44PrivKeyList(BRKey keys[], size_t keysCount, const void *seed, size_t seedLen, uint32_t coinIndex,
						uint32_t chain, const uint32_t indexes[]) {
	UInt256 chainCode;
	for (size_t i = 0; i < keysCount; i++) {
		BRBIP32PrivKeyPath(&keys[i], &chainCode, seed, seedLen, 5, 44 | BIP32_HARD, coinIndex | BIP32_HARD, 0 | BIP32_HARD,
						   chain, indexes[i]);
	}
	var_clean(&chainCode);
}

size_t BRBIP32PubKeyPath(uint8_t *pubKey, size_t pubKeyLen, BRMasterPubKey mpk, int depth, ...) {
	size_t len;
	va_list ap;

	va_start(ap, depth);
	len = BRBIP32vPubKeyPath(pubKey, pubKeyLen, mpk, depth, ap);
	va_end(ap);

	return len;
}

size_t BRBIP32vPubKeyPath(uint8_t *pubKey, size_t pubKeyLen, BRMasterPubKey mpk, int depth, va_list vlist) {
	UInt256 chainCode = mpk.chainCode;

	assert(memcmp(&mpk, &BR_MASTER_PUBKEY_NONE, sizeof(mpk)) != 0);

	if (pubKey && sizeof(BRECPoint) <= pubKeyLen) {
		*(BRECPoint *) pubKey = *(BRECPoint *) mpk.pubKey;

		for (int i = 0; i < depth; i++) {
			_CKDpub((BRECPoint *) pubKey, &chainCode, va_arg(vlist, uint32_t));
		}
		var_clean(&chainCode);
	}

	return (!pubKey || sizeof(BRECPoint) <= pubKeyLen) ? sizeof(BRECPoint) : 0;
}

// sets the private key for the specified path to key
// depth is the number of arguments used to specify the path
void BRBIP32PrivKeyPath(BRKey *key, UInt256 *chainCode, const void *seed, size_t seedLen, int depth, ...) {
	va_list ap;

	va_start(ap, depth);
	BRBIP32vPrivKeyPath(key, chainCode, seed, seedLen, depth, ap);
	va_end(ap);
}

// sets the private key for the path specified by vlist to key
// depth is the number of arguments in vlist
void BRBIP32vPrivKeyPath(BRKey *key, UInt256 *chainCode, const void *seed, size_t seedLen, int depth, va_list vlist) {
	UInt512 I;
	UInt256 secret;

	assert(key != NULL);
	assert(seed != NULL || seedLen == 0);
	assert(depth >= 0);

	if (key && (seed || seedLen == 0)) {
		BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
		secret = *(UInt256 *) &I;
		*chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];
		var_clean(&I);

		for (int i = 0; i < depth; i++) {
			_CKDpriv(&secret, chainCode, va_arg(vlist, uint32_t));
		}

		BRKeySetSecret(key, &secret, 1);
		var_clean(&secret);
	}
}

// writes the base58check encoded serialized master private key (xprv) to str
// returns number of bytes written including NULL terminator, or strLen needed if str is NULL
size_t BRBIP32SerializeMasterPrivKey(char *str, size_t strLen, const void *seed, size_t seedLen) {
	// TODO: XXX implement
	return 0;
}

// writes a master private key to seed given a base58check encoded serialized master private key (xprv)
// returns number of bytes written, or seedLen needed if seed is NULL
size_t BRBIP32ParseMasterPrivKey(void *seed, size_t seedLen, const char *str) {
	// TODO: XXX implement
	return 0;
}

// writes the base58check encoded serialized master public key (xpub) to str
// returns number of bytes written including NULL terminator, or strLen needed if str is NULL
size_t BRBIP32SerializeMasterPubKey(char *str, size_t strLen, BRMasterPubKey mpk) {
	// TODO: XXX implement
	return 0;
}

// returns a master public key give a base58check encoded serialized master public key (xpub)
BRMasterPubKey BRBIP32ParseMasterPubKey(const char *str) {
	// TODO: XXX implement
	return BR_MASTER_PUBKEY_NONE;
}

// key used for authenticated API calls, i.e. bitauth: https://github.com/bitpay/bitauth - path m/1H/0
void BRBIP32APIAuthKey(BRKey *key, const void *seed, size_t seedLen) {
	UInt256 chainCode;
	BRBIP32PrivKeyPath(key, &chainCode, seed, seedLen, 2, 1 | BIP32_HARD, 0);
}

// key used for BitID: https://github.com/bitid/bitid/blob/master/BIP_draft.md
void BRBIP32BitIDKey(BRKey *key, const void *seed, size_t seedLen, uint32_t index, const char *uri) {
	assert(key != NULL);
	assert(seed != NULL || seedLen == 0);
	assert(uri != NULL);

	if (key && (seed || seedLen == 0) && uri) {
		UInt256 hash;
		size_t uriLen = strlen(uri);
		uint8_t data[sizeof(index) + uriLen];

		UInt32SetLE(data, index);
		memcpy(&data[sizeof(index)], uri, uriLen);
		BRSHA256(&hash, data, sizeof(data));
		BRBIP32PrivKeyPath(key, seed, seedLen, 5, 13 | BIP32_HARD, UInt32GetLE(&hash.u32[0]) | BIP32_HARD,
						   UInt32GetLE(&hash.u32[1]) | BIP32_HARD, UInt32GetLE(&hash.u32[2]) | BIP32_HARD,
						   UInt32GetLE(&hash.u32[3]) | BIP32_HARD); // path m/13H/aH/bH/cH/dH
	}
}

