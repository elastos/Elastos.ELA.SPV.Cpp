// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_NEOCHAINSUBWALLET_H__
#define __ELASTOS_SDK_NEOCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include <Interface/INeochainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {
		class NeochainSubWallet : public SidechainSubWallet {
		public:
			virtual ~NeochainSubWallet();

		protected:
			friend class MasterWallet;

			NeochainSubWallet(const CoinInfo &info, const ChainParams &chainParams, const PluginType &pluginTypes,
			                  MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const uint256 &hash, uint32_t blockHeight, uint32_t timeStamp);
		};
	}
}


#endif //__ELASTOS_SDK_NEOCHAINSUBWALLET_H__
