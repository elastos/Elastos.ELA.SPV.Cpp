// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__NEOPLUGIN_H__
#define __ELASTOS_SDK__NEOPLUGIN_H__

#include <SDK/Plugin/Interface/IPlugin.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/NeoMerkleBlock.h>

#include <fruit/fruit.h>

namespace Elastos {
	namespace ElaWallet {

		class NeoPlugin : public IPlugin {
		public:
			INJECT (NeoPlugin(INeoMerkleBlockFactory * merkleBlockFactory)) :
					_merkleBlockFactory(merkleBlockFactory) {
			}

			virtual MerkleBlockPtr CreateBlock();
		private:
			INeoMerkleBlockFactory *_merkleBlockFactory;
		};

		fruit::Component<> getNeoPluginComponent();
	}
}

#endif //__ELASTOS_SDK__NEOPLUGIN_H__
