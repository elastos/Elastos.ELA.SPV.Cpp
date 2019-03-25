// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <SDK/BIPs/Address.h>
#include <SDK/BIPs/Mnemonic.h>
#include <SDK/BIPs/HDKeychain.h>
#include <SDK/BIPs/BIP39.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Address test", "[Address]") {
	SECTION("Address with public key derived from mnemonic") {

		std::string phrase = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
		std::string phrasePasswd = "";

		uint512 seed = BIP39::DeriveSeed(phrase, phrasePasswd);

		HDKeychain child = HDKeychain(HDSeed(seed.bytes()).getExtendedKey(true)).getChild("44'/0'/0'/0/0");

		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == Address(PrefixStandard, child.pubkey()).String());
	}
}