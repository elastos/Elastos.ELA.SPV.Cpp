// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ADDRESS_H__
#define __ELASTOS_SDK_ADDRESS_H__

#include <string>
#include <boost/shared_ptr.hpp>

#include "BRAddress.h"

#include "Wrapper.h"

#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class Address :
				public Wrapper<BRAddress *> {
		public:
			Address(std::string address);

			Address(boost::shared_ptr<BRAddress> address);

			static Address *createAddress(std::string address);

			static Address *fromScriptPubKey(ByteData script);

			static Address *fromScriptSignature(ByteData script);

			std::string stringify() const;

			bool isValid();

			ByteData getPubKeyScript();

			virtual std::string toString() const;

			virtual BRAddress* getRaw() const;
			//todo complete me
		private:
			boost::shared_ptr<BRAddress> _address;
		};

		typedef boost::shared_ptr<Address> AddressPtr;

	}
}

#endif //__ELASTOS_SDK_ADDRESS_H__
