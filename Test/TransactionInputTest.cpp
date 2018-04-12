// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "TransactionInput.h"
#include "Address.h"

using namespace Elastos::SDK;

TEST_CASE( "TransactionInput test", "[TransactionInput]" )
{
	SECTION("Constructor with null and object") {
		TransactionInput transactionInput(nullptr);
		REQUIRE(transactionInput.getRaw() == nullptr);

		BRTxInput* txInput = new BRTxInput();
		TransactionInput transactionInput1(txInput);
		REQUIRE(transactionInput1.getRaw() == txInput);
	}

	SECTION( "Contructor width input attributes", "[TransactionInput]" ) {
		UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		uint32_t index = 8000;
		uint64_t amount = 10000;
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);
		ByteData script = myaddress.getPubKeyScript();
		ByteData signature;
		uint32_t sequence = 1;

		TransactionInput transactionInput(hash, index, amount, script, signature, sequence);
		REQUIRE(transactionInput.getRaw() != nullptr);
		UInt256 tempHash = transactionInput.getHash();
		int result = UInt256Eq(&tempHash, &hash);
		REQUIRE(result > 0);
		REQUIRE(transactionInput.getIndex() == index);
		REQUIRE(transactionInput.getAmount() == amount);
	}
}
