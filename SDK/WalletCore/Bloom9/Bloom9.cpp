// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Bloom9.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/hash.h>

namespace Elastos {
	namespace ElaWallet {
		Bloom9::Bloom9() {
			_bloom.resize(ETHEREUM_BLOOM_FILTER_BYTES, 0);
		}

		Bloom9::Bloom9(const bytes_t &filter) :
				_bloom(filter) {
		}

		void Bloom9::AddAddress(const uint168 &address) {
			const bytes_t data = address.bytes();
			FilterCreateHash(uint256(sha256_2(data)));
		}

		void Bloom9::AddTopic(const uint160 &address) {
			bytes_t data = address.bytes();
			FilterCreateHash(uint256(sha256_2(data)));
		}

		uint Bloom9::CreateFilterIndex(uint8_t highByte, uint8_t lowByte) {
			return ((highByte << 8) | lowByte) & ETHERUM_BLOOM_FILTER_BITS_MASK;
		}

		void Bloom9::FilterCreateHash(const uint256 &hash) {
			const bytes_t bytes = hash.bytes();
			SetFilterBit(CreateFilterIndex(bytes[0], bytes[1]));
			SetFilterBit(CreateFilterIndex(bytes[2], bytes[3]));
			SetFilterBit(CreateFilterIndex(bytes[4], bytes[5]));
		}

		void Bloom9::SetFilterBit(uint index) {
			uint byteIndex;
			uint bitIndex;
			FilterExtractLocation(index, &byteIndex, &bitIndex);
			_bloom[byteIndex] |= (1 << bitIndex);
		}

		void Bloom9::FilterExtractLocation(uint index, uint *byteIndex, uint *bitIndex) {
			assert (index < ETHEREUM_BLOOM_FILTER_BITS);
			assert (NULL != byteIndex && NULL != bitIndex);

			*byteIndex = (ETHEREUM_BLOOM_FILTER_BITS - 1 - index) / 8;
			*bitIndex = index % 8;
		}

		const Bloom9 Bloom9::BloomFilterOr(const Bloom9 &otherFilter) {
			const bytes_t bloom = otherFilter.GetBloomData();
			bytes_t result(ETHEREUM_BLOOM_FILTER_BYTES);
			for (int i = 0; i < ETHEREUM_BLOOM_FILTER_BYTES; ++i) {
				result[i] = _bloom[i] | bloom[i];
			}
			Bloom9 bloom9;
			bloom9.SetBloomData(result);
			return bloom9;
		}

		void Bloom9::BloomFilterOrInPlace(const Bloom9 &otherFilter) {
			for (int i = 0; i < ETHEREUM_BLOOM_FILTER_BYTES; ++i) {
				_bloom[i] |= otherFilter._bloom[i];
			}
		}

		bool Bloom9::FilterIsEqual(const Bloom9 &otherFilter) const {
			const bytes_t bloom = otherFilter.GetBloomData();
			return 0 == strcmp(_bloom.getHex().c_str(), bloom.getHex().c_str());
		}

		bool Bloom9::Match(const Bloom9 &otherFilter) {
			const Bloom9 filter = BloomFilterOr(otherFilter);
			return FilterIsEqual(filter);
		}

		const bytes_t &Bloom9::GetBloomData() const {
			return _bloom;
		}

		void Bloom9::SetBloomData(const bytes_t &filter) {
			_bloom = filter;
		}

	}
}