// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ADDRESSMESSAGE_H_
#define __ELASTOS_SDK_ADDRESSMESSAGE_H_

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		class AddressMessage :
				public Message {
		public:
			AddressMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const std::string &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;

		private:
			MessagePeerPtr _peer;
		};

	}
}


#endif //__ELASTOS_SDK_ADDRESSMESSAGE_H_
