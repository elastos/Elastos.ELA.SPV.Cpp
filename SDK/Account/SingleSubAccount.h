// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SINGLESUBACCOUNT_H__
#define __ELASTOS_SDK_SINGLESUBACCOUNT_H__

#include "SubAccountBase.h"

namespace Elastos {
	namespace ElaWallet {

		class SingleSubAccount : public SubAccountBase {
		public:
			SingleSubAccount(IAccount *account);

			~SingleSubAccount();

			virtual nlohmann::json GetBasicInfo() const;

			virtual CMBlock GetRedeemScript(const Address &addr) const;

			virtual bool IsSingleAddress() const;

			virtual std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal);

			virtual std::vector<Address> GetAllAddresses(uint32_t start, size_t addrsCount, bool containInternal) const;

			virtual bool ContainsAddress(const Address &address) const;

			virtual bool IsAddressUsed(const Address &address) const;

			virtual Key DeriveVoteKey(const std::string &payPasswd);

		};

	}
}


#endif //__ELASTOS_SDK_SINGLESUBACCOUNT_H__
