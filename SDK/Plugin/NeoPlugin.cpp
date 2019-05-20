// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NeoPlugin.h"

namespace Elastos {
	namespace ElaWallet {
		MerkleBlockPtr NeoPlugin::CreateBlock() {
			return _merkleBlockFactory->createBlock();
		}

		fruit::Component<> getNeoPluginComponent() {
			return fruit::createComponent().addMultibinding<IPlugin, NeoPlugin>().install(
					getNeoBlockFactoryComponent);

		}

		REGISTER_MERKLEBLOCKPLUGIN(Neo, getNeoPluginComponent);

	}
}