// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPVSDK_SUBACCOUNTBASE_H
#define SPVSDK_SUBACCOUNTBASE_H

#include "ISubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class SubAccountBase : public ISubAccount {
		public:
			SubAccountBase(IAccount *account);

			virtual void InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock);

			virtual IAccount *GetParent() const;

			virtual void AddUsedAddrs(const TransactionPtr &tx);

			virtual void ClearUsedAddresses();

			virtual bytes_t GetMultiSignPublicKey() const;

			virtual Key DeriveMultiSignKey(const std::string &payPassword);

			virtual bytes_t GetVotePublicKey() const;

			virtual bool IsDepositAddress(const Address &address) const;

			virtual void SignTransaction(const TransactionPtr &transaction, const std::string &payPassword);

			virtual bool FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd);

		protected:
			mutable Address _depositAddress;
			bytes_t _votePublicKey;
			IAccount *_parentAccount;
			HDKeychain _masterPubKey;
			uint32_t _coinIndex;
		};

	}
}

#endif //SPVSDK_SUBACCOUNTBASE_H
