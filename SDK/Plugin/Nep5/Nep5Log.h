// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_NEP5LOG_H
#define __ELASTOS_SDK_NEP5LOG_H

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <SDK/Common/BigInt.h>
#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {
		class Nep5Log : public ELAMessageSerializable {
		public:
			Nep5Log();

			Nep5Log(const std::string &nep5Hash, const std::string &from, const std::string &to, const BigInt &data,
			        const std::string &txid);

			~Nep5Log();

			void SetNep5Hash(const std::string &nep5Hash);

			void SetFrom(const std::string &from);

			void SetTo(const std::string &to);

			void SetData(const BigInt &value);

			void SetTxId(const std::string &txid);

			const std::string &GetNep5Hash() const;

			const std::string &GetFrom() const;

			const std::string &GetTo() const;

			const BigInt &GetData() const;

			const std::string &GetTxID() const;

		public:
			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

		private:
			std::string _nep5Hash;
			std::string _from;
			std::string _to;
			BigInt _data;
			std::string _txid;
		};

		typedef boost::shared_ptr<Nep5Log> Nep5LogPtr;
	}
}

#endif //__ELASTOS_SDK_NEP5LOG_H
