// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IMERKLEBLOCK_H__
#define __ELASTOS_SDK_IMERKLEBLOCK_H__

#include "ELAMessageSerializable.h"

#include <Core/BRInt.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class IMerkleBlock : public ELAMessageSerializable {
		public:
			virtual ~IMerkleBlock() {}

			virtual uint32_t GetHeight() const = 0;

			virtual void SetHeight(uint32_t height) = 0;

			virtual uint32_t GetTimestamp() const = 0;

			virtual void SetTimestamp(uint32_t timestamp) = 0;

			virtual uint32_t GetTarget() const = 0;

			virtual void SetTarget(uint32_t target) = 0;

			virtual const UInt256 &GetPrevBlockHash() const = 0;

			virtual void SetPrevBlockHash(const UInt256 &hash) = 0;

			virtual const UInt256 &GetRootBlockHash() const = 0;

			virtual void SetRootBlockHash(const UInt256 &hash) = 0;

			virtual uint32_t GetNonce() const = 0;

			virtual void SetNonce(uint32_t nonce) = 0;

			virtual uint32_t GetTransactionCount() const = 0;

			virtual void SetTransactionCount(uint32_t count) = 0;

			virtual const UInt256 &GetHash() const = 0;

			virtual void SetHash(const UInt256 &hash) = 0;

			virtual bool IsValid(uint32_t currentTime) const = 0;

			virtual bool IsEqual(const IMerkleBlock *block) const = 0;

			virtual std::string GetBlockType() const = 0;

			virtual size_t MerkleBlockTxHashes(std::vector<UInt256> &txHashes) const = 0;
		};

		typedef boost::shared_ptr<IMerkleBlock> MerkleBlockPtr;

	}
}

#endif //__ELASTOS_SDK_IMERKLEBLOCK_H__
