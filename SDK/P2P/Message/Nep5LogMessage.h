// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__NEP5LOGMESSAGE_H__
#define __ELASTOS_SDK__NEP5LOGMESSAGE_H__

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		struct Nep5LogMessageParameter : public SendMessageParameter {
			const uint256 blockHash;
			const uint32_t height;
			const uint160 topicAddr;

			Nep5LogMessageParameter(const uint256 hash, const uint32_t height, uint160 &topic) :
					blockHash(hash),
					height(height),
					topicAddr(topic) {}
		};

		class Nep5LogMessage : public Message {
		public:
			explicit Nep5LogMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const bytes_t &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;
		};
	}
}

#endif //__ELASTOS_SDK__NEP5LOGMESSAGE_H__
