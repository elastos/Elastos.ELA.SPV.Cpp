// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NeochainSubWallet.h"

namespace Elastos {
	namespace ElaWallet {
		NeochainSubWallet::NeochainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
		                                     const PluginType &pluginTypes,
		                                     MasterWallet *parent) :
				SidechainSubWallet(info, chainParams, pluginTypes, parent) {

		}

		NeochainSubWallet::~NeochainSubWallet() {

		}

		nlohmann::json NeochainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Neochain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

		void NeochainSubWallet::onTxAdded(const TransactionPtr &transaction) {
			SubWallet::onTxAdded(transaction);
		}

		void NeochainSubWallet::onTxUpdated(const uint256 &hash, uint32_t blockHeight, uint32_t timeStamp) {
			SubWallet::onTxUpdated(hash, blockHeight, timeStamp);
		}

	}
}