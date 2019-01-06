// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Transaction.h"
#include "Payload/PayloadCoinBase.h"
#include "SDK/Plugin/Transaction/Payload/PayloadRechargeToSideChain.h"
#include "Payload/PayloadWithDrawAsset.h"
#include "Payload/PayloadRecord.h"
#include "Payload/PayloadRegisterAsset.h"
#include "Payload/PayloadSideMining.h"
#include "Payload/PayloadTransferCrossChainAsset.h"
#include "Payload/PayloadTransferAsset.h"
#include "Payload/PayloadRegisterIdentification.h"

#include <SDK/Common/Utils.h>
#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>

#include <Core/BRCrypto.h>
#include <Core/BRAddress.h>
#include <Core/BRTransaction.h>

#include <boost/make_shared.hpp>
#include <cstring>

#define STANDARD_FEE_PER_KB 10000
#define DEFAULT_PAYLOAD_TYPE  TransferAsset
#define TX_LOCKTIME          0x00000000

namespace Elastos {
	namespace ElaWallet {

		Transaction::Transaction() :
				_version(TxVersion::Default),
				_lockTime(TX_LOCKTIME),
				_blockHeight(TX_UNCONFIRMED),
				_payloadVersion(0),
				_fee(0),
				_assetTableID(""),
				_payload(nullptr),
				_type(DEFAULT_PAYLOAD_TYPE),
				_isRegistered(false),
				_txHash(UINT256_ZERO) {
			initPayloadFromType(_type);
		}

		Transaction::Transaction(const Transaction &tx) {
			operator=(tx);
		}

		Transaction &Transaction::operator=(const Transaction &orig) {
			_isRegistered = orig._isRegistered;
			_txHash = orig.getHash();
			_assetTableID = orig._assetTableID;

			_version = orig._version;
			_lockTime = orig._lockTime;
			_blockHeight = orig._blockHeight;
			_timestamp = orig._timestamp;

			_type = orig._type;
			_payloadVersion = orig._payloadVersion;
			_fee = orig._fee;

			initPayloadFromType(orig._type);

			*_payload = *orig._payload;

			_inputs.clear();
			for (size_t i = 0; i < orig._inputs.size(); ++i) {
				_inputs.push_back(orig._inputs[i]);
			}

			_outputs.clear();
			for (size_t i = 0; i < orig._outputs.size(); ++i) {
				_outputs.push_back(orig._outputs[i]);
			}

			_attributes.clear();
			for (size_t i = 0; i < orig._attributes.size(); ++i) {
				_attributes.push_back(orig._attributes[i]);
			}

			_programs.clear();
			for (size_t i = 0; i < orig._programs.size(); ++i) {
				_programs.push_back(orig._programs[i]);
			}

			_remark = orig._remark;
			return *this;
		}

		Transaction::~Transaction() {
		}

		bool Transaction::isRegistered() const {
			return _isRegistered;
		}

		bool &Transaction::isRegistered() {
			return _isRegistered;
		}

		void Transaction::resetHash() {
			UInt256Set(&_txHash, UINT256_ZERO);
		}

		const UInt256 &Transaction::getHash() const {
			if (UInt256IsZero(&_txHash)) {
				ByteStream ostream;
				serializeUnsigned(ostream);
				CMBlock buff = ostream.getBuffer();
				BRSHA256_2(&_txHash, buff, buff.GetSize());
			}
			return _txHash;
		}

		const Transaction::TxVersion &Transaction::getVersion() const {
			return _version;
		}

		void Transaction::setVersion(const TxVersion &version) {
			_version = version;
		}

		void Transaction::setTransactionType(Type t, const PayloadPtr &payload) {
			if (_type != t) {
				_type = t;
				if (payload != nullptr) {
					_payload = payload;
				} else {
					initPayloadFromType(_type);
				}
			}
		}

		Transaction::Type Transaction::getTransactionType() const {
			return _type;
		}

		void Transaction::reinit() {
			Cleanup();
			_type = DEFAULT_PAYLOAD_TYPE;
			initPayloadFromType(_type);

			_version = TxVersion::Default;
			_lockTime = TX_LOCKTIME;
			_blockHeight = TX_UNCONFIRMED;
			_payloadVersion = 0;
			_fee = 0;
		}

		const std::vector<TransactionOutput> &Transaction::getOutputs() const {
			return _outputs;
		}

		std::vector<TransactionOutput> &Transaction::getOutputs() {
			return _outputs;
		}

		const std::vector<TransactionInput> &Transaction::getInputs() const {
			return _inputs;
		}

		std::vector<TransactionInput>& Transaction::getInputs() {
			return _inputs;
		}

		std::vector<std::string> Transaction::getOutputAddresses() {

			const std::vector<TransactionOutput> &_outputs = getOutputs();
			ssize_t len = _outputs.size();
			std::vector<std::string> addresses(len);
			for (int i = 0; i < len; i++)
				addresses[i] = _outputs[i].getAddress();

			return addresses;
		}

		uint32_t Transaction::getLockTime() const {

			return _lockTime;
		}

		void Transaction::setLockTime(uint32_t t) {

			_lockTime = t;
		}

		uint32_t Transaction::getBlockHeight() const {
			return _blockHeight;
		}

		void Transaction::setBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

		uint32_t Transaction::getTimestamp() const {
			return _timestamp;
		}

		void Transaction::setTimestamp(uint32_t t) {
			_timestamp = t;
		}

		void Transaction::addOutput(const TransactionOutput &o) {
			_outputs.push_back(o);
		}

		void Transaction::removeChangeOutput() {
			if (_outputs.size() > 1) {
				_outputs.erase(_outputs.begin() + 1);
			}
		}

		void Transaction::addInput(const TransactionInput &input) {
			_inputs.push_back(input);
		}

		size_t Transaction::getSize() {
			size_t size;
			size = 8 + BRVarIntSize(_inputs.size()) + BRVarIntSize(_outputs.size());

			for (size_t i = 0; _inputs.size(); i++) {
				size += _inputs[i].getSize();
			}

			for (size_t i = 0; _outputs.size(); i++) {
				size += _outputs[i].GetSize();
			}

			return size;
		}

		bool Transaction::isSigned() const {
			if (_type == Type::TransferAsset || _type == Type::TransferCrossChainAsset) {
				if (_programs.size() <= 0) {
					return false;
				}
				for (size_t i = 0; i < _programs.size(); ++i) {
					if (!_programs[i].isValid(this)) {
						return false;
					}
				}
			} else if (_type == Type::RechargeToSideChain) {
				return true;
			} else if (_type == Type::CoinBase) {
				return true;
			}

			return true;
		}

		bool Transaction::sign(const WrapperList<Key, BRKey> &keys, const boost::shared_ptr<TransactionHub> &wallet) {
			return transactionSign(keys, wallet);
		}

		bool Transaction::transactionSign(const WrapperList<Key, BRKey> keys, const boost::shared_ptr<TransactionHub> &wallet) {
			size_t i, j, keysCount = keys.size();
			Address addrs[keysCount], address;

			ParamChecker::checkCondition(keysCount <= 0, Error::Transaction,
										 "Transaction sign key not found");
			SPVLOG_DEBUG("tx sign with {} keys", keysCount);

			for (i = 0; i < keysCount; i++) {
				addrs[i] = Address::None;
				std::string tempAddr = keys[i].address();
				if (!tempAddr.empty()) {
					addrs[i] = tempAddr;
				}
			}

			for (i = 0; i < _inputs.size(); i++) {
				const TransactionPtr &tx = wallet->transactionForHash(_inputs[i].getTransctionHash());
				address = tx->getOutputs()[_inputs[i].getIndex()].getAddress();

				for (j = 0; j < keysCount && !addrs[j].IsEqual(address); j++);
				if (j >= keysCount) continue;
				int signType = address.getSignType();
				std::string redeemScript = keys[j].keyToRedeemScript(signType);
				CMBlock code = Utils::decodeHex(redeemScript);
				if (_type == Type::RegisterIdentification ||
					i >= _programs.size()) {

					Program newProgram;
					newProgram.setCode(code);
					_programs.push_back(newProgram);
				}

				CMBlock signData = keys[j].compactSign(GetShaData());
				_programs[i].setParameter(signData);
			}

			return isSigned();
		}

		UInt256 Transaction::getReverseHash() {

			return UInt256Reverse(&_txHash);
		}

		uint64_t Transaction::getMinOutputAmount() {

			return TX_MIN_OUTPUT_AMOUNT;
		}

		const IPayload *Transaction::getPayload() const {
			return _payload.get();
		}

		IPayload *Transaction::getPayload() {
			return _payload.get();
		}

		void Transaction::SetPayload(const PayloadPtr &payload) {
			_payload = payload;
		}

		void Transaction::addAttribute(const Attribute &attribute) {
			_attributes.push_back(attribute);
		}

		const std::vector<Attribute> &Transaction::getAttributes() const {
			return _attributes;
		}

		void Transaction::addProgram(const Program &program) {
			_programs.push_back(program);
		}

		const std::vector<Program> &Transaction::getPrograms() const {
			return _programs;
		}

		std::vector<Program> &Transaction::getPrograms() {
			return _programs;
		}

		void Transaction::clearPrograms() {
			_programs.clear();
		}

		void Transaction::removeDuplicatePrograms() {
			std::set<std::string> programSet;

			for (std::vector<Program>::iterator iter = _programs.begin(); iter != _programs.end();) {
				std::string key = Utils::encodeHex(iter->getCode());
				if (programSet.find(key) == programSet.end()) {
					programSet.insert(key);
					++iter;
				} else {
					iter = _programs.erase(iter);
				}
			}
		}

		const std::string Transaction::getRemark() const {
			return _remark;
		}

		void Transaction::setRemark(const std::string &remark) {
			_remark = remark;
		}

		void Transaction::Serialize(ByteStream &ostream) const {
			serializeUnsigned(ostream);

			ostream.writeVarUint(_programs.size());
			for (size_t i = 0; i < _programs.size(); i++) {
				_programs[i].Serialize(ostream);
			}
		}

		void Transaction::serializeUnsigned(ByteStream &ostream) const {
			if (_version >= TxVersion::V09) {
				ostream.writeByte(_version);
			}
			ostream.writeByte(_type);

			ostream.writeByte(_payloadVersion);

			ParamChecker::checkCondition(_payload == nullptr, Error::Transaction,
										 "payload should not be null");

			_payload->Serialize(ostream, _payloadVersion);

			ostream.writeVarUint(_attributes.size());
			for (size_t i = 0; i < _attributes.size(); i++) {
				_attributes[i].Serialize(ostream);
			}

			ostream.writeVarUint(_inputs.size());
			for (size_t i = 0; i < _inputs.size(); i++) {
				_inputs[i].Serialize(ostream);
			}

			ostream.writeVarUint(_outputs.size());
			for (size_t i = 0; i < _outputs.size(); i++) {
				_outputs[i].Serialize(ostream, _version);
			}

			ostream.writeUint32(_lockTime);
		}

		bool Transaction::Deserialize(ByteStream &istream) {
			reinit();

			uint8_t flagByte = 0;
			if (!istream.readByte(flagByte)) {
				Log::error("deserialize flag byte error");
				return false;
			}

			if (flagByte >= TxVersion::V09) {
				_version = static_cast<TxVersion>(flagByte);
				uint8_t txType = 0;
				if (!istream.readByte(txType)) {
					Log::error("deserialize type error");
					return false;
				}
				_type = static_cast<Type>(txType);
			} else {
				_version = TxVersion::Default;
				_type = static_cast<Type>(flagByte);
			}

			if (!istream.readByte(_payloadVersion))
				return false;

			initPayloadFromType(_type);

			if (_payload == nullptr) {
				Log::error("new _payload with _type={} when deserialize error", _type);
				return false;
			}
			if (!_payload->Deserialize(istream, _payloadVersion))
				return false;

			uint64_t attributeLength = 0;
			if (!istream.readVarUint(attributeLength))
				return false;

			for (size_t i = 0; i < attributeLength; i++) {
				Attribute attribute;
				if (!attribute.Deserialize(istream)) {
					Log::error("deserialize tx attribute[{}] error", i);
					return false;
				}
				_attributes.push_back(attribute);
			}

			uint64_t inCount = 0;
			if (!istream.readVarUint(inCount)) {
				Log::error("deserialize tx inCount error");
				return false;
			}

			for (size_t i = 0; i < inCount; i++) {
				TransactionInput input;
				if (!input.Deserialize(istream)) {
					Log::error("deserialize tx input [{}] error", i);
					return false;
				}
				_inputs.push_back(input);
			}

			uint64_t outputLength = 0;
			if (!istream.readVarUint(outputLength)) {
				Log::error("deserialize tx output len error");
				return false;
			}

			for (size_t i = 0; i < outputLength; i++) {
				TransactionOutput output;
				if (!output.Deserialize(istream, _version)) {
					Log::error("deserialize tx output[{}] error", i);
					return false;
				}
				_outputs.push_back(output);
			}

			if (!istream.readUint32(_lockTime)) {
				Log::error("deserialize tx lock time error");
				return false;
			}

			uint64_t programLength = 0;
			if (!istream.readVarUint(programLength)) {
				Log::error("deserialize tx program length error");
				return false;
			}

			for (size_t i = 0; i < programLength; i++) {
				Program program;
				if (!program.Deserialize(istream)) {
					Log::error("deserialize program[{}] error", i);
					return false;
				}
				_programs.push_back(program);
			}

			ByteStream ostream;
			serializeUnsigned(ostream);
			CMBlock buff = ostream.getBuffer();
			BRSHA256_2(&_txHash, buff, buff.GetSize());

			return true;
		}

		nlohmann::json Transaction::toJson() const {
			nlohmann::json jsonData;

			jsonData["IsRegistered"] = _isRegistered;

			jsonData["TxHash"] = Utils::UInt256ToString(getHash(), true);
			jsonData["Version"] = _version;
			jsonData["LockTime"] = _lockTime;
			jsonData["BlockHeight"] = _blockHeight;
			jsonData["Timestamp"] = _timestamp;

			std::vector<nlohmann::json> inputsJson(_inputs.size());
			for (size_t i = 0; i < _inputs.size(); ++i) {
				inputsJson[i] = _inputs[i].toJson();
			}
			jsonData["Inputs"] = inputsJson;

			jsonData["Type"] = (uint8_t) _type;
			jsonData["PayloadVersion"] = _payloadVersion;
			jsonData["PayLoad"] = _payload->toJson();

			std::vector<nlohmann::json> attributesJson(_attributes.size());
			for (size_t i = 0; i < _attributes.size(); ++i) {
				attributesJson[i] = _attributes[i].toJson();
			}
			jsonData["Attributes"] = attributesJson;

			std::vector<nlohmann::json> programsJson(_programs.size());
			for (size_t i = 0; i < _programs.size(); ++i) {
				programsJson[i] = _programs[i].toJson();
			}
			jsonData["Programs"] = programsJson;

			std::vector<nlohmann::json> outputsJson(_outputs.size());
			for (size_t i = 0; i < _outputs.size(); ++i) {
				outputsJson[i] = _outputs[i].toJson(_version);
			}
			jsonData["Outputs"] = outputsJson;

			jsonData["Fee"] = _fee;

			jsonData["Remark"] = _remark;

			return jsonData;
		}

		void Transaction::fromJson(const nlohmann::json &jsonData) {
			reinit();

			try {
				_isRegistered = jsonData["IsRegistered"];

				_txHash = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>(), true);
				uint8_t version = jsonData["Version"].get<uint8_t>();
				_version = static_cast<TxVersion>(version);
				_lockTime = jsonData["LockTime"].get<uint32_t>();
				_blockHeight = jsonData["BlockHeight"].get<uint32_t>();
				_timestamp = jsonData["Timestamp"].get<uint32_t>();

				std::vector<nlohmann::json> inputJsons = jsonData["Inputs"];
				for (size_t i = 0; i < inputJsons.size(); ++i) {
					TransactionInput input;
					input.fromJson(inputJsons[i]);
					_inputs.push_back(input);
				}

				_type = Type(jsonData["Type"].get<uint8_t>());
				_payloadVersion = jsonData["PayloadVersion"];
				initPayloadFromType(_type);

				if (_payload == nullptr) {
					Log::error("_payload is nullptr when convert from json");
				} else {
					_payload->fromJson(jsonData["PayLoad"]);
				}

				std::vector<nlohmann::json> attributesJson = jsonData["Attributes"];
				for (size_t i = 0; i < attributesJson.size(); ++i) {
					Attribute attribute;
					attribute.fromJson(attributesJson[i]);
					_attributes.push_back(attribute);
				}

				std::vector<nlohmann::json> programsJson = jsonData["Programs"];
				for (size_t i = 0; i < programsJson.size(); ++i) {
					Program program;
					program.fromJson(programsJson[i]);
					_programs.push_back(program);
				}

				std::vector<nlohmann::json> outputsJson = jsonData["Outputs"];
				for (size_t i = 0; i < outputsJson.size(); ++i) {
					TransactionOutput output;
					output.fromJson(outputsJson[i], _version);
					_outputs.push_back(output);
				}

				_fee = jsonData["Fee"].get<uint64_t>();

				_remark = jsonData["Remark"].get<std::string>();
			} catch (const std::bad_cast &e) {
				ParamChecker::throwLogicException(Error::Code::JsonFormatError, "tx from json: " +
																				std::string(e.what()));
			}
		}

		uint64_t Transaction::calculateFee(uint64_t feePerKb) {
			return ((getSize() + 999) / 1000) * feePerKb;
		}

		uint64_t Transaction::getTxFee(const boost::shared_ptr<TransactionHub> &wallet) {
			uint64_t fee = 0, inputAmount = 0, outputAmount = 0;

			for (size_t i = 0; i < _inputs.size(); ++i) {
				const TransactionPtr &tx = wallet->transactionForHash(_inputs[i].getTransctionHash());
				if (tx == nullptr) continue;
				inputAmount += tx->getOutputs()[_inputs[i].getIndex()].getAmount();
			}

			for (size_t i = 0; i < _outputs.size(); ++i) {
				outputAmount += _outputs[i].getAmount();
			}

			if (inputAmount >= outputAmount)
				fee = inputAmount - outputAmount;

			return fee;
		}

		nlohmann::json Transaction::GetSummary(const WalletPtr &wallet, uint32_t confirms, bool detail) {
			std::string remark = wallet->GetRemark(Utils::UInt256ToString(getHash(), true));
			setRemark(remark);

			std::string addr;
			nlohmann::json summary;
			std::string direction = "Received";
			uint64_t inputAmount = 0, outputAmount = 0, changeAmount = 0, fee = 0;

			std::map<std::string, uint64_t> inputList;
			for (size_t i = 0; i < _inputs.size(); i++) {
				TransactionPtr tx = wallet->transactionForHash(_inputs[i].getTransctionHash());
				if (tx) {
					addr = tx->getOutputs()[_inputs[i].getIndex()].getAddress();
					if (wallet->containsAddress(addr)) {
						// sent or moved
						uint64_t spentAmount = tx->getOutputs()[_inputs[i].getIndex()].getAmount();
						direction = "Sent";
						inputAmount += spentAmount;

						if (detail) {
							if (inputList.find(addr) == inputList.end())
								inputList[addr] = spentAmount;
							else
								inputList[addr] += spentAmount;
						}
					}
				}
			}

			std::map<std::string, uint64_t> outputList;
			for (size_t i = 0; i < _outputs.size(); ++i) {
				addr = _outputs[i].getAddress();
				uint64_t oAmount = _outputs[i].getAmount();
				bool containAddress = wallet->containsAddress(addr);
				if (containAddress) {
					changeAmount += oAmount;
				} else {
					outputAmount += oAmount;
				}
				if (detail && ((direction == "Received" && containAddress) || direction != "Received")) {
					if (outputList.find(addr) == outputList.end())
						outputList[addr] = oAmount;
					else
						outputList[addr] += oAmount;
				}
			}

			if (direction == "Sent" && outputAmount == 0) {
				direction = "Moved";
			}

			fee = inputAmount > (outputAmount + changeAmount) ? inputAmount - outputAmount - changeAmount : 0;
			uint64_t amount = 0;
			if (direction == "Received") {
				amount = changeAmount;
			} else if (direction == "Sent") {
				amount = outputAmount;
			} else {
				amount = 0;
			}

			summary["TxHash"] = Utils::UInt256ToString(getHash(), true);
			summary["Status"] = confirms <= 6 ? "Pending" : "Confirmed";
			summary["ConfirmStatus"] = confirms <= 6 ? std::to_string(confirms) : "6+";
			summary["Timestamp"] = getTimestamp();
			summary["Direction"] = direction;
			summary["Amount"] = amount;
			if (detail) {
				summary["Fee"] = fee;
				summary["Remark"] = getRemark();
				summary["Type"] = getTransactionType();
				summary["Inputs"] = inputList;
				summary["Outputs"] = outputList;
			}

			return summary;
		}

		CMBlock Transaction::GetShaData() const {
			ByteStream ostream;
			serializeUnsigned(ostream);
			CMBlock data = ostream.getBuffer();
			CMBlock shaData(sizeof(UInt256));
			BRSHA256(shaData, data, data.GetSize());
			return shaData;
		}

		void Transaction::initPayloadFromType(Type type) {
			if (type == CoinBase) {
				_payload = PayloadPtr(new PayloadCoinBase());
			} else if (type == RegisterAsset) {
				_payload = PayloadPtr(new PayloadRegisterAsset());
			} else if (type == TransferAsset) {
				_payload = PayloadPtr(new PayloadTransferAsset());
			} else if (type == Record) {
				_payload = PayloadPtr(new PayloadRecord());
			} else if (type == Deploy) {
				//todo add deploy _payload
				//_payload = boost::shared_ptr<PayloadDeploy>(new PayloadDeploy());
			} else if (type == SideChainPow) {
				_payload = PayloadPtr(new PayloadSideMining());
			} else if (type == RechargeToSideChain) {
				_payload = PayloadPtr(new PayloadRechargeToSideChain());
			} else if (type == WithdrawFromSideChain) {
				_payload = PayloadPtr(new PayloadWithDrawAsset());
			} else if (type == TransferCrossChainAsset) {
				_payload = PayloadPtr(new PayloadTransferCrossChainAsset());
			} else if (type == RegisterIdentification) {
				_payload = PayloadPtr(new PayloadRegisterIdentification());
			}
		}

		void Transaction::Cleanup() {
			_inputs.clear();
			_outputs.clear();
			_attributes.clear();
			_programs.clear();
			_payload.reset();
		}

		uint8_t Transaction::getPayloadVersion() const {
			return _payloadVersion;
		}

		void Transaction::setPayloadVersion(uint8_t version) {
			_payloadVersion = version;
		}

		uint64_t Transaction::getFee() const {
			return _fee;
		}

		void Transaction::setFee(uint64_t f) {
			_fee = f;
		}

		bool Transaction::IsEqual(const Transaction *tx) const {
			return (tx == this || UInt256Eq(&_txHash, &tx->getHash()));
		}

		UInt256 Transaction::GetAssetID() const {
			UInt256 result = UINT256_ZERO;
			if (!_outputs.empty())
				result = _outputs.begin()->getAssetId();
			return result;
		}

		const std::string &Transaction::GetAssetTableID() const {
			return _assetTableID;
		}

		void Transaction::SetAssetTableID(const std::string &assetTableID) {
			_assetTableID = assetTableID;
		}

	}
}
