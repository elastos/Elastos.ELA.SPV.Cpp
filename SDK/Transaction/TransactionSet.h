// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONSET_H__
#define __ELASTOS_SDK_TRANSACTIONSET_H__

#include <map>

#include "Transaction.h"

namespace Elastos {
	namespace ElaWallet {

		class TransactionSet {
		public:

			const TransactionPtr &GetTransaction(const UInt256 &hash) const;

			bool Contains(const TransactionPtr &tx) const;

			bool Contains(const UInt256 &hash) const;

			void Insert(const TransactionPtr &tx);

			void Remove(const TransactionPtr &tx);

		private:

		};

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONSET_H__
