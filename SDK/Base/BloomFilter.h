// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BLOOMFILTER_H__
#define __ELASTOS_SDK_BLOOMFILTER_H__

#include <SDK/Common/CMemBlock.h>
#include <SDK/Plugin/Interface/ELAMessageSerializable.h>

#include <boost/shared_ptr.hpp>

#define BLOOM_DEFAULT_FALSEPOSITIVE_RATE 0.0005 // use 0.00005 for less data, 0.001 for good anonymity
#define BLOOM_REDUCED_FALSEPOSITIVE_RATE 0.00005
#define BLOOM_UPDATE_NONE                0
#define BLOOM_UPDATE_ALL                 1
#define BLOOM_UPDATE_P2PUBKEY_ONLY       2
#define BLOOM_MAX_FILTER_LENGTH          36000 // this allows for 10,000 elements with a <0.0001% false positive rate

namespace Elastos {
	namespace ElaWallet {

		class BloomFilter :
				ELAMessageSerializable {
		public:
			BloomFilter(double falsePositiveRate, size_t elemCount, uint32_t tweak, uint8_t flags);

			~BloomFilter();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			void insertData(const void *data, size_t dataLen);

			bool ContainsData(const void *data, size_t dataLen);

			uint32_t calculateHash(const void *data, size_t dataLen, uint32_t hashNum);

		private:
			CMBlock _filter;
			uint32_t _hashFuncs;
			size_t _elemCount;
			uint32_t _tweak;
			uint8_t _flags;
		};

		typedef boost::shared_ptr<BloomFilter> BloomFilterPtr;

	}
}

#endif //__ELASTOS_SDK_BLOOMFILTER_H__
