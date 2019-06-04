// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NeoMerkleBlock.h"

#include <SDK/Plugin/Registry.h>
#include <SDK/Common/hash.h>

namespace Elastos {
	namespace ElaWallet {
		NeoMerkleBlock::NeoMerkleBlock() {

		}

		NeoMerkleBlock::~NeoMerkleBlock() {

		}

		void NeoMerkleBlock::Serialize(ByteStream &ostream) const {
			SerializeNoAux(ostream);
			idAuxPow.Serialize(ostream);
			ostream.WriteUint8(1);
			MerkleBlockBase::SerializeAfterAux(ostream);
		}

		bool NeoMerkleBlock::Deserialize(const ByteStream &istream) {
			if (!DeserializeNoAux(istream) || !idAuxPow.Deserialize(istream))
				return false;

			istream.Skip(1);
			if (!MerkleBlockBase::DeserializeAfterAux(istream))
				return false;

			GetHash();

			return true;
		}

		void NeoMerkleBlock::SerializeNoAux(ByteStream &ostream) const {

			MerkleBlockBase::SerializeNoAux(ostream);

			ostream.WriteBytes(_receiptHash);

			ostream.WriteVarBytes(_bloom);
		}

		bool NeoMerkleBlock::DeserializeNoAux(const ByteStream &istream) {

			if (!MerkleBlockBase::DeserializeNoAux(istream)) {
				return false;
			}

			if (!istream.ReadBytes(_receiptHash))
				return false;

			if (!istream.ReadVarBytes(_bloom))
				return false;

			return true;
		}

		const uint256 &NeoMerkleBlock::GetHash() const {
			if (_blockHash == 0) {
				ByteStream ostream;
				SerializeNoAux(ostream);
				_blockHash = sha256_2(ostream.GetBytes());
			}
			return _blockHash;
		}

		std::string NeoMerkleBlock::GetBlockType() const {
			return "NeoSide";
		}

		const bytes_t &NeoMerkleBlock::GetBloom() const {
			return _bloom;
		}

		MerkleBlockPtr NeoMerkleBlockFactory::createBlock() {
			return MerkleBlockPtr(new NeoMerkleBlock);
		}

		fruit::Component<INeoMerkleBlockFactory> getNeoBlockFactoryComponent() {
			return fruit::createComponent()
					.bind<INeoMerkleBlockFactory, NeoMerkleBlockFactory>();
		}
	}
}