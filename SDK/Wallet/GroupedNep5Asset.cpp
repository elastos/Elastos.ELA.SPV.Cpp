// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GroupedNep5Asset.h"

#include <SDK/Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {

		GroupedNep5Asset::GroupedNep5Asset() {

		}

		GroupedNep5Asset::~GroupedNep5Asset() {

		}

		BigInt GroupedNep5Asset::GetBalance(const std::string &addr) const {
			if (ContainsAddr(addr)) {
				return _balanceMap[addr];
			}
			return 0;
		}

		BigInt GroupedNep5Asset::GetBalance() const {
			BigInt balance = 0;
			for(std::map<std::string, BigInt>::iterator iter = _balanceMap.begin(); iter != _balanceMap.end(); iter++) {
				balance += iter->second;
			}
			return balance;
		}

		void GroupedNep5Asset::AddBalance(const std::string &addr, const BigInt &value) {
			if (ContainsAddr(addr)) {
				_balanceMap[addr] += value;
			} else {
				_balanceMap[addr] = value;
			};
		}

		void GroupedNep5Asset::MinusBalance(const std::string &addr, const BigInt &value) {
			ErrorChecker::CheckLogic(ContainsAddr(addr) == false, Error::InvalidArgument, "Invalid Nep5log addrress hash");
			_balanceMap[addr] -= value;
		}

		bool GroupedNep5Asset::ContainsAddr(const std::string &addr) const {
			return _balanceMap.find(addr) != _balanceMap.end();
		}
	}
}