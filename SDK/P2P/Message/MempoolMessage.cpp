// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MempoolMessage.h"
#include <SDK/P2P/Peer.h>

#include <sys/time.h>

namespace Elastos {
	namespace ElaWallet {

		MempoolMessage::MempoolMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool MempoolMessage::Accept(const CMBlock &msg) {
			_peer->info("drop {} message, not implemented.", Type());
			return false;
		}

		void MempoolMessage::Send(const SendMessageParameter &param) {
			const MempoolParameter &mempoolParameter = dynamic_cast<const MempoolParameter &>(param);

			struct timeval tv;
			bool sentMempool = _peer->SentMempool();
			_peer->SetSentMempool(true);

			if (!sentMempool && _peer->GetMemPoolCallback().empty()) {
				_peer->AddKnownTxHashes(mempoolParameter.KnownTxHashes);

				if (!mempoolParameter.CompletionCallback.empty()) {
					gettimeofday(&tv, NULL);

					_peer->SetMempoolTime(tv.tv_sec + (double) tv.tv_usec / 1000000 + 10.0);
					_peer->SetMempoolCallback(mempoolParameter.CompletionCallback);
				}

				_peer->SendMessage(CMBlock(), Type());
			} else {
				_peer->SendMessage(CMBlock(), Type());
//				_peer->info("mempool request already sent");
//				if (mempoolParameter.CompletionCallback) mempoolParameter.CompletionCallback(0);
			}
		}

		std::string MempoolMessage::Type() const {
			return MSG_MEMPOOL;
		}
	}
}

