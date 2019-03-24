// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADIDCHAIN_H__
#define __ELASTOS_SDK_PAYLOADIDCHAIN_H__

#include "IPayload.h"
#include <SDK/Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadRegisterIdentification : public IPayload {
		public:
			struct ValueItem {
				uint256 DataHash;
				std::string Proof;
				std::string Info;
			};

			struct SignContent {
				std::string Path;
				std::vector<ValueItem> Values;
			};

		public:
			PayloadRegisterIdentification();

			PayloadRegisterIdentification(const PayloadRegisterIdentification &payload);

			~PayloadRegisterIdentification();

			const std::string &GetID() const;

			void SetID(const std::string &id);

			const std::string &GetPath(size_t index) const;

			void SetPath(const std::string &path, size_t index);

			const uint256 &GetDataHash(size_t index, size_t valueIndex) const;

			void SetDataHash(const uint256 &dataHash, size_t index, size_t valueIndex);

			const std::string &GetProof(size_t index, size_t valueIndex) const;

			void SetProof(const std::string &proof, size_t index, size_t valueIndex);

			size_t GetContentCount() const;

			void AddContent(const SignContent &content);

			void RemoveContent(size_t index);

			const bytes_t &GetSign() const;

			void SetSign(const bytes_t &sign);

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool IsValid() const;

			virtual IPayload &operator=(const IPayload &payload);

			PayloadRegisterIdentification &operator=(const PayloadRegisterIdentification &payload);

		private:
			std::string _id;
			bytes_t _sign;
			std::vector<SignContent> _contents;
		};

	}
}

#endif //__ELASTOS_SDK_PAYLOADIDCHAIN_H__
