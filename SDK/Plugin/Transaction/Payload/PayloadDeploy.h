// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADDEPLOY_H__
#define __ELASTOS_SDK_PAYLOADDEPLOY_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		typedef enum {
			Signature,
			Boolean,
			Integer,
			Hash160,
			Hash256,
			ByteArray,
			PublicKey,
			String,
			Object,

			Array = 0x10,
			Map = 0x12,
			Void = 0xff
		} ParameterType;

		struct FunctionCode {
			bytes_t code;

			std::vector<ParameterType> parameters;

			ParameterType returnType;

			uint168 codeHash;
		};

		class PayloadDeploy :
				public IPayload {
		public:
			PayloadDeploy();

			~PayloadDeploy();

			void SetFunctionCode(const FunctionCode &functionCode);

			void SetName(const std::string &name);

			void SetVersion(const std::string &version);

			void SetAuthor(const std::string &author);

			void SetEmail(const std::string &email);

			void SetDescription(const std::string &desc);

			void SetProgramHash(const uint168 &programHash);

			void SetGas(const uint64_t &gas);

			const FunctionCode &GetFunctionCode() const;

			const std::string &GetName() const;

			const std::string &GetVersion() const;

			const std::string &GetAuthor() const;

			const std::string &GetEmail() const;

			const std::string &GetDescription() const;

			const uint168 &GetProgramHash() const;

			const uint64_t &GetGas() const;


			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadDeploy &operator=(const PayloadDeploy &payload);

		private:
			FunctionCode _functionCode;
			std::string _name;
			std::string _version;
			std::string _author;
			std::string _email;
			std::string _description;
			uint168 _programHash;
			uint64_t _gas;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADDEPLOY_H__
