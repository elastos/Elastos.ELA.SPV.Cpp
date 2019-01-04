// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"
#include <SDK/Plugin/Transaction/Payload/PayloadRecord.h>

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadRecord test", "PayloadRecord") {

	SECTION("Serialize and deserialize") {
		PayloadRecord p1(getRandString(20), getRandCMBlock(50)), p2;

		ByteStream stream;

		p1.Serialize(stream, 0);

		stream.setPosition(0);
		REQUIRE(p2.Deserialize(stream, 0));

		REQUIRE(p1.getRecordType() == p2.getRecordType());
		REQUIRE((p1.getRecordData() == p2.getRecordData()));
	}

	SECTION("to json and from json") {
		PayloadRecord p1(getRandString(20), getRandCMBlock(50)), p2;

		nlohmann::json p1Json = p1.toJson();

		p2.fromJson(p1Json);

		REQUIRE(p1.getRecordType() == p2.getRecordType());
		REQUIRE(p1.getRecordData() == p2.getRecordData());
	}
}
