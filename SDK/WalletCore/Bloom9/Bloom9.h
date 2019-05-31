// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BLOOM9_H__
#define __ELASTOS_SDK_BLOOM9_H__

#include "SDK/Common/ByteStream.h"

namespace Elastos {
	namespace ElaWallet {

#define ETHEREUM_BLOOM_FILTER_BITS 2048
#define ETHEREUM_BLOOM_FILTER_BYTES   (ETHEREUM_BLOOM_FILTER_BITS / 8)
#define ETHERUM_BLOOM_FILTER_BITS_MASK  (ETHEREUM_BLOOM_FILTER_BITS - 1)

		class Bloom9 {
		public:

			Bloom9();

			Bloom9(const bytes_t &filter);

			void AddAddress(const uint168 &address);

			void AddTopic(const uint160 &address);

			void SetBloomData(const bytes_t &filter);

			const bytes_t &GetBloomData() const;

			bool Match(const Bloom9 &otherFilter);

		public:
			const Bloom9 BloomFilterOr(const Bloom9 &otherFilter);

			void BloomFilterOrInPlace(const Bloom9 &otherFilter);

			void FilterCreateHash(const uint256 &hash);

			uint CreateFilterIndex(uint8_t highBye, uint8_t lowByte);

			void SetFilterBit(uint index);

			void FilterExtractLocation(uint index, uint *byteIndex, uint *bitIndex);

			bool FilterIsEqual(const Bloom9 &otherFilter) const;

		private:
			bytes_t _bloom;
		};

	}
}

#endif //__ELASTOS_SDK_BLOOM9_H__
