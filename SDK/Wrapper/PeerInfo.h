// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERINFO_H__
#define __ELASTOS_SDK_PEERINFO_H__

#include "BRInt.h"

//#define SERVICES_NODE_NETWORK 0x01 // services value indicating a node carries full blocks, not just headers
//#define SERVICES_NODE_BLOOM   0x04 // BIP111: https://github.com/bitcoin/bips/blob/master/bip-0111.mediawiki
//#define SERVICES_NODE_BCASH   0x20 // https://github.com/Bitcoin-UAHF/spec/blob/master/uahf-technical-spec.md
#define SERVICES_NODE_NETWORK 0x04 // services value indicating a node carries full blocks, not just headers
#define SERVICES_NODE_BLOOM   0x05 // BIP111: https://github.com/bitcoin/bips/blob/master/bip-0111.mediawiki
#define SERVICES_NODE_BCASH   0x20 // https://github.com/Bitcoin-UAHF/spec/blob/master/uahf-technical-spec.md

namespace Elastos {
	namespace ElaWallet {

		struct PeerInfo {
			UInt128 Address; // IPv6 address of peer
			uint16_t Port; // port number for peer connection
			uint64_t Services; // bitcoin network services supported by peer
			uint64_t Timestamp; // timestamp reported by peer
			uint8_t Flags; // scratch variable

			PeerInfo();

			PeerInfo(const UInt128 &addr, uint16_t port, uint64_t timestamp);

			PeerInfo(const UInt128 &addr, uint16_t port, uint64_t timestamp, uint64_t services);

			PeerInfo(const PeerInfo &peerInfo);

			PeerInfo &operator=(const PeerInfo &peerInfo);

			bool operator==(const PeerInfo &info) const;

			bool operator!=(const PeerInfo &info) const;
		};

	}
}

#endif //__ELASTOS_SDK_PEERINFO_H__
