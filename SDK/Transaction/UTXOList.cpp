// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UTXOList.h"
#include "TransactionInput.h"

namespace Elastos {
	namespace ElaWallet {

		bool UTXOList::Constains(const UInt256 &hash) const {
			//fixme [refactor]
			return false;
		}

		UTXO &UTXOList::operator[](size_t i) {
			//fixme [refactor]
			static UTXO temp;
			return temp;
		}

		size_t UTXOList::size() const {
			//fixme [refactor]
			return 0;
		}

		void UTXOList::Clear() {
			//fixme [refactor]
		}

		void UTXOList::AddByTxInput(const TransactionInput &input) {
			//fixme [refactor]
		}

		void UTXOList::AddUTXO(const UInt256 &hash, uint32_t index) {
			//fixme [refactor]
		}

		void UTXOList::RemoveAt(size_t index) {
			//fixme [refactor]
		}
	}
}

