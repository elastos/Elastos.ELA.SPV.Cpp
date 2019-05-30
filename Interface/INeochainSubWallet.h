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
			 * Deploy smart contract.
			 * @param avmBytes contract's code bytes
			 * @param name contract's name.
			 * @param version contract's version
			 * @param author contract's author name or owner's ela address.
			 * @param email owner's email address.
			 * @param desc description of the contract.
			 * @param gas fee of deploy transaction, default 500 ela
			 * @return Transaction in json format.
			 */
			virtual nlohmann::json CreateDeployContractTx (
					const std::string &avmBytes,
					const std::string &name,
					const std::string &version,
					const std::string author,
					const std::string email,
					const std::string desc,
					const std::string gas,
					const std::string memo,
					const std::string &remark) = 0;

			/**
			 * Invoke smart contract.
			 * @param contractAddrHash contract's code hash
			 * @param params contract parameters list to invoke. separate params with comma ','.
			 * @param fee transaction fee
			 * @param gas contract's execution of consumption.
			 * @return Transaction in json format.
			 */
			virtual nlohmann::json CreateInvokeContractTx (
					const std::string &contractAddrHash,
					const std::string &params,
					const std::string fee,
					const std::string gas,
					const std::string memo,
					const std::string &remark) = 0;
		};
	}
}

#endif //__ELASTOS_SDK_INEOCHAINSUBWALLET_H__
