// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPNEP5ASSET_H__
#define __ELASTOS_SDK__GROUPNEP5ASSET_H__


#include <SDK/Common/BigInt.h>
#include <SDK/Plugin/Nep5/Nep5Log.h>

#include <map>

namespace Elastos {
	namespace ElaWallet {

		class GroupedNep5Asset {
		public:
			GroupedNep5Asset();

			~GroupedNep5Asset();

		public:
			BigInt GetBalance(const std::string &addr) const;

			BigInt GetBalance() const;

			void AddBalance(const std::string &addr, const BigInt &value);

			void MinusBalance(const std::string &addr, const BigInt &value);

			bool ContainsAddr(const std::string &addr) const;

		private:
			 mutable std::map<std::string, BigInt> _balanceMap;
		};

		typedef boost::shared_ptr<GroupedNep5Asset> GroupedNep5Ptr;
	}
}

#endif //__ELASTOS_SDK__GROUPNEP5ASSET_H__
