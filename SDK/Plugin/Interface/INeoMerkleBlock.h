// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_INEOMERKLEBLOCK_H__
#define __ELASTOS_SDK_INEOMERKLEBLOCK_H__

namespace Elastos {
	namespace ElaWallet {
		class INeoMerkleBlock {
			virtual bytes_t GetBloom() const = 0;
		};
	}
}

#endif //__ELASTOS_SDK_INEOMERKLEBLOCK_H__
