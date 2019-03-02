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

			virtual CMBlock GetRedeemScript(const std::string &addr) const;

			virtual bool IsSingleAddress() const;

			virtual std::vector<std::string> UnusedAddresses(uint32_t gapLimit, bool internal);

			virtual std::vector<std::string> GetAllAddresses(size_t addrsCount) const;

			virtual bool ContainsAddress(const std::string &address) const;

			virtual bool IsAddressUsed(const std::string &address) const;

			virtual Key DeriveVoteKey(const std::string &payPasswd);

		};

	}
}


#endif //__ELASTOS_SDK_SINGLESUBACCOUNT_H__
