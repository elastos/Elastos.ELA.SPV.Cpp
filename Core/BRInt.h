//
//  BRInt.h
//
//  Created by Aaron Voisine on 8/16/15.
//  Copyright (c) 2015 breadwallet LLC.
//  Copyright (c) 2018 elastos zxb.
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

#ifndef BRInt_h
#define BRInt_h

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// large integers
typedef union _u16 {
    uint8_t u8[16/8];
} UInt16;

typedef union _u32 {
    uint8_t u8[32/8];
} UInt32;

typedef union _u64 {
    uint8_t u8[64/8];
} UInt64;

typedef union _u128 {
    uint8_t u8[128/8];
    uint16_t u16[128/16];
    uint32_t u32[128/32];
    uint64_t u64[128/64];
} UInt128;

typedef union _u160 {
    uint8_t u8[160/8];
    uint16_t u16[160/16];
    uint32_t u32[160/32];
} UInt160;

typedef union _u168 {
    uint8_t u8[168/8];
}UInt168;

typedef union _u256 {
    uint8_t u8[256/8];
    uint16_t u16[256/16];
    uint32_t u32[256/32];
    uint64_t u64[256/64];
} UInt256;

typedef union _u512 {
    uint8_t u8[512/8];
    uint16_t u16[512/16];
    uint32_t u32[512/32];
    uint64_t u64[512/64];
} UInt512;

inline static int UInt128Eq(UInt128 a, UInt128 b)
{
    return (a.u64[0] == b.u64[0] && a.u64[1] == b.u64[1]);
}

inline static int UInt160Eq(UInt160 a, UInt160 b)
{
    return (a.u32[0] == b.u32[0] && a.u32[1] == b.u32[1] && a.u32[2] == b.u32[2] && a.u32[3] == b.u32[3] &&
            a.u32[4] == b.u32[4]);
}

inline static int UInt168Eq(UInt168 a, UInt168 b)
{
    for(short i = 0; i < 21; i++)
    {
        if(a.u8[i] != b.u8[i])
        {
            return 0;
        }
    }
    return 1;
}

inline static int UInt256Eq(UInt256 a, UInt256 b)
{
    return (a.u64[0] == b.u64[0] && a.u64[1] == b.u64[1] && a.u64[2] == b.u64[2] && a.u64[3] == b.u64[3]);
}

inline static int UInt512Eq(UInt512 a, UInt512 b)
{
    return (a.u64[0] == b.u64[0] && a.u64[1] == b.u64[1] && a.u64[2] == b.u64[2] && a.u64[3] == b.u64[3] &&
            a.u64[4] == b.u64[4] && a.u64[5] == b.u64[5] && a.u64[6] == b.u64[6] && a.u64[7] == b.u64[7]);
}

inline static int UInt128IsZero(UInt128 u)
{
    return ((u.u64[0] | u.u64[1]) == 0);
}

inline static int UInt160IsZero(UInt160 u)
{
    return ((u.u32[0] | u.u32[1] | u.u32[2] | u.u32[3] | u.u32[4]) == 0);
}

inline static int UInt168IsZero(UInt168 u)
{
    int value = 0;
    for(short i = 0; i < 21; i++)
    {
        value = value | u.u8[i];
        if(value != 0)
        {
            break;
        }
    }
    return value == 0;
}

inline static int UInt256IsZero(UInt256 u)
{
    return ((u.u64[0] | u.u64[1] | u.u64[2] | u.u64[3]) == 0);
}

inline static int UInt512IsZero(UInt512 u)
{
    return ((u.u64[0] | u.u64[1] | u.u64[2] | u.u64[3] | u.u64[4] | u.u64[5] | u.u64[6] | u.u64[7]) == 0);
}

inline static UInt256 UInt256Reverse(UInt256 u)
{
    return ((UInt256) { .u8 = { u.u8[31], u.u8[30], u.u8[29], u.u8[28], u.u8[27], u.u8[26], u.u8[25], u.u8[24],
                                u.u8[23], u.u8[22], u.u8[21], u.u8[20], u.u8[19], u.u8[18], u.u8[17], u.u8[16],
                                u.u8[15], u.u8[14], u.u8[13], u.u8[12], u.u8[11], u.u8[10], u.u8[ 9], u.u8[ 8],
                                u.u8[ 7], u.u8[ 6], u.u8[5],  u.u8[ 4], u.u8[ 3], u.u8[ 2], u.u8[ 1], u.u8[ 0] } });
}

#define UINT128_ZERO ((UInt128) { .u64 = { 0, 0 } })
#define UINT160_ZERO ((UInt160) { .u32 = { 0, 0, 0, 0, 0 } })
#define UINT168_ZERO ((UInt168) { .u8 = { 0, 0, 0, 0, 0 ,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} })
#define UINT256_ZERO ((UInt256) { .u64 = { 0, 0, 0, 0 } })
#define UINT512_ZERO ((UInt512) { .u64 = { 0, 0, 0, 0, 0, 0, 0, 0 } })

// hex encoding/decoding

#define u256hex(u) ((const char[]) {\
    _hexc((u).u8[ 0] >> 4), _hexc((u).u8[ 0]), _hexc((u).u8[ 1] >> 4), _hexc((u).u8[ 1]),\
    _hexc((u).u8[ 2] >> 4), _hexc((u).u8[ 2]), _hexc((u).u8[ 3] >> 4), _hexc((u).u8[ 3]),\
    _hexc((u).u8[ 4] >> 4), _hexc((u).u8[ 4]), _hexc((u).u8[ 5] >> 4), _hexc((u).u8[ 5]),\
    _hexc((u).u8[ 6] >> 4), _hexc((u).u8[ 6]), _hexc((u).u8[ 7] >> 4), _hexc((u).u8[ 7]),\
    _hexc((u).u8[ 8] >> 4), _hexc((u).u8[ 8]), _hexc((u).u8[ 9] >> 4), _hexc((u).u8[ 9]),\
    _hexc((u).u8[10] >> 4), _hexc((u).u8[10]), _hexc((u).u8[11] >> 4), _hexc((u).u8[11]),\
    _hexc((u).u8[12] >> 4), _hexc((u).u8[12]), _hexc((u).u8[13] >> 4), _hexc((u).u8[13]),\
    _hexc((u).u8[14] >> 4), _hexc((u).u8[14]), _hexc((u).u8[15] >> 4), _hexc((u).u8[15]),\
    _hexc((u).u8[16] >> 4), _hexc((u).u8[16]), _hexc((u).u8[17] >> 4), _hexc((u).u8[17]),\
    _hexc((u).u8[18] >> 4), _hexc((u).u8[18]), _hexc((u).u8[19] >> 4), _hexc((u).u8[19]),\
    _hexc((u).u8[20] >> 4), _hexc((u).u8[20]), _hexc((u).u8[21] >> 4), _hexc((u).u8[21]),\
    _hexc((u).u8[22] >> 4), _hexc((u).u8[22]), _hexc((u).u8[23] >> 4), _hexc((u).u8[23]),\
    _hexc((u).u8[24] >> 4), _hexc((u).u8[24]), _hexc((u).u8[25] >> 4), _hexc((u).u8[25]),\
    _hexc((u).u8[26] >> 4), _hexc((u).u8[26]), _hexc((u).u8[27] >> 4), _hexc((u).u8[27]),\
    _hexc((u).u8[28] >> 4), _hexc((u).u8[28]), _hexc((u).u8[29] >> 4), _hexc((u).u8[29]),\
    _hexc((u).u8[30] >> 4), _hexc((u).u8[30]), _hexc((u).u8[31] >> 4), _hexc((u).u8[31]), '\0' })

#define uint256(s) { .u8 = {\
    (_hexu((s)[ 0]) << 4) | _hexu((s)[ 1]), (_hexu((s)[ 2]) << 4) | _hexu((s)[ 3]),\
    (_hexu((s)[ 4]) << 4) | _hexu((s)[ 5]), (_hexu((s)[ 6]) << 4) | _hexu((s)[ 7]),\
    (_hexu((s)[ 8]) << 4) | _hexu((s)[ 9]), (_hexu((s)[10]) << 4) | _hexu((s)[11]),\
    (_hexu((s)[12]) << 4) | _hexu((s)[13]), (_hexu((s)[14]) << 4) | _hexu((s)[15]),\
    (_hexu((s)[16]) << 4) | _hexu((s)[17]), (_hexu((s)[18]) << 4) | _hexu((s)[19]),\
    (_hexu((s)[20]) << 4) | _hexu((s)[21]), (_hexu((s)[22]) << 4) | _hexu((s)[23]),\
    (_hexu((s)[24]) << 4) | _hexu((s)[25]), (_hexu((s)[26]) << 4) | _hexu((s)[27]),\
    (_hexu((s)[28]) << 4) | _hexu((s)[29]), (_hexu((s)[30]) << 4) | _hexu((s)[31]),\
    (_hexu((s)[32]) << 4) | _hexu((s)[33]), (_hexu((s)[34]) << 4) | _hexu((s)[35]),\
    (_hexu((s)[36]) << 4) | _hexu((s)[37]), (_hexu((s)[38]) << 4) | _hexu((s)[39]),\
    (_hexu((s)[40]) << 4) | _hexu((s)[41]), (_hexu((s)[42]) << 4) | _hexu((s)[43]),\
    (_hexu((s)[44]) << 4) | _hexu((s)[45]), (_hexu((s)[46]) << 4) | _hexu((s)[47]),\
    (_hexu((s)[48]) << 4) | _hexu((s)[49]), (_hexu((s)[50]) << 4) | _hexu((s)[51]),\
    (_hexu((s)[52]) << 4) | _hexu((s)[53]), (_hexu((s)[54]) << 4) | _hexu((s)[55]),\
    (_hexu((s)[56]) << 4) | _hexu((s)[57]), (_hexu((s)[58]) << 4) | _hexu((s)[59]),\
    (_hexu((s)[60]) << 4) | _hexu((s)[61]), (_hexu((s)[62]) << 4) | _hexu((s)[63]) } }

#define _hexc(u) (((u) & 0x0f) + ((((u) & 0x0f) <= 9) ? '0' : 'a' - 0x0a))
#define _hexu(c) (((c) >= '0' && (c) <= '9') ? (c) - '0' : ((c) >= 'a' && (c) <= 'f') ? (c) - ('a' - 0x0a) :\
                  ((c) >= 'A' && (c) <= 'F') ? (c) - ('A' - 0x0a) : -1)

// unaligned memory access helpers

static void UInt16SetBE(void *b2, uint16_t u);

static void UInt16SetLE(void *b2, uint16_t u);

static void UInt32SetBE(void *b4, uint32_t u);

static void UInt32SetLE(void *b4, uint32_t u);

static void UInt64SetBE(void *b8, uint64_t u);

static void UInt64SetLE(void *b8, uint64_t u);

static void UInt128Set(void *b16, UInt128 u);

static void UInt160Set(void *b20, UInt160 u);

static void UInt168Set(void *b21, UInt168 u);

static void UInt256Set(void *b32, UInt256 u);

static uint16_t UInt16GetBE(const void *b2);

static uint16_t UInt16GetLE(const void *b2);

static uint32_t UInt32GetBE(const void *b4);

static uint32_t UInt32GetLE(const void *b4);

static uint64_t UInt64GetBE(const void *b8);

static uint64_t UInt64GetLE(const void *b8);

static UInt128 UInt128Get(const void *b16);

static UInt160 UInt160Get(const void *b20);

static UInt168 UInt168Get(const void *b21);

static UInt256 UInt256Get(const void *b32);

#ifdef __cplusplus
}
#endif

#endif // BRInt_h
