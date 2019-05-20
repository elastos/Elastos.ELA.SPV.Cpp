//
// Created by Apple on 5/20/19.
//

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
