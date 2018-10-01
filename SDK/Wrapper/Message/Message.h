// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IWRAPPERMESSAGE_H__
#define __ELASTOS_SDK_IWRAPPERMESSAGE_H__

#include "SDK/Wrapper/Peer.h"

namespace Elastos {
	namespace ElaWallet {

		struct SendMessageParameter {
		};

		class Message {
		public:
			virtual ~Message();

			virtual const PeerPtr &GetPeer() const = 0;

			virtual int Accept(const std::string &msg) = 0;

			virtual void Send(const SendMessageParameter &param) = 0;

			virtual std::string Type() const = 0;

			static SendMessageParameter DefaultParam;
		};

	}
}

#endif //__ELASTOS_SDK_IWRAPPERMESSAGE_H__
