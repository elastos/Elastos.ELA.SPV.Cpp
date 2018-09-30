// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONSET_H__
#define __ELASTOS_SDK_TRANSACTIONSET_H__

#include <map>

#include "Transaction.h"
#include "Plugin/Interface/IMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		template <class T>
		class ElementSet {
		public:

			const T &GetTransaction(const UInt256 &hash) const {
				//fixme [refactor]
			}

			bool Contains(const T &tx) const {
				//fixme [refactor]
			}

			bool Contains(const UInt256 &hash) const {
				//fixme [refactor]
			}

			void Insert(const T &tx) {
				//fixme [refactor]
			}

			void Remove(const T &tx) {
				//fixme [refactor]
			}

		private:

		};

		typedef ElementSet<TransactionPtr> TransactionSet;
		typedef ElementSet<MerkleBlockPtr> BlockSet;
	}
}

#endif //__ELASTOS_SDK_TRANSACTIONSET_H__
