// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRPeerMessages.h>
#include <BRBloomFilter.h>
#include "BRBloomFilter.h"

#include "BloomFilter.h"
#include "BloomFilterMessage.h"
#include "ByteStream.h"

namespace Elastos {
	namespace ElaWallet {

		namespace {

			BRBloomFilter *BRBloomFilterCopy(BRBloomFilter *filter) {
				BRBloomFilter *result = (BRBloomFilter *) calloc(1, sizeof(*filter));
				result->length = filter->length;
				result->tweak = filter->tweak;
				result->hashFuncs = filter->hashFuncs;
				result->flags = filter->flags;
				result->elemCount = filter->elemCount;
				result->filter = (uint8_t *) malloc(filter->length);
				memcpy(result->filter, filter->filter, filter->length);
				return result;
			}
		}

		bool BloomFilterMessage::Accept(const std::string &msg) {
			return false;
		}

		void BloomFilterMessage::Send(const SendMessageParameter &param) {
			const BloomFilterParameter &bloomFilterParameter = static_cast<const BloomFilterParameter &>(param);
			//fixme [refactor] replace BRBloomFileter with BloomFilter
//			BloomFilter wrappedFilter = BloomFilter(BRBloomFilterCopy());
//			ByteStream byteStream;
//			wrappedFilter.Serialize(byteStream);
//			((BRPeerContext *) peer)->sentFilter = 1;
//			((BRPeerContext *) peer)->sentMempool = 0;
//			CMBlock buf = byteStream.getBuffer();
//			BRPeerSendMessage(peer, buf, buf.GetSize(), MSG_FILTERLOAD);
		}

		std::string BloomFilterMessage::Type() const {
			return MSG_FILTERLOAD;
		}
	}
}