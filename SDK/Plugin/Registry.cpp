// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Registry.h"
#include "Block/MerkleBlock.h"
#include "Block/SidechainMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		Registry::Registry() {

		}

		Registry *Registry::Instance(bool erase) {
			static std::shared_ptr<Registry> instance(new Registry);
			if (erase) {
				instance.reset();
				instance = nullptr;
			}
			return instance.get();
		}

		IMerkleBlock * Registry::CreateMerkleBlock(const std::string &blockType) {
//			IMerkleBlock *result = nullptr;
//			if (blockType == "ELA") {
//				fruit::Injector<IMerkleBlock> injector(GetMerkleBlockComponent);
//				result = injector.get<IMerkleBlock*>();
//			} else if (blockType == "SideStandard") {
//					injector = boost::make_shared<MerkleBlockInj>(GetSidechainMerkleBlockComponent);
//			}

//			return result;
			return new MerkleBlock();
		}

	}
}