// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Nep5LogMessage.h"

#include <SDK/P2P/Peer.h>
#include <SDK/Plugin/Nep5/Nep5Log.h>

namespace Elastos {
	namespace ElaWallet {
		Nep5LogMessage::Nep5LogMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool Nep5LogMessage::Accept(const bytes_t &msg) {
			ByteStream stream(msg);
			uint32_t len = 0;
			stream.ReadUint32(len);
			for (int i = 0; i < len; ++i) {
				Nep5LogPtr nep5LogPtr = Nep5LogPtr(new Nep5Log);
				if (!nep5LogPtr->Deserialize(stream)) {
					_peer->error("malformed tx message with length: {}", msg.size());
					return false;
				}
				FireRelayNep5Log(nep5LogPtr);
			}

			return true;
		}

		void Nep5LogMessage::Send(const SendMessageParameter &param) {
			const Nep5LogMessageParameter &parameter = static_cast<const Nep5LogMessageParameter &>(param);

			ByteStream stream;
			stream.WriteBytes(parameter.blockHash);
			stream.WriteUint32(parameter.height);
			stream.WriteBytes(parameter.topicAddr);
			_peer->info("sending Nep5LogMessage: blockHash = {}", parameter.blockHash.GetHex());
			SendMessage(stream.GetBytes(), Type());
		}

		std::string Nep5LogMessage::Type() const {
			return MSG_NEP5LOG;
		}
	}
}