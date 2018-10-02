// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubAccountBase.h"

namespace Elastos {
	namespace ElaWallet {

		SubAccountBase::SubAccountBase(IAccount *account) :
			_parentAccount(account) {

		}

		IAccount *SubAccountBase::GetParent() const {
			return _parentAccount;
		}

		Address SubAccountBase::KeyToAddress(const BRKey *key) const {
			BRKey *brKey = new BRKey;
			memcpy(brKey, key, sizeof(BRKey));

			Key keyPtr(brKey);
			return keyPtr.address();
		}

	}
}
