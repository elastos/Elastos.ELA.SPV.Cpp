// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IDCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include <Interface/IIdChainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class IdChainSubWallet : public SidechainSubWallet, public IIdChainSubWallet {
		public:
			virtual ~IdChainSubWallet();

			virtual nlohmann::json CreateIdTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJson,
					const nlohmann::json &programJson,
					const std::string &memo,
					const std::string &remark);

		protected:
			friend class MasterWallet;

			IdChainSubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const PluginType &pluginTypes,
							 MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINSUBWALLET_H__
