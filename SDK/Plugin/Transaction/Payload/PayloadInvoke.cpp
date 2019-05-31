// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadInvoke.h"

#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		PayloadInvoke::PayloadInvoke() {

		}

		PayloadInvoke::~PayloadInvoke() {

		}

		void PayloadInvoke::SetCodeHash(const uint168 &codeHash) {
			_codeHash = codeHash;
		}

		void PayloadInvoke::SetCode(const bytes_t &code) {
			_code = code;
		}

		void PayloadInvoke::SetProgramHash(const uint168 &programHash) {
			_programHash = programHash;
		}

		void PayloadInvoke::SetGas(const uint64_t &gas) {
			_gas = gas;
		}

		const uint168 &PayloadInvoke::GetCodeHash() const {
			return _codeHash;
		}

		const bytes_t &PayloadInvoke::GetCode() const {
			return _code;
		}

		const uint168 &PayloadInvoke::GetProgramHash() const {
			return _programHash;
		}

		const uint64_t &PayloadInvoke::GetGas() const {
			return _gas;
		}

		void PayloadInvoke::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteBytes(_codeHash);

			ostream.WriteVarBytes(_code);

			ostream.WriteBytes(_programHash);

			ostream.WriteUint64(_gas);
		}

		bool PayloadInvoke::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadBytes(_codeHash)) {
				Log::error("deserialize PayloadInvoke _codeHash error");
				return false;
			}

			if (!istream.ReadVarBytes(_code)) {
				Log::error("deserialize PayloadInvoke _code error");
				return false;
			}

			if (!istream.ReadBytes(_programHash)) {
				Log::error("deserialize PayloadInvoke _programHash error");
				return false;
			}

			if (!istream.ReadUint64(_gas)) {
				Log::error("deserialize PayloadInvoke _gas error");
				return false;
			}
			return true;
		}

		nlohmann::json PayloadInvoke::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["codeHash"] = _codeHash.GetHex();

			j["code"] = _code.getHex();

			j["programHash"] = _programHash.GetHex();

			j["gas"] = _gas;

			return j;
		}

		void PayloadInvoke::FromJson(const nlohmann::json &j, uint8_t version) {
			std::string codeHash = j["codeHash"].get<std::string>();
			_codeHash.SetHex(codeHash);

			std::string codeHex = j["code"].get<std::string>();
			_code.setHex(codeHex);

			_programHash.SetHex(j["programHash"].get<std::string>());

			_gas = j["gas"].get<uint64_t>();
		}

		IPayload &PayloadInvoke::operator=(const IPayload &payload) {
			try {
				const PayloadInvoke &payloadInvoke = dynamic_cast<const PayloadInvoke &>(payload);
				operator=(payloadInvoke);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadDeploy");
			}

			return *this;
		}

		PayloadInvoke &PayloadInvoke::operator=(const PayloadInvoke &payload) {

			return *this;
		}
	}
}