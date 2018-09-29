// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <iostream>
#include <cstring>
#include <Core/BRTransaction.h>
#include <SDK/Wrapper/Address.h>

#include "TransactionOutput.h"
#include "Utils.h"
#include "Log.h"
#include "Key.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionOutput::TransactionOutput() :
				_assetId(UINT256_ZERO),
				_outputLock(0),
				_programHash(UINT168_ZERO),
				_signType(ELA_STANDARD) {
		}

		TransactionOutput::TransactionOutput(const TransactionOutput &output) {
			SetScript(output.getScript(), output.getAddressSignType());
			_amount = output.getAmount();
			_assetId = output.getAssetId();
			_programHash = output.getProgramHash();
			_outputLock = output.getOutputLock();

		}

		TransactionOutput::TransactionOutput(uint64_t a, const CMBlock &script, int _signType) {
			SetScript(script, _signType);
			_amount = a;
			_assetId = Key::getSystemAssetId();
			if (!Utils::UInt168FromAddress(_programHash, _address)) {
				Log::getLogger()->error("Invalid receiver _address: {}", _address);
			}
		}

		TransactionOutput::~TransactionOutput() {
		}

		std::string TransactionOutput::getAddress() const {
			return _address;
		}

		void TransactionOutput::setAddress(const std::string &addr) {
			_script.Zero();
			memset(_address, 0, sizeof(_address));

			if (!addr.empty()) {
				strncpy(_address, addr.data(), sizeof(_address) - 1);
				size_t len = BRAddressScriptPubKey(NULL, 0, _address);
				_script.Resize(len);
				BRAddressScriptPubKey(_script, _script.GetSize(), _address);
			}
		}

		void TransactionOutput::setAddressSignType(int type) {
			_signType = type;
		}

		int TransactionOutput::getAddressSignType() const {
			return _signType;
		}

		uint64_t TransactionOutput::getAmount() const {
			return _amount;
		}

		void TransactionOutput::setAmount(uint64_t a) {
			_amount = a;
		}

		const CMBlock &TransactionOutput::getScript() const {
			return _script;
		}

		void TransactionOutput::Serialize(ByteStream &ostream) const {
			ostream.writeBytes(_assetId.u8, sizeof(_assetId));
			ostream.writeUint64(_amount);
			ostream.writeUint32(_outputLock);
			ostream.writeBytes(_programHash.u8, sizeof(_programHash));
		}

		bool TransactionOutput::Deserialize(ByteStream &istream) {
			if (!istream.readBytes(_assetId.u8, sizeof(_assetId))) {
				Log::getLogger()->error("deserialize output assetid error");
				return false;
			}

			if (!istream.readUint64(_amount)) {
				Log::getLogger()->error("deserialize output _amount error");
				return false;
			}

			if (!istream.readUint32(_outputLock)) {
				Log::getLogger()->error("deserialize output lock error");
				return false;
			}

			if (!istream.readBytes(_programHash.u8, sizeof(_programHash))) {
				Log::getLogger()->error("deserialize output program hash error");
				return false;
			}

			setAddress(Utils::UInt168ToAddress(_programHash));

			return true;
		}

		const UInt256 &TransactionOutput::getAssetId() const {
			return _assetId;
		}

		void TransactionOutput::setAssetId(const UInt256 &assetId) {
			_assetId = assetId;
		}

		uint32_t TransactionOutput::getOutputLock() const {
			return _outputLock;
		}

		void TransactionOutput::setOutputLock(uint32_t lock) {
			_outputLock = lock;
		}

		const UInt168 &TransactionOutput::getProgramHash() const {
			return _programHash;
		}

		void TransactionOutput::setProgramHash(const UInt168 &hash) {
			_programHash = hash;
		}

		nlohmann::json TransactionOutput::toJson() const {
			nlohmann::json jsonData;

			std::string addr = _address;
			jsonData["Address"] = addr;
			jsonData["Amount"] = _amount;
			jsonData["ScriptLen"] = _script.GetSize();
			jsonData["Script"] = Utils::encodeHex(_script);
			jsonData["AssetId"] = Utils::UInt256ToString(_assetId);
			jsonData["OutputLock"] = _outputLock;
			jsonData["ProgramHash"] = Utils::UInt168ToString(_programHash);
			jsonData["SignType"] = _signType;
			return jsonData;
		}

		void TransactionOutput::fromJson(const nlohmann::json &jsonData) {
			std::string addr = jsonData["Address"].get<std::string>();
			size_t addressSize = sizeof(addr);
			strncpy(_address, addr.c_str(), addressSize - 1);
			_address[addressSize - 1] = 0;

			_amount = jsonData["Amount"].get<uint64_t>();
			_signType = jsonData["SignType"].get<int>();
			
			size_t scriptLen = jsonData["ScriptLen"].get<size_t>();
			std::string scriptString = jsonData["Script"].get<std::string>();
			if (scriptLen > 0) {
				if (scriptLen == scriptString.length() / 2) {
					SetScript(Utils::decodeHex(scriptString), _signType);
				} else {
					Log::getLogger()->error("scriptLen={} and script=\"{}\" do not match of json",
											scriptLen, scriptString);
				}
			}

			_assetId = Utils::UInt256FromString(jsonData["AssetId"].get<std::string>());
			_outputLock = jsonData["OutputLock"].get<uint32_t>();
			_programHash = Utils::UInt168FromString(jsonData["ProgramHash"].get<std::string>());
		}

		void TransactionOutput::SetScript(const CMBlock &script, int type) {
			_script.Zero();
			memset(_address, 0, sizeof(_address));
			_signType = type;

			if (script) {
				_script.Resize(script.GetSize());
				memcpy(_script, script, script.GetSize());

				AddressPtr addressPtr = Address::fromScriptPubKey(_script, _signType);
				std::string addr = addressPtr->stringify();
				strncpy(_address, addr.c_str(), sizeof(addr) - 1);
			}
		}

	}
}