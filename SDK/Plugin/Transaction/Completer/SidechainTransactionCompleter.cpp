// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SidechainTransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		SidechainTransactionCompleter::SidechainTransactionCompleter(const TransactionPtr &transaction,
																	 const WalletPtr &wallet) : TransactionCompleter(
				transaction, wallet) {

		}

		SidechainTransactionCompleter::~SidechainTransactionCompleter() {

		}
	}
}