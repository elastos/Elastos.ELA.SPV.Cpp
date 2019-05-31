// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadDeploy.h"

#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		PayloadDeploy::PayloadDeploy() {
		}

		PayloadDeploy::~PayloadDeploy() {
			_functionCode.parameters.clear();
			_functionCode.code.clear();
		}

		void PayloadDeploy::SetFunctionCode(const FunctionCode functionCode) {
			_functionCode.code = functionCode.code;
			_functionCode.parameters.assign(functionCode.parameters.begin(), functionCode.parameters.end());
			_functionCode.returnType = functionCode.returnType;
			_functionCode.codeHash.SetHex(functionCode.codeHash.GetHex());

		}

		void PayloadDeploy::SetName(const std::string &name) {
			_name = name;
		}

		void PayloadDeploy::SetVersion(const std::string &version) {
			_version = version;
		}

		void PayloadDeploy::SetAuthor(const std::string &author) {
			_author = author;
		}

		void PayloadDeploy::SetEmail(const std::string &email) {
			_email = email;
		}

		void PayloadDeploy::SetDescription(const std::string &desc) {
			_description = desc;
		}

		void PayloadDeploy::SetProgramHash(const uint168 &programHash) {
			_programHash = programHash;
		}

		void PayloadDeploy::SetGas(const uint64_t &gas) {
			_gas = gas;
		}

		const FunctionCode &PayloadDeploy::GetFunctionCode() const {
			return _functionCode;
		}

		const std::string &PayloadDeploy::GetName() const {
			return _name;
		}

		const std::string &PayloadDeploy::GetVersion() const {
			return _version;
		}

		const std::string &PayloadDeploy::GetAuthor() const {
			return _author;
		}

		const std::string &PayloadDeploy::GetEmail() const {
			return _email;
		}

		const std::string &PayloadDeploy::GetDescription() const {
			return _description;
		}

		const uint168 &PayloadDeploy::GetProgramHash() const {
			return _programHash;
		}

		const uint64_t &PayloadDeploy::GetGas() const {
			return _gas;
		}

		void PayloadDeploy::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarString(_functionCode.code.getHex());

			size_t len = _functionCode.parameters.size();
			ostream.WriteVarUint(len);
			for (size_t i = 0; i < len; ++i) {
				ostream.WriteByte(_functionCode.parameters[i]);
			}

			ostream.WriteByte(_functionCode.returnType);

			ostream.WriteBytes(_functionCode.codeHash);

			ostream.WriteVarString(_name);

			ostream.WriteVarString(_version);

			ostream.WriteVarString(_author);

			ostream.WriteVarString(_email);

			ostream.WriteVarString(_description);

			ostream.WriteBytes(_programHash);

			ostream.WriteVarUint(_gas);

		}

		bool PayloadDeploy::Deserialize(const ByteStream &istream, uint8_t version) {
			std::string codeHex;
			if (!istream.ReadVarString(codeHex)) {
				Log::error(("deserialize payloadDeploy code error"));
				return false;
			}
			_functionCode.code.setHex(codeHex);

			uint64_t len;
			istream.ReadVarUint(len);
			_functionCode.parameters.resize(len);
			for (size_t i = 0; i < len; ++i) {
				uint8_t parameter;
				if (!istream.ReadByte(parameter)) {
					Log::error(("deserialize payloadDeploy parameter error"));
					return false;
				}
				_functionCode.parameters[i] = static_cast<ParameterType>(parameter);
			}

			uint8_t returnType;
			if (!istream.ReadByte(returnType)) {
				Log::error(("deserialize payloadDeploy returnType error"));
				return false;
			}
			_functionCode.returnType = static_cast<ParameterType>(returnType);

			if (!istream.ReadBytes(_functionCode.codeHash)) {
				Log::error(("deserialize payloadDeploy codeHash error"));
				return false;
			}

			if (!istream.ReadVarString(_name)) {
				Log::error(("deserialize payloadDeploy _name error"));
				return false;
			}

			if (!istream.ReadVarString(_version)) {
				Log::error(("deserialize payloadDeploy _version error"));
				return false;
			}

			if (!istream.ReadVarString(_author)) {
				Log::error(("deserialize payloadDeploy _author error"));
				return false;
			}

			if (!istream.ReadVarString(_email)) {
				Log::error(("deserialize payloadDeploy _email error"));
				return false;
			}

			if (!istream.ReadVarString(_description)) {
				Log::error(("deserialize payloadDeploy _description error"));
				return false;
			}

			if (!istream.ReadBytes(_programHash)) {
				Log::error(("deserialize payloadDeploy _programHash error"));
				return false;
			}

			if (!istream.ReadVarUint(_gas)) {
				Log::error(("deserialize payloadDeploy _gas error"));
				return false;
			}

			return true;
		}

		nlohmann::json PayloadDeploy::ToJson(uint8_t version) const {
			nlohmann::json j;

			nlohmann::json codeJson;
			codeJson["code"] = _functionCode.code.getHex();
			size_t len = _functionCode.parameters.size();
			std::vector<nlohmann::json> parameters(len);
			for(size_t i = 0; i < len; ++i) {
				parameters[i] = _functionCode.parameters[i];
			}
			codeJson["parameters"] = parameters;
			codeJson["returnType"] = _functionCode.returnType;
			codeJson["codeHash"] = _functionCode.codeHash.GetHex();

			j["function"] = codeJson;

			j["name"] = _name;

			j["version"] = _version;

			j["author"] = _author;

			j["email"] = _email;

			j["description"] = _description;

			j["programHash"] = _programHash.GetHex();

			j["gas"] = _gas;


			return j;
		}

		void PayloadDeploy::FromJson(const nlohmann::json &j, uint8_t version) {
			nlohmann::json codeJson = j["function"];

			std::string codeHex = codeJson["code"].get<std::string>();
			_functionCode.code.setHex(codeHex);

			std::vector<nlohmann::json> parameters = codeJson["parameters"];
			size_t len = parameters.size();
			_functionCode.parameters.resize(len);
			for (size_t i = 0; i < len; ++i) {
				_functionCode.parameters[i] = parameters[i].get<ParameterType >();
			}

			_functionCode.returnType = codeJson["returnType"].get<ParameterType>();

			std::string codeHash = codeJson["codeHash"].get<std::string>();
			_functionCode.codeHash.SetHex(codeHash);

			_name = j["name"].get<std::string>();

			_version = j["version"].get<std::string>();

			_author = j["author"].get<std::string>();

			_email = j["email"].get<std::string>();

			_description = j["description"].get<std::string>();

			_programHash.SetHex(j["programHash"].get<std::string>());

			_gas = j["gas"].get<uint64_t>();

		}

		IPayload &PayloadDeploy::operator=(const IPayload &payload) {
			try {
				const PayloadDeploy &payloadDeploy = dynamic_cast<const PayloadDeploy &>(payload);
				operator=(payloadDeploy);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadDeploy");
			}

			return *this;
		}

		PayloadDeploy &PayloadDeploy::operator=(const PayloadDeploy &payload) {
			_functionCode = payload._functionCode;
			_name = payload._name;
			_version = payload._version;
			_author = payload._author;
			_email = payload._email;
			_description = payload._description;
			_programHash = payload._programHash;
			_gas = payload._gas;

			return *this;
		}

	}
}