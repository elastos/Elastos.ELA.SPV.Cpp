/*
 * Copyright (c) 2020 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK_IETHSIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_IETHSIDECHAINSUBWALLET_H__

#include "ISubWallet.h"

namespace Elastos {
	namespace ElaWallet {

		enum EthereumAmountUnit {
			TOKEN_DECIMAL = 0,
			TOKEN_INTEGER = 1,

			ETHER_WEI = 0,
			ETHER_GWEI = 3,
			ETHER_ETHER = 6,
		};

		class IEthSidechainSubWallet : public virtual ISubWallet {
		public:
			virtual ~IEthSidechainSubWallet() noexcept {}

			/**
			 *
			 * @param targetAddress
			 * @param amount Decimal string in unit.  The `number` must be either an integer or have
			 * a single decimal point with at least one preceeding characters.  Thus: 0.001, 1.0000, 12
			 * and 12.100 are all valid.  But .1 is invalid (required 0.1).
			 * @param amountUnit
			 * @param nonce
			 * @return
			 */
			virtual nlohmann::json CreateTransfer(const std::string &targetAddress,
												  const std::string &amount,
												  EthereumAmountUnit amountUnit,
                                                  const std::string &gasPrice,
                                                  EthereumAmountUnit gasPriceUnit,
                                                  const std::string &gasLimit,
												  uint64_t nonce) const = 0;

			/**
			 *
			 * @param targetAddress "" is different from "0x0000000000000000000000000000000000000000". "" means
			 * address is null.
			 * @param amount Decimal string in unit.  The `number` must be either an integer or have
			 * a single decimal point with at least one preceeding characters.  Thus: 0.001, 1.0000, 12
			 * and 12.100 are all valid.  But .1 is invalid (required 0.1).
			 * @param amountUnit
			 * @param gasPrice Decimal string in unit.  The `number` must be either an integer or have
			 * a single decimal point with at least one preceeding characters.  Thus: 0.001, 1.0000, 12
			 * and 12.100 are all valid.  But .1 is invalid (required 0.1).
			 * @param gasPriceUnit
			 * @param gasLimit
			 * @param data
			 * @param nonce
			 * @return
			 */
			virtual nlohmann::json CreateTransferGeneric(const std::string &targetAddress,
														 const std::string &amount,
														 EthereumAmountUnit amountUnit,
														 const std::string &gasPrice,
														 EthereumAmountUnit gasPriceUnit,
														 const std::string &gasLimit,
														 const std::string &data,
														 uint64_t nonce) const = 0;

			virtual std::string ExportPrivateKey(const std::string &payPassword) const = 0;

        };

	}
}

#endif
