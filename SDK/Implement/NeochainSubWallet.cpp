// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NeochainSubWallet.h"

#include <SDK/Common/ErrorChecker.h>

#include <map>

namespace Elastos {
	namespace ElaWallet {
		NeochainSubWallet::NeochainSubWallet(const CoinInfoPtr &info,
		                                     const ChainConfigPtr &config,
		                                     MasterWallet *parent) :
				SidechainSubWallet(info, config, parent) {
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

		void NeochainSubWallet::onSaveNep5Log(const Nep5LogPtr &nep5LogPtr) {
			std::string hash = nep5LogPtr->GetNep5Hash();
			GroupedNep5Ptr nep5Asset;
			if (ContainNep5Asset(hash)) {
				nep5Asset = _nep5AssetMap[hash];
			} else {
				nep5Asset = GroupedNep5Ptr(new GroupedNep5Asset());
				_nep5AssetMap[hash] = nep5Asset;
			}
			const std::string from = nep5LogPtr->GetFrom();
			if (ContainsAddrHash(from)) {
				nep5Asset->MinusBalance(from, nep5LogPtr->GetData());
			}

			const std::string to = nep5LogPtr->GetTo();
			if (ContainsAddrHash(to)) {
				nep5Asset->AddBalance(to, nep5LogPtr->GetData());
			}

		}

		void NeochainSubWallet::InitNep5Assets(const std::vector<Nep5LogPtr> &nep5Assets) {
			_nep5AssetMap.clear();

			for (size_t i = 0; i < nep5Assets.size(); ++i) {
				onSaveNep5Log(nep5Assets[i]);
			}
		}

		bool NeochainSubWallet::ContainNep5Asset(const std::string &nep5Hash) const {
			return _nep5AssetMap.find(nep5Hash) != _nep5AssetMap.end();
		}

		bool NeochainSubWallet::ContainsAddrHash(const std::string &addrHash) {
			std::vector<Address> addrs;
			_subAccount->GetAllAddresses(addrs, 0, size_t(-1), true);
			size_t len = addrs.size();
			for (size_t i = 0; i < len; ++i) {

				const bytes_t &uInt168 = addrs[i].ProgramHash().bytes();
				if (uInt168.getHex().compare(2, addrHash.size(), addrHash) == 0) {
					return true;
				}

			}
			return false;
		}

		std::vector<std::string> NeochainSubWallet::GetAllNep5Token() const {
			std::vector<std::string> tokens;
			for (std::map<std::string, GroupedNep5Ptr>::iterator iter = _nep5AssetMap.begin();
			     iter != _nep5AssetMap.end(); iter++) {
				tokens.push_back(iter->first);
			}
			return tokens;
		}

		std::string NeochainSubWallet::GetBalance(const std::string &nep5Hash) const {
			GroupedNep5Ptr groupNep5Ptr = _nep5AssetMap[nep5Hash];
			if (groupNep5Ptr) {
				return groupNep5Ptr->GetBalance().getDec();
			}

			return "0";
		}

		std::string NeochainSubWallet::GetBalanceWithAddress(const std::string &nep5Hash,
		                                                     const std::string &addrProgramHash) const {
			GroupedNep5Ptr groupNep5Ptr = _nep5AssetMap[nep5Hash];
			if (groupNep5Ptr) {
				return groupNep5Ptr->GetBalance(addrProgramHash).getDec();
			}
			return "0";
		}

		nlohmann::json NeochainSubWallet::GetBalanceInfo() const {
			nlohmann::json jsonData;
			for (std::map<std::string, GroupedNep5Ptr>::iterator iter = _nep5AssetMap.begin();
			     iter != _nep5AssetMap.end(); iter++) {
				nlohmann::json nep5Info;
				nep5Info["hash"] = iter->first;
				GroupedNep5Ptr groupNep5Ptr = _nep5AssetMap[iter->first];
				nep5Info["balance"] = groupNep5Ptr->GetBalance().getDec();

				jsonData.push_back(nep5Info);
			}
			return jsonData;
		}

	}
}