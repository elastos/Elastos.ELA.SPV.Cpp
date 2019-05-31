// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADINVOKE_H__
#define __ELASTOS_SDK_PAYLOADINVOKE_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {
		class PayloadInvoke :
				public IPayload {
		public:
			PayloadInvoke();

			~PayloadInvoke();

			void SetCodeHash(const uint168 &codeHash);

			void SetCode(const bytes_t &code);

			void SetProgramHash(const uint168 &programHash);

			void SetGas(const uint64_t &gas);

			const uint168 &GetCodeHash() const;

			const bytes_t &GetCode() const;

			const uint168 &GetProgramHash() const;

			const uint64_t &GetGas() const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadInvoke &operator=(const PayloadInvoke &payload);

		private:
			uint168 _codeHash;
			bytes_t _code;
			uint168 _programHash;
			uint64_t _gas;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADINVOKE_H__
