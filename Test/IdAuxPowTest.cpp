// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <SDK/Plugin/Block/IdAuxPow.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <Core/BRMerkleBlock.h>

using namespace Elastos::ElaWallet;

TEST_CASE("IdAuxPow test", "[IdAuxPow]") {
	srand((unsigned int)time(nullptr));

	SECTION("Serialize and deserialize") {
		IdAuxPow ap1;

		std::vector<UInt256> hashes;
		hashes.resize(20);
		for (size_t i = 0; i < hashes.size(); ++i) {
			hashes[i] = getRandUInt256();
		}

		ap1.setIdAuxMerkleBranch(hashes);
		ap1.setIdAuxMerkleIndex(getRandUInt32());

		Transaction tx;
		initTransaction(tx, Transaction::TxVersion::V09);

		ap1.setIdAuxBlockTx(tx);

		ELAMerkleBlock *block = ELAMerkleBlockNew();
		block->raw.blockHash = getRandUInt256();
		block->raw.version = getRandUInt32();
		block->raw.prevBlock = getRandUInt256();
		block->raw.merkleRoot = getRandUInt256();
		block->raw.timestamp = getRandUInt32();
		block->raw.target = getRandUInt32();
		block->raw.nonce = getRandUInt32();
		block->raw.totalTx = getRandUInt32();
		block->raw.hashesCount = 10;
		hashes.resize(block->raw.hashesCount);
		for (size_t i = 0; i < block->raw.hashesCount; ++i) {
			hashes[i] = getRandUInt256();
		}
		CMBlock flags = getRandCMBlock(5);
		BRMerkleBlockSetTxHashes(&block->raw, hashes.data(), hashes.size(), flags, flags.GetSize());
		ap1.setMainBlockHeader(block);

		ByteStream stream;
		ap1.Serialize(stream);


//		 verify
		IdAuxPow ap2;
		stream.setPosition(0);
		REQUIRE(ap2.Deserialize(stream));

		const std::vector<UInt256> &hashes1 = ap1.getIdAuxMerkleBranch();
		const std::vector<UInt256> &hashes2 = ap2.getIdAuxMerkleBranch();
		REQUIRE(hashes1.size() == hashes2.size());
		for (size_t i = 0; i < hashes1.size(); ++i) {
			REQUIRE(UInt256Eq(&hashes1[i], &hashes2[i]));
		}

		ELAMerkleBlock *b1 = ap1.getMainBlockHeader();
		ELAMerkleBlock *b2 = ap2.getMainBlockHeader();

		REQUIRE(b1->auxPow.getParBlockHeader()->nonce == b2->auxPow.getParBlockHeader()->nonce);
		REQUIRE(b1->auxPow.getParBlockHeader()->target == b2->auxPow.getParBlockHeader()->target);

		const Transaction &tx1 = ap1.getIdAuxBlockTx();
		const Transaction &tx2 = ap2.getIdAuxBlockTx();

		verifyTransaction(tx1, tx2, false);
	}

	SECTION("to and from json") {

	}

}
