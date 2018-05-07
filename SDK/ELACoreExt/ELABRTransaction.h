// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELABRTRANSACTION_H
#define __ELASTOS_SDK_ELABRTRANSACTION_H

#include <vector>

#include "BRTransaction.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		typedef struct _ELABRTransaction {
			BRTransaction raw;
			uint8_t  type;
			uint8_t payloadVersion;
			ByteData payloadData;
			std::vector<ByteData> attributeData;
			std::vector<ByteData> programData;

			_ELABRTransaction &operator = (const _ELABRTransaction &ebt);

		} ELABRTransaction;

		ELABRTransaction *ELABRTransactionNew(void);
		ELABRTransaction *ELABRTransactioCopy(const ELABRTransaction *tx);

		void ELABRTransactionFree(ELABRTransaction *tx);
	}
}

#endif //__ELASTOS_SDK_ELABRTRANSACTION_H
