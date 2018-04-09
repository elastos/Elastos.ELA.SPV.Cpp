//
//BRInt.c
// Created by elastos zxb on 2018/4/9.
//
#include "BRInt.h"

void UInt16SetBE(void *b2, uint16_t u)
{
    *(union _u16 *)b2 = (union _u16) { (u >> 8) & 0xff, u & 0xff };
}

void UInt16SetLE(void *b2, uint16_t u)
{
    *(union _u16 *)b2 = (union _u16) { u & 0xff, (u >> 8) & 0xff };
}

void UInt32SetBE(void *b4, uint32_t u)
{
    *(union _u32 *)b4 = (union _u32) { (u >> 24) & 0xff, (u >> 16) & 0xff, (u >> 8) & 0xff, u & 0xff };
}

void UInt32SetLE(void *b4, uint32_t u)
{
    *(union _u32 *)(b4) = (union _u32) { (u) & 0xff, ((u) >> 8) & 0xff, ((u) >> 16) & 0xff, ((u) >> 24) & 0xff };
}

void UInt64SetBE(void *b8, uint64_t u)
{
    *(union _u64 *)b8 =(union _u64) {
	(u >> 56) & 0xff, (u >> 48) & 0xff, (u >> 40) & 0xff, (u >> 32) & 0xff,
	(u >> 24) & 0xff, (u >> 16) & 0xff, (u >> 8) & 0xff, u & 0xff };
}

void UInt64SetLE(void *b8, uint64_t u)
{
    *(union _u64 *)b8 = (union _u64) { u & 0xff, (u >> 8) & 0xff, (u >> 16) & 0xff, (u >> 24) & 0xff,
                                       (u >> 32) & 0xff, (u >> 40) & 0xff, (u >> 48) & 0xff, (u >> 56) & 0xff
                                     };
}

void UInt128Set(void *b16, UInt128 u)
{
    *(union _u128 *)(b16) =
        (union _u128) { (u).u8[0], (u).u8[1], (u).u8[2],  (u).u8[3],  (u).u8[4],  (u).u8[5],  (u).u8[6],  (u).u8[7],
                        (u).u8[8], (u).u8[9], (u).u8[10], (u).u8[11], (u).u8[12], (u).u8[13], (u).u8[14], (u).u8[15]
                      };
}

void UInt160Set(void *b20, UInt160 u)
{
    *(union _u160 *)(b20) =
        (union _u160) { (u).u8[0],  (u).u8[1],  (u).u8[2],  (u).u8[3],  (u).u8[4],  (u).u8[5],  (u).u8[6],  (u).u8[7],
                        (u).u8[8],  (u).u8[9],  (u).u8[10], (u).u8[11], (u).u8[12], (u).u8[13], (u).u8[14], (u).u8[15],
                        (u).u8[16], (u).u8[17], (u).u8[18], (u).u8[19]
                      };
}

void UInt168Set(void *b21, UInt168 u)
{
    *(union _u168 *)(b21) =
        (union _u168) { (u).u8[0],  (u).u8[1],  (u).u8[2],  (u).u8[3],  (u).u8[4],  (u).u8[5],  (u).u8[6],  (u).u8[7],
                        (u).u8[8],  (u).u8[9],  (u).u8[10], (u).u8[11], (u).u8[12], (u).u8[13], (u).u8[14], (u).u8[15],
                        (u).u8[16], (u).u8[17], (u).u8[18], (u).u8[19], (u).u8[20]
                      };
}

void UInt256Set(void *b32, UInt256 u)
{
    *(union _u256 *)(b32) =
        (union _u256) { (u).u8[0],  (u).u8[1],  (u).u8[2],  (u).u8[3],  (u).u8[4],  (u).u8[5],  (u).u8[6],  (u).u8[7],
                        (u).u8[8],  (u).u8[9],  (u).u8[10], (u).u8[11], (u).u8[12], (u).u8[13], (u).u8[14], (u).u8[15],
                        (u).u8[16], (u).u8[17], (u).u8[18], (u).u8[19], (u).u8[20], (u).u8[21], (u).u8[22], (u).u8[23],
                        (u).u8[24], (u).u8[25], (u).u8[26], (u).u8[27], (u).u8[28], (u).u8[29], (u).u8[30], (u).u8[31]
                      };
}

uint16_t UInt16GetBE(const void *b2)
{
   return (((uint16_t)((const uint8_t *)b2)[0] << 8) | ((uint16_t)((const uint8_t *)b2)[1]));
}

uint16_t UInt16GetLE(const void *b2)
{
   return (((uint16_t)((const uint8_t *)b2)[1] << 8) | ((uint16_t)((const uint8_t *)b2)[0]));
}

uint32_t UInt32GetBE(const void *b4)
{
   return (((uint32_t)((const uint8_t *)b4)[0] << 24) | ((uint32_t)((const uint8_t *)b4)[1] << 16) |
            ((uint32_t)((const uint8_t *)b4)[2] << 8)  | ((uint32_t)((const uint8_t *)b4)[3]));

}

uint32_t UInt32GetLE(const void *b4)
{
    return (((uint32_t)((const uint8_t *)b4)[3] << 24) | ((uint32_t)((const uint8_t *)b4)[2] << 16) |
            ((uint32_t)((const uint8_t *)b4)[1] << 8)  | ((uint32_t)((const uint8_t *)b4)[0]));
}

uint64_t UInt64GetBE(const void *b8)
{
    return (((uint64_t)((const uint8_t *)b8)[0] << 56) | ((uint64_t)((const uint8_t *)b8)[1] << 48) |
            ((uint64_t)((const uint8_t *)b8)[2] << 40) | ((uint64_t)((const uint8_t *)b8)[3] << 32) |
            ((uint64_t)((const uint8_t *)b8)[4] << 24) | ((uint64_t)((const uint8_t *)b8)[5] << 16) |
            ((uint64_t)((const uint8_t *)b8)[6] << 8)  | ((uint64_t)((const uint8_t *)b8)[7]));
}

uint64_t UInt64GetLE(const void *b8)
{
    return (((uint64_t)((const uint8_t *)b8)[7] << 56) | ((uint64_t)((const uint8_t *)b8)[6] << 48) |
            ((uint64_t)((const uint8_t *)b8)[5] << 40) | ((uint64_t)((const uint8_t *)b8)[4] << 32) |
            ((uint64_t)((const uint8_t *)b8)[3] << 24) | ((uint64_t)((const uint8_t *)b8)[2] << 16) |
            ((uint64_t)((const uint8_t *)b8)[1] << 8)  | ((uint64_t)((const uint8_t *)b8)[0]));
}

UInt128 UInt128Get(const void *b16)
{
    return (UInt128) { .u8 = {
        ((const uint8_t *)b16)[0],  ((const uint8_t *)b16)[1],  ((const uint8_t *)b16)[2],  ((const uint8_t *)b16)[3],
        ((const uint8_t *)b16)[4],  ((const uint8_t *)b16)[5],  ((const uint8_t *)b16)[6],  ((const uint8_t *)b16)[7],
        ((const uint8_t *)b16)[8],  ((const uint8_t *)b16)[9],  ((const uint8_t *)b16)[10], ((const uint8_t *)b16)[11],
        ((const uint8_t *)b16)[12], ((const uint8_t *)b16)[13], ((const uint8_t *)b16)[14], ((const uint8_t *)b16)[15]
     }};
}

UInt160 UInt160Get(const void *b20)
{
   return (UInt160) { .u8 = {
        ((const uint8_t *)b20)[0],  ((const uint8_t *)b20)[1],  ((const uint8_t *)b20)[2],  ((const uint8_t *)b20)[3], \
        ((const uint8_t *)b20)[4],  ((const uint8_t *)b20)[5],  ((const uint8_t *)b20)[6],  ((const uint8_t *)b20)[7], \
        ((const uint8_t *)b20)[8],  ((const uint8_t *)b20)[9],  ((const uint8_t *)b20)[10], ((const uint8_t *)b20)[11], \
        ((const uint8_t *)b20)[12], ((const uint8_t *)b20)[13], ((const uint8_t *)b20)[14], ((const uint8_t *)b20)[15], \
        ((const uint8_t *)b20)[16], ((const uint8_t *)b20)[17], ((const uint8_t *)b20)[18], ((const uint8_t *)b20)[19] \
    }};
}

UInt168 UInt168Get(const void *b21)
{
    return (UInt168) { .u8 = {
        ((const uint8_t *)b21)[0],  ((const uint8_t *)b21)[1],  ((const uint8_t *)b21)[2],  ((const uint8_t *)b21)[3],
        ((const uint8_t *)b21)[4],  ((const uint8_t *)b21)[5],  ((const uint8_t *)b21)[6],  ((const uint8_t *)b21)[7],
        ((const uint8_t *)b21)[8],  ((const uint8_t *)b21)[9],  ((const uint8_t *)b21)[10], ((const uint8_t *)b21)[11],
        ((const uint8_t *)b21)[12], ((const uint8_t *)b21)[13], ((const uint8_t *)b21)[14], ((const uint8_t *)b21)[15],
        ((const uint8_t *)b21)[16], ((const uint8_t *)b21)[17], ((const uint8_t *)b21)[18], ((const uint8_t *)b21)[19],
        ((const uint8_t *)b21)[20]
    } };
}

UInt256 UInt256Get(const void *b32)
{
    return (UInt256) { .u8 = {
        ((const uint8_t *)b32)[0],  ((const uint8_t *)b32)[1],  ((const uint8_t *)b32)[2],  ((const uint8_t *)b32)[3],
        ((const uint8_t *)b32)[4],  ((const uint8_t *)b32)[5],  ((const uint8_t *)b32)[6],  ((const uint8_t *)b32)[7],
        ((const uint8_t *)b32)[8],  ((const uint8_t *)b32)[9],  ((const uint8_t *)b32)[10], ((const uint8_t *)b32)[11],
        ((const uint8_t *)b32)[12], ((const uint8_t *)b32)[13], ((const uint8_t *)b32)[14], ((const uint8_t *)b32)[15],
        ((const uint8_t *)b32)[16], ((const uint8_t *)b32)[17], ((const uint8_t *)b32)[18], ((const uint8_t *)b32)[19],
        ((const uint8_t *)b32)[20], ((const uint8_t *)b32)[21], ((const uint8_t *)b32)[22], ((const uint8_t *)b32)[23],
        ((const uint8_t *)b32)[24], ((const uint8_t *)b32)[25], ((const uint8_t *)b32)[26], ((const uint8_t *)b32)[27],
        ((const uint8_t *)b32)[28], ((const uint8_t *)b32)[29], ((const uint8_t *)b32)[30], ((const uint8_t *)b32)[31]
    } };
}