// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ISidechainSubWallet.h"

#ifndef __ELASTOS_SDK_INEOCHAINSUBWALLET_H__
#define __ELASTOS_SDK_INEOCHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {
		class INeochainSubWallet: public virtual ISidechainSubWallet {
		public:

			/**
			 * Get sum of balances of all addresses.
			 * @param nep5Hash nep5 smartcontract hash.
			 * @return sum of balances in big int string.
			 */
			virtual std::string GetBalance(const std::string &nep5Hash) const = 0;

			/**
			 * Get balance of only the specified address.
			 * @param assetID asset hex code from asset hash.
			 * @param address is one of addresses created by current sub wallet.
			 * @return balance of specified address in big int string.
			 */
			virtual std::string GetBalanceWithAddress(const std::string &nep5Hash,
			                                          const std::string &addrProgramHash) const = 0;

			/**
			 * Get all Nep5 token hash
			 * @return
			 */
			virtual std::vector<std::string> GetAllNep5Token() const = 0;

			/**
			 * Get balances of all addresses in json format.
			 * @return balances of all addresses in json format.
			 */
			virtual nlohmann::json GetBalanceInfo() const = 0;

		};
	}
}

#endif //__ELASTOS_SDK_INEOCHAINSUBWALLET_H__
