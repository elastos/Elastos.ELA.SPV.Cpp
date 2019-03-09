// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ATTRIBUTE_H__
#define __ELASTOS_SDK_ATTRIBUTE_H__

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <SDK/Common/CMemBlock.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Attribute :
				public ELAMessageSerializable {
		public:
			enum Usage {
				Nonce = 0x00,
				Script = 0x20,
				DescriptionUrl = 0x91,
				Description = 0x90,
				Memo = 0x81,
				Confirmations = 0x92
			};

		public:
			Attribute();

			Attribute(const Attribute &attr);

			Attribute(Usage usage, const CMBlock &data);

			~Attribute();

			Usage GetUsage() const;

			const CMBlock &GetData() const;

			bool IsValid() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &jsonData);

		private:
			Usage _usage;
			CMBlock _data;
		};

		typedef boost::shared_ptr<Attribute> AttributePtr;

	}
}

#endif //__ELASTOS_SDK_ATTRIBUTE_H__
