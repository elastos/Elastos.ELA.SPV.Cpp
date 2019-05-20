// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "SDK/WalletCore/Bloom9/Bloom9.h"

using namespace Elastos::ElaWallet;

TEST_CASE("BloomTest", "[Bloom]") {

	SECTION("addAddress and match test") {
		uint168 addr1("21ee9737eda27983b2c72259dc4d81635fb9da09c8");
		Bloom9 bloom1;
		bloom1.AddAddress(addr1);

		Bloom9 bloom2;
		bloom2.AddAddress(addr1);

		bool result = bloom1.Match(bloom2);
		REQUIRE(result == true);

		uint168 addr2("22ee9737eda27983b2c72259dc4d81635fb9da09c8");
		Bloom9 bloom3;
		bloom3.AddAddress(addr2);
		result = bloom1.Match(bloom3);
		REQUIRE(result == false);

		uint160 topic1("095e7baea6a6c7c4c2dfeb977efac326af552d87");
		Bloom9 bloom4;
		bloom4.AddTopic(topic1);
		result = bloom1.Match(bloom4);
		REQUIRE(result == false);

		bloom1.AddTopic(topic1);
		result = bloom1.Match(bloom4);
		REQUIRE(result == true);

	}

	SECTION("SetBloomData test") {
		std::vector<uint8_t> filter = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                              4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0,
		                              0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8,
		                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0,
		                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		Bloom9 filter1;
		filter1.SetBloomData(filter);
		Bloom9 filter2;

		std::vector<uint8_t> topic(20);
		topic = {219, 33, 93, 226, 117, 139, 125, 116, 63, 102, 228, 198, 108, 252, 195, 93, 197, 76, 203, 203};
		uint160 u160(topic);
		filter2.AddTopic(u160);

		bool result = filter1.Match(filter2);
		REQUIRE(result == true);

	}
}
