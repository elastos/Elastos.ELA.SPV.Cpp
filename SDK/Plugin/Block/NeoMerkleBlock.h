// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_NEOMERKLEBLOCK_H__
#define __ELASTOS_SDK_NEOMERKLEBLOCK_H__

#include "SidechainMerkleBlock.h"

#include <SDK/Plugin/Block/IDAuxPow.h>
#include <SDK/Plugin/Interface/INeoMerkleBlock.h>

#include <fruit/fruit.h>

namespace Elastos {
	namespace ElaWallet {
		class NeoMerkleBlock : public SidechainMerkleBlock {
		public:
			NeoMerkleBlock();
			~NeoMerkleBlock();
		public:
			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			virtual const uint256 &GetHash() const;

			virtual std::string GetBlockType() const;

			virtual const bytes_t &GetBloom() const;

		protected:
			void SerializeNoAux(ByteStream &ostream) const;

			bool DeserializeNoAux(const ByteStream &istream);

		protected:
			uint256 _receiptHash;
			bytes_t _bloom;
		};

		class INeoMerkleBlockFactory {
		public:
			virtual MerkleBlockPtr createBlock() = 0;
		};

		class NeoMerkleBlockFactory : public INeoMerkleBlockFactory {
		public:
			INJECT(NeoMerkleBlockFactory()) = default;

			virtual MerkleBlockPtr createBlock();
		};

		fruit::Component<INeoMerkleBlockFactory> getNeoBlockFactoryComponent();
	}
}

#endif //__ELASTOS_SDK_NEOMERKLEBLOCK_H__
