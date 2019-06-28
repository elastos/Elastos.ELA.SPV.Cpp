// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_NEOCHAINSUBWALLET_H__
#define __ELASTOS_SDK_NEOCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"

#include <Interface/INeochainSubWallet.h>
#include <SDK/Plugin/Nep5/Nep5Log.h>
#include "../Wallet/GroupedNep5Asset.h"

namespace Elastos {
	namespace ElaWallet {
		class NeochainSubWallet : public SidechainSubWallet, public INeochainSubWallet {
		public:
			virtual ~NeochainSubWallet();

		protected:
			friend class MasterWallet;

			NeochainSubWallet(const CoinInfoPtr &info, const ChainConfigPtr &config, MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::vector<uint256> &hash, uint32_t blockHeight, time_t timeStamp);

			virtual void onSaveNep5Log(const Nep5LogPtr &nep5LogPtr);

			virtual std::string GetBalance(const std::string &nep5Hash) const;

			virtual std::string GetBalanceWithAddress(const std::string &nep5Hash,
			                                          const std::string &addrProgramHash) const;

			virtual std::vector<std::string> GetAllNep5Token() const;

			virtual nlohmann::json GetBalanceInfo() const;

		private:
			void InitNep5Assets(const std::vector<Nep5LogPtr> &nep5Assets);

			bool ContainNep5Asset(const std::string &nep5Hash) const;

			bool ContainsAddrHash(const std::string &addrHash);
		private:
			typedef std::map<std::string, GroupedNep5Ptr> Nep5AssetMap;
			mutable Nep5AssetMap _nep5AssetMap;
		};
	}
}


#endif //__ELASTOS_SDK_NEOCHAINSUBWALLET_H__
