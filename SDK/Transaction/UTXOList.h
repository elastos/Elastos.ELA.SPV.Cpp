// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTXOSET_H__
#define __ELASTOS_SDK_UTXOSET_H__

#include <vector>

#include "BRInt.h"
#include "Transaction/TransactionInput.h"

namespace Elastos {
	namespace ElaWallet {

		struct UTXO {
			UTXO() : hash(UINT256_ZERO), n(0) {}

			bool operator<(const UTXO &otherUtxo) {
				if (UInt256Eq(&hash, &otherUtxo.hash))
					return UInt256LessThan(&hash, &otherUtxo.hash) == 1;
				else
					return n < otherUtxo.n;
			}

			UInt256 hash;
			uint32_t n;
		};

		class UTXOList {
		public:

			bool Constains(const UInt256 &hash) const;

			UTXO &operator[](size_t i);

			size_t size() const;

			void Clear();

			void AddByTxInput(const TransactionInput &input);

			void AddUTXO(const UInt256 &hash, uint32_t index);

			void RemoveAt(size_t index);
		};

	}
}

#endif //__ELASTOS_SDK_UTXOSET_H__
