// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDPLUGIN_H__
#define __ELASTOS_SDK_IDPLUGIN_H__

#include <fruit/fruit.h>

#include "Interface/IPlugin.h"
#include "Block/SidechainMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		struct IDPluginTag {};

		class IDPlugin : public IPlugin {
		public:
			INJECT(IDPlugin(ISidechainMerkleBlockFactory *merkleBlockFactory)) :
					_merkleBlockFactory(merkleBlockFactory) {
			}

			virtual MerkleBlockPtr CreateBlock();

		private:
			ISidechainMerkleBlockFactory *_merkleBlockFactory;
		};

		fruit::Component<fruit::Annotated<IDPluginTag, IPlugin>, IDPlugin> getIDPluginComponent();
	}
}

#endif //__ELASTOS_SDK_IDPLUGIN_H__
