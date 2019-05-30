// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NeochainSubWallet.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		NeochainSubWallet::NeochainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
		                                     const PluginType &pluginTypes,
		                                     MasterWallet *parent) :
				SidechainSubWallet(info, chainParams, pluginTypes, parent) {

			std::vector<Nep5LogPtr> nep5logs = _walletManager->loadNep5Logs();
			InitNep5Assets(nep5logs);

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

		void NeochainSubWallet::InitNep5Assets(const std::vector<Nep5LogPtr> &nep5Assets) {
			for (size_t i = 0; i < nep5Assets.size(); ++i) {
				std::string hash = nep5Assets[i]->GetNep5Hash();
				GroupNep5Ptr nep5Asset;
				if (!ContainNep5Asset(hash)) {
					nep5Asset = GroupNep5Ptr(new GroupNep5Asset());
					_nep5AssetMap[hash] = nep5Asset;
				} else {
					nep5Asset = _nep5AssetMap[hash];
				}
				nep5Asset->AddNep5Log(nep5Assets[i]);
			}
		}

		bool NeochainSubWallet::ContainNep5Asset(const std::string &nep5Hash) const {
			return _nep5AssetMap.find(nep5Hash) != _nep5AssetMap.end();
		}

		nlohmann::json NeochainSubWallet::CreateDeployContractTx(
				const std::string &avmBytes,
				const std::string &name,
				const std::string &version,
				const std::string author,
				const std::string email,
				const std::string desc,
				const std::string gas,
				const std::string memo,
				const std::string &remark) {
			ErrorChecker::CheckParam(avmBytes.size() > 0, Error::InvalidArgument, "avmBytes is empty");

			//todo complete this
			nlohmann::json jsonData;
			return jsonData;
		}

		nlohmann::json NeochainSubWallet::CreateInvokeContractTx(
				const std::string &contractAddrHash,
				const std::string &params,
				const std::string fee,
				const std::string gas,
				const std::string memo,
				const std::string &remark) {

				//todo complete this
			nlohmann::json jsonData;
			return jsonData;
		}
	}
}