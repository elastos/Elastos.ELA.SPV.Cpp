// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BigIntFormat.h"

using namespace Elastos::SDK;

TEST_CASE("tesing Normal", "[BigIntFormatTest.cpp]") {
	uint8_t block[] = {0xBF, 0xCE, 0x24, 0x45, 0x67, 0xA6};
	CMemBlock<uint8_t> mbsrc;
	mbsrc.SetMemFixed(block, sizeof(block));

	CMemBlock<char> mbdest = Hex2Str(mbsrc);

	std::string strdest = "BFCE244567A6";
	REQUIRE(strdest == (const char *) mbdest);

	uint8_t block1[] = {165, 226, 33};
	CMemBlock<uint8_t>mbsrc1;
	mbsrc1.SetMemFixed(block1, sizeof(block1));
	CMemBlock<char> mbdest1 = Dec2Str(mbsrc1);

	std::string strdest1 = (const char *) mbdest1;
	REQUIRE(strdest1 == (const char *) mbdest1);

	CMemBlock<uint8_t> mbsrc1recov = Str2Dec(mbdest1);
	REQUIRE(0 == memcmp(mbsrc1recov, block1, sizeof(block1)));
}

TEST_CASE("tesing None Normal", "[BigIntFormatTest.cpp]") {
	CMemBlock<uint8_t> mbsrc;

	CMemBlock<char> mbdest = Hex2Str(mbsrc);

	REQUIRE((const void *) mbdest == (const void *) mbsrc);

	CMemBlock<uint8_t> mbsrcrecov = Str2Hex(mbdest);
	REQUIRE((const void *) mbsrcrecov == (const void *) mbsrc);
}