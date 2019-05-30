// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_NEOCHAINSUBWALLET_H__
#define __ELASTOS_SDK_NEOCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"

#include <Interface/INeochainSubWallet.h>
#include <SDK/Plugin/Nep5/Nep5Log.h>
#include <SDK/Wallet/GroupNep5Asset.h>

namespace Elastos {
	namespace ElaWallet {
		class NeochainSubWallet : public SidechainSubWallet, public INeochainSubWallet {
		public:
			virtual ~NeochainSubWallet();

		protected:
			friend class MasterWallet;

			NeochainSubWallet(const CoinInfo &info, const ChainParams &chainParams, const PluginType &pluginTypes,
			                  MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const uint256 &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual nlohmann::json CreateDeployContractTx (
					const std::string &avmBytes,
					const std::string &name,
					const std::string &version,
					const std::string author,
					const std::string email,
					const std::string desc,
					const std::string gas,
					const std::string memo,
					const std::string &remark);

			virtual nlohmann::json CreateInvokeContractTx (
					const std::string &contractAddrHash,
					const std::string &params,
					const std::string fee,
					const std::string gas,
					const std::string memo,
					const std::string &remark);

			void InitNep5Assets(const std::vector<Nep5LogPtr> &nep5Assets);

		private:
			bool ContainNep5Asset(const std::string &nep5Hash) const;

		private:
			typedef std::map<std::string, GroupNep5Ptr> Nep5AssetMap;
			Nep5AssetMap _nep5AssetMap;
		};
	}
}


#endif //__ELASTOS_SDK_NEOCHAINSUBWALLET_H__
