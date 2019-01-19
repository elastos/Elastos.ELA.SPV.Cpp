// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IOUTPUTPAYLOAD_H__
#define __ELASTOS_SDK_IOUTPUTPAYLOAD_H__

#include <SDK/Common/CMemBlock.h>
#include <SDK/Common/ByteStream.h>
#include <nlohmann/json.hpp>
#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class IOutputPayload {
		public:
			virtual ~IOutputPayload();

			virtual CMBlock getData() const;

			virtual void Serialize(ByteStream &ostream) const = 0;
			virtual bool Deserialize(ByteStream &istream) = 0;

			virtual nlohmann::json toJson() const = 0;
			virtual void fromJson(const nlohmann::json &) = 0;

			virtual IOutputPayload &operator=(const IOutputPayload &payload) = 0;
		};

		typedef boost::shared_ptr<IOutputPayload> OutputPayloadPtr;

	}
}

#endif //__ELASTOS_SDK_IOUTPUTPAYLOAD_H__
