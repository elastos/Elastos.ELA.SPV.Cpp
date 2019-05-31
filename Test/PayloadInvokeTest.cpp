// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <SDK/Plugin/Transaction/Payload/PayloadInvoke.h>
#include <SDK/Common/Log.h>

using namespace Elastos::ElaWallet;


TEST_CASE("PayloadInvoke test", "[PayloadInvoke]") {
	Log::registerMultiLogger();

	SECTION("Serialize and deserialize") {
		PayloadInvoke p1;
		p1.SetCodeHash(getRandUInt168());
		p1.SetCode(getRandBytes(50));
		p1.SetProgramHash(getRandUInt168());
		p1.SetGas(343);

		ByteStream stream;
		p1.Serialize(stream, 0);

		PayloadInvoke p2;
		REQUIRE(p2.Deserialize(stream, 0));
		REQUIRE(p1.GetProgramHash() == p2.GetProgramHash());
		REQUIRE(p1.GetCodeHash() == p2.GetCodeHash());
		REQUIRE(p1.GetCode() == p2.GetCode());
		REQUIRE(p1.GetGas() == p2.GetGas());
	}

	SECTION("to json and from json") {
		PayloadInvoke p1;
		p1.SetCodeHash(getRandUInt168());
		p1.SetCode(getRandBytes(50));
		p1.SetProgramHash(getRandUInt168());
		p1.SetGas(343);

		nlohmann::json p1Json = p1.ToJson(0);

		PayloadInvoke p2;
		p2.FromJson(p1Json, 0);

		REQUIRE(p1.GetProgramHash() == p2.GetProgramHash());
		REQUIRE(p1.GetCodeHash() == p2.GetCodeHash());
		REQUIRE(p1.GetCode() == p2.GetCode());
		REQUIRE(p1.GetGas() == p2.GetGas());
	}
}