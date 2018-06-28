// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>

#include "BRPeerManager.h"
#include "BRPeerMessages.h"
#include "BRArray.h"
#include "BRMerkleBlock.h"

#include "Peer.h"
#include "MerkleBlockMessage.h"
#include "Log.h"
#include "Utils.h"
#include "AuxPow.h"
#include "ELACoreExt/ELAPeerManager.h"
#include "ELACoreExt/ELAMerkleBlock.h"
#include "Plugin/Registry.h"
#include "Plugin/Block/MerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		int MerkleBlockMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			BRPeerContext *ctx = (BRPeerContext *) peer;
#ifdef MERKLE_BLOCK_PLUGIN
			// msg is holding by payload pointer create by malloc, do not match delete[] in ByteStream
			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);

			ELAPeerManager *elaPeerManager = (ELAPeerManager *)ctx->manager;
			MerkleBlockPtr wrappedBlock(Registry::Instance()->CreateMerkleBlock(elaPeerManager->Plugins.BlockType));
			assert(wrappedBlock != nullptr);
			wrappedBlock->Deserialize(stream);

			ELAMerkleBlock *elablock = ELAMerkleBlockCopy((ELAMerkleBlock *)wrappedBlock->getRawBlock());
			BRMerkleBlock *block = (BRMerkleBlock *)elablock;
			block->blockHash = wrappedBlock->getBlockHash();
			int r = 1;

			if (!wrappedBlock->isValid((uint32_t) time(nullptr))) {
#else
			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);
			MerkleBlock wrappedBlock;
			wrappedBlock.Deserialize(stream);

			ELAMerkleBlock *elablock = ELAMerkleBlockCopy((ELAMerkleBlock *)wrappedBlock.getRaw());
			BRMerkleBlock *block = (BRMerkleBlock *)elablock;
			block->blockHash = wrappedBlock.getBlockHash();
			int r = 1;

			if (!wrappedBlock.isValid((uint32_t) time(nullptr))) {
#endif
				peer_log(peer, "invalid merkleblock: %s", Utils::UInt256ToString(block->blockHash).c_str());
				ELAMerkleBlockFree(elablock);
				block = nullptr;
				r = 0;
			} else if (!ctx->sentFilter && !ctx->sentGetdata) {
				peer_log(peer, "got merkleblock message before loading a filter");
				ELAMerkleBlockFree(elablock);
				block = nullptr;
				r = 0;
			} else {
				size_t count = BRMerkleBlockTxHashes(block, NULL, 0);
				UInt256 _hashes[(sizeof(UInt256) * count <= 0x1000) ? count : 0],
						*hashes = (sizeof(UInt256) * count <= 0x1000) ? _hashes : (UInt256 *) malloc(
						count * sizeof(*hashes));
				assert(hashes != nullptr);
				count = BRMerkleBlockTxHashes(block, hashes, count);
				for (size_t i = count; i > 0; i--) { // reverse order for more efficient removal as tx arrive
					if (BRSetContains(ctx->knownTxHashSet, &hashes[i - 1])) continue;
					array_add(ctx->currentBlockTxHashes, hashes[i - 1]);
				}

				if (hashes != _hashes) free(hashes);
			}

			if (block) {
				if (array_count(ctx->currentBlockTxHashes) > 0) { // wait til we get all tx messages before processing the block
					ctx->currentBlock = block;
				}
				else if (ctx->relayedBlock) {
					ctx->relayedBlock(ctx->info, block);
				} else {
					ELAMerkleBlockFree(elablock);
				}
			}
			return r;
		}

		void MerkleBlockMessage::Send(BRPeer *peer, void *serializable) {
		}
	}
}
