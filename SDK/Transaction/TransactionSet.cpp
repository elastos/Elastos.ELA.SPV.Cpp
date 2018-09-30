// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionSet.h"

namespace Elastos {
	namespace ElaWallet {

		const TransactionPtr &TransactionSet::GetTransaction(const UInt256 &hash) const {
			//fixme [refactor]
			return nullptr;
		}

		bool TransactionSet::Contains(const TransactionPtr &tx) const {
			//fixme [refactor]
			return false;
		}

		bool TransactionSet::Contains(const UInt256 &hash) const {
			//fixme [refactor]
			return false;
		}

		void TransactionSet::Insert(const TransactionPtr &tx) {
//fixme [refactor]
		}

		void TransactionSet::Remove(const TransactionPtr &tx) {
//fixme [refactor]
		}
	}
}
