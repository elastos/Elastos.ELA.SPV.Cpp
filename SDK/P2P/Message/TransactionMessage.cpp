// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionMessage.h"

#include <SDK/P2P/Peer.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <Core/BRTransaction.h>
#include <Core/BRArray.h>

namespace Elastos {
	namespace ElaWallet {

		TransactionMessage::TransactionMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool TransactionMessage::Accept(const CMBlock &msg) {
			ByteStream stream(msg);
			TransactionPtr tx = TransactionPtr(new Transaction());

			UInt256 txHash;

			if (!tx->Deserialize(stream)) {
				_peer->error("malformed tx message with length: {}", msg.GetSize());
				return false;
			} else if (!_peer->SentFilter() && !_peer->SentGetdata()) {
				_peer->error("got tx message before loading filter");
				return false;
			} else {
				txHash = tx->getHash();
				_peer->debug("got tx: {}", Utils::UInt256ToString(txHash, true));

				FireRelayedTx(tx);

				if (_peer->CurrentBlock() != nullptr) { // we're collecting tx messages for a merkleblock
					_peer->CurrentBlockTxHashesRemove(txHash);

					if (_peer->CurrentBlockTxHashes().size() == 0) { // we received the entire block including all matched tx
						const MerkleBlockPtr block = _peer->CurrentBlock();

						_peer->SetCurrentBlock(nullptr);
						FireRelayedBlock(block);
					}
				}
			}

			return true;
		}

		void TransactionMessage::Send(const SendMessageParameter &param) {
			const TransactionParameter &txParam = static_cast<const TransactionParameter &>(param);

			ByteStream stream;
			txParam.tx->Serialize(stream);
			CMBlock buf = stream.getBuffer();
			_peer->info("sending tx: tx hash = {}", Utils::UInt256ToString(txParam.tx->getHash(), true));
			SendMessage(stream.getBuffer(), Type());
		}

		std::string TransactionMessage::Type() const {
			return MSG_TX;
		}

	}
}
