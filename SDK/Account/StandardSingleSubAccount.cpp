// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Utils.h>
#include "StandardSingleSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		StandardSingleSubAccount::StandardSingleSubAccount(const MasterPubKey &masterPubKey, IAccount *account,
														   uint32_t coinIndex) :
				SingleSubAccount(account) {
				_masterPubKey = masterPubKey;
				_coinIndex = coinIndex;
		}

		Key StandardSingleSubAccount::DeriveMainAccountKey(const std::string &payPassword) {
			UInt512 seed = _parentAccount->DeriveSeed(payPassword);
			Key key;
			UInt256 chainCode;
			BRBIP32PrivKeyPath(key.getRaw(), &chainCode, &seed, sizeof(seed), 3, 44 | BIP32_HARD,
							   _coinIndex | BIP32_HARD, 0 | BIP32_HARD);
			var_clean(&seed);
			return key;
		}

		std::string StandardSingleSubAccount::GetMainAccountPublicKey() const {
			return Utils::encodeHex(_masterPubKey.getPubKey());
		}

		WrapperList<Key, BRKey> StandardSingleSubAccount::DeriveAccountAvailableKeys(const std::string &payPassword,
																					 const TransactionPtr &transaction) {
			WrapperList<Key, BRKey> result;
			UInt512 seed = _parentAccount->DeriveSeed(payPassword);
			Key key;
			UInt256 chainCode;
			BRBIP32PrivKeyPath(key.getRaw(), &chainCode, &seed, sizeof(seed), 5, 44 | BIP32_HARD,
							   _coinIndex | BIP32_HARD, 0 | BIP32_HARD, 1, 0);
			var_clean(&seed);
			result.push_back(key);
			return result;
		}
	}
}
