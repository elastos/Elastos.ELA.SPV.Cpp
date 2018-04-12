// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Wallet.h"

using namespace Elastos::SDK;

TEST_CASE( "Wallet listener test", "[Wallet]")
{
	class WalletListener : public Wallet::Listener {

		// func balanceChanged(_ balance: UInt64)
		virtual void balanceChanged(uint64_t balance) {
			printf("balanceChanged");
		}

		// func txAdded(_ tx: BRTxRef)
		virtual void onTxAdded(Transaction *transaction) {
			printf("onTxAdded");
		}

		// func txUpdated(_ txHashes: [UInt256], blockHeight: UInt32, timestamp: UInt32)
		virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			printf("onTxUpdated");
		}

		// func txDeleted(_ txHash: UInt256, notifyUser: Bool, recommendRescan: Bool)
		virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			printf("onTxDeleted");
		}
	};

	SECTION("constructor width params") {
		//todo completed me
	}


}
