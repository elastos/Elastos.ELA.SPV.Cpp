// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERPUBKEY_H__
#define __ELASTOS_SDK_MASTERPUBKEY_H__

#include "Key.h"

#include <SDK/Common/ByteStream.h>

#include <Core/BRBIP32Sequence.h>

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class MasterPubKey :
			public Wrapper<BRMasterPubKey> {
		public:

			MasterPubKey();

			MasterPubKey(const std::string &phrase, const std::string &phrasePassword = "");

			MasterPubKey(const CMBlock &pubKey, const UInt256 &chainCode);

			MasterPubKey(const BRKey &key, const UInt256 &chainCode);

			explicit MasterPubKey(const BRMasterPubKey &pubKey);

			virtual std::string toString() const;

			virtual BRMasterPubKey *getRaw() const;

			void Serialize(ByteStream &stream) const;

			bool Deserialize(ByteStream &stream);

			uint32_t getFingerPrint() const;

			CMBlock getPubKey() const;

			const UInt256 &getChainCode() const;

		private:
			boost::shared_ptr<BRMasterPubKey> _masterPubKey;
		};

		typedef boost::shared_ptr<MasterPubKey> MasterPubKeyPtr;

	}
}

#endif //__ELASTOS_SDK_MASTERPUBKEY_H__
