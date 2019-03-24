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
#include "Payload/PayloadRegisterProducer.h"
#include "Payload/PayloadCancelProducer.h"
#include "Payload/PayloadUpdateProducer.h"
#include "Payload/PayloadReturnDepositCoin.h"

#include <SDK/Common/Utils.h>
#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>

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
				_txHash(0) {
			InitPayloadFromType(_type);
		}

		Transaction::Transaction(const Transaction &tx) {
			operator=(tx);
		}

		Transaction &Transaction::operator=(const Transaction &orig) {
			_isRegistered = orig._isRegistered;
			_txHash = orig.GetHash();
			_assetTableID = orig._assetTableID;

			_version = orig._version;
			_lockTime = orig._lockTime;
			_blockHeight = orig._blockHeight;
			_timestamp = orig._timestamp;

			_type = orig._type;
			_payloadVersion = orig._payloadVersion;
			_fee = orig._fee;

			InitPayloadFromType(orig._type);

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

		bool Transaction::IsRegistered() const {
			return _isRegistered;
		}

		bool &Transaction::IsRegistered() {
			return _isRegistered;
		}

		void Transaction::ResetHash() {
			_txHash = 0;
		}

		const uint256 &Transaction::GetHash() const {
			if (_txHash == 0) {
				ByteStream stream;
				SerializeUnsigned(stream);
				_txHash = sha256_2(stream.GetBytes());
			}
			return _txHash;
		}

		const Transaction::TxVersion &Transaction::GetVersion() const {
			return _version;
		}

		void Transaction::SetVersion(const TxVersion &version) {
			_version = version;
		}

		void Transaction::SetTransactionType(Type t, const PayloadPtr &payload) {
			if (_type != t) {
				_type = t;
				if (payload != nullptr) {
					_payload = payload;
				} else {
					InitPayloadFromType(_type);
				}
			}
		}

		Transaction::Type Transaction::GetTransactionType() const {
			return _type;
		}

		void Transaction::Reinit() {
			Cleanup();
			_type = DEFAULT_PAYLOAD_TYPE;
			InitPayloadFromType(_type);

			_version = TxVersion::Default;
			_lockTime = TX_LOCKTIME;
			_blockHeight = TX_UNCONFIRMED;
			_payloadVersion = 0;
			_fee = 0;
		}

		const std::vector<TransactionOutput> &Transaction::GetOutputs() const {
			return _outputs;
		}

		std::vector<TransactionOutput> &Transaction::GetOutputs() {
			return _outputs;
		}

		void Transaction::SetOutputs(const std::vector<TransactionOutput> &outputs) {
			_outputs = outputs;
		}

		void Transaction::AddOutput(const TransactionOutput &output) {
			_outputs.push_back(output);
		}

		const std::vector<TransactionInput> &Transaction::GetInputs() const {
			return _inputs;
		}

		std::vector<TransactionInput>& Transaction::GetInputs() {
			return _inputs;
		}

		void Transaction::AddInput(const TransactionInput &Input) {
			_inputs.push_back(Input);
		}

		bool Transaction::ContainInput(const uint256 &hash, uint32_t n) const {
			for (size_t i = 0; i < _inputs.size(); ++i) {
				if (_inputs[i].GetTransctionHash() == hash && n == _inputs[i].GetIndex()) {
					return true;
				}
			}

			return false;
		}

		uint32_t Transaction::GetLockTime() const {

			return _lockTime;
		}

		void Transaction::SetLockTime(uint32_t t) {

			_lockTime = t;
		}

		uint32_t Transaction::GetBlockHeight() const {
			return _blockHeight;
		}

		void Transaction::SetBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

		uint32_t Transaction::GetTimestamp() const {
			return _timestamp;
		}

		void Transaction::SetTimestamp(uint32_t t) {
			_timestamp = t;
		}

		void Transaction::RemoveChangeOutput() {
			if (_outputs.size() > 1) {
				_outputs.erase(_outputs.begin() + _outputs.size() - 1);
			}
		}

		size_t Transaction::GetSize() {
			ByteStream ostream;
			Serialize(ostream);
			return ostream.GetBytes().size();
		}

		nlohmann::json Transaction::GetSignedInfo() const {
			nlohmann::json info;
			uint256 md = GetShaData();

			for (size_t i = 0; i < _programs.size(); ++i) {
				info.push_back(_programs[i].GetSignedInfo(md));
			}
			return info;
		}

		bool Transaction::IsSigned() const {
			if (_type == Type::RechargeToSideChain || _type == Type::CoinBase)
				return true;

			if (_programs.size() == 0)
				return false;

			uint256 md = GetShaData();

			for (size_t i = 0; i < _programs.size(); ++i) {
				if (!_programs[i].VerifySignature(md))
					return false;
			}

			return true;
		}

		bool Transaction::IsValid() const {
			if (!IsSigned()) {
				Log::error("verify tx signature fail");
				return false;
			}

			for (size_t i = 0; i < _attributes.size(); ++i) {
				if (!_attributes[i].IsValid()) {
					Log::error("tx attribute is invalid");
					return false;
				}
			}

			if (_payload == nullptr || !_payload->IsValid()) {
				Log::error("tx payload invalid");
				return false;
			}

			if (_outputs.size() == 0) {
				Log::error("tx without output");
				return false;
			}

			for (size_t i = 0; i < _outputs.size(); ++i) {
				if (!_outputs[i].IsValid()) {
					Log::error("tx output is invalid");
					return false;
				}
			}

			return true;
		}

		uint64_t Transaction::GetMinOutputAmount() {

			return TX_MIN_OUTPUT_AMOUNT;
		}

		const IPayload *Transaction::GetPayload() const {
			return _payload.get();
		}

		IPayload *Transaction::GetPayload() {
			return _payload.get();
		}

		void Transaction::SetPayload(const PayloadPtr &payload) {
			_payload = payload;
		}

		void Transaction::AddAttribute(const Attribute &attribute) {
			_attributes.push_back(attribute);
		}

		const std::vector<Attribute> &Transaction::GetAttributes() const {
			return _attributes;
		}

		void Transaction::AddProgram(const Program &program) {
			_programs.push_back(program);
		}

		const std::vector<Program> &Transaction::GetPrograms() const {
			return _programs;
		}

		std::vector<Program> &Transaction::GetPrograms() {
			return _programs;
		}

		void Transaction::ClearPrograms() {
			_programs.clear();
		}

		const std::string Transaction::GetRemark() const {
			return _remark;
		}

		void Transaction::SetRemark(const std::string &remark) {
			_remark = remark;
		}

		void Transaction::Serialize(ByteStream &ostream) const {
			SerializeUnsigned(ostream);

			ostream.WriteVarUint(_programs.size());
			for (size_t i = 0; i < _programs.size(); i++) {
				_programs[i].Serialize(ostream);
			}
		}

		void Transaction::SerializeUnsigned(ByteStream &ostream) const {
			if (_version >= TxVersion::V09) {
				ostream.WriteByte(_version);
			}
			ostream.WriteByte(_type);

			ostream.WriteByte(_payloadVersion);

			ErrorChecker::CheckCondition(_payload == nullptr, Error::Transaction,
										 "payload should not be null");

			_payload->Serialize(ostream, _payloadVersion);

			ostream.WriteVarUint(_attributes.size());
			for (size_t i = 0; i < _attributes.size(); i++) {
				_attributes[i].Serialize(ostream);
			}

			ostream.WriteVarUint(_inputs.size());
			for (size_t i = 0; i < _inputs.size(); i++) {
				_inputs[i].Serialize(ostream);
			}

			ostream.WriteVarUint(_outputs.size());
			for (size_t i = 0; i < _outputs.size(); i++) {
				_outputs[i].Serialize(ostream, _version);
			}

			ostream.WriteUint32(_lockTime);
		}

		bool Transaction::Deserialize(const ByteStream &istream) {
			Reinit();

			uint8_t flagByte = 0;
			if (!istream.ReadByte(flagByte)) {
				Log::error("deserialize flag byte error");
				return false;
			}

			if (flagByte >= TxVersion::V09) {
				_version = static_cast<TxVersion>(flagByte);
				uint8_t txType = 0;
				if (!istream.ReadByte(txType)) {
					Log::error("deserialize type error");
					return false;
				}
				_type = static_cast<Type>(txType);
			} else {
				_version = TxVersion::Default;
				_type = static_cast<Type>(flagByte);
			}

			if (!istream.ReadByte(_payloadVersion))
				return false;

			InitPayloadFromType(_type);

			if (_payload == nullptr) {
				Log::error("new _payload with _type={} when deserialize error", _type);
				return false;
			}
			if (!_payload->Deserialize(istream, _payloadVersion))
				return false;

			uint64_t attributeLength = 0;
			if (!istream.ReadVarUint(attributeLength))
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
			if (!istream.ReadVarUint(inCount)) {
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
			if (!istream.ReadVarUint(outputLength)) {
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

			if (!istream.ReadUint32(_lockTime)) {
				Log::error("deserialize tx lock time error");
				return false;
			}

			uint64_t programLength = 0;
			if (!istream.ReadVarUint(programLength)) {
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

			ByteStream stream;
			SerializeUnsigned(stream);
			_txHash = sha256_2((stream.GetBytes()));

			return true;
		}

		nlohmann::json Transaction::ToJson() const {
			nlohmann::json jsonData;

			jsonData["IsRegistered"] = _isRegistered;

			jsonData["TxHash"] = GetHash().GetHex();
			jsonData["Version"] = _version;
			jsonData["LockTime"] = _lockTime;
			jsonData["BlockHeight"] = _blockHeight;
			jsonData["Timestamp"] = _timestamp;

			std::vector<nlohmann::json> inputsJson(_inputs.size());
			for (size_t i = 0; i < _inputs.size(); ++i) {
				inputsJson[i] = _inputs[i].ToJson();
			}
			jsonData["Inputs"] = inputsJson;

			jsonData["Type"] = (uint8_t) _type;
			jsonData["PayloadVersion"] = _payloadVersion;
			jsonData["PayLoad"] = _payload->ToJson(_payloadVersion);

			std::vector<nlohmann::json> attributesJson(_attributes.size());
			for (size_t i = 0; i < _attributes.size(); ++i) {
				attributesJson[i] = _attributes[i].ToJson();
			}
			jsonData["Attributes"] = attributesJson;

			std::vector<nlohmann::json> programsJson(_programs.size());
			for (size_t i = 0; i < _programs.size(); ++i) {
				programsJson[i] = _programs[i].ToJson();
			}
			jsonData["Programs"] = programsJson;

			std::vector<nlohmann::json> outputsJson(_outputs.size());
			for (size_t i = 0; i < _outputs.size(); ++i) {
				outputsJson[i] = _outputs[i].ToJson(_version);
			}
			jsonData["Outputs"] = outputsJson;

			jsonData["Fee"] = _fee;

			jsonData["Remark"] = _remark;

			return jsonData;
		}

		void Transaction::FromJson(const nlohmann::json &jsonData) {
			Reinit();

			try {
				_isRegistered = jsonData["IsRegistered"];

				_txHash.SetHex(jsonData["TxHash"].get<std::string>());
				uint8_t version = jsonData["Version"].get<uint8_t>();
				_version = static_cast<TxVersion>(version);
				_lockTime = jsonData["LockTime"].get<uint32_t>();
				_blockHeight = jsonData["BlockHeight"].get<uint32_t>();
				_timestamp = jsonData["Timestamp"].get<uint32_t>();

				std::vector<nlohmann::json> inputJsons = jsonData["Inputs"];
				for (size_t i = 0; i < inputJsons.size(); ++i) {
					TransactionInput input;
					input.FromJson(inputJsons[i]);
					_inputs.push_back(input);
				}

				_type = Type(jsonData["Type"].get<uint8_t>());
				_payloadVersion = jsonData["PayloadVersion"];
				InitPayloadFromType(_type);

				if (_payload == nullptr) {
					Log::error("_payload is nullptr when convert from json");
				} else {
					_payload->FromJson(jsonData["PayLoad"], _payloadVersion);
				}

				std::vector<nlohmann::json> attributesJson = jsonData["Attributes"];
				for (size_t i = 0; i < attributesJson.size(); ++i) {
					Attribute attribute;
					attribute.FromJson(attributesJson[i]);
					_attributes.push_back(attribute);
				}

				std::vector<nlohmann::json> programsJson = jsonData["Programs"];
				for (size_t i = 0; i < programsJson.size(); ++i) {
					Program program;
					program.FromJson(programsJson[i]);
					_programs.push_back(program);
				}

				std::vector<nlohmann::json> outputsJson = jsonData["Outputs"];
				for (size_t i = 0; i < outputsJson.size(); ++i) {
					TransactionOutput output;
					output.FromJson(outputsJson[i], _version);
					_outputs.push_back(output);
				}

				_fee = jsonData["Fee"].get<uint64_t>();

				_remark = jsonData["Remark"].get<std::string>();
			} catch (const std::bad_cast &e) {
				ErrorChecker::ThrowLogicException(Error::Code::JsonFormatError, "tx from json: " +
																				std::string(e.what()));
			}
		}

		uint64_t Transaction::CalculateFee(uint64_t feePerKb) {
			return ((GetSize() + 999) / 1000) * feePerKb;
		}

		uint64_t Transaction::GetTxFee(const boost::shared_ptr<TransactionHub> &wallet) {
			uint64_t fee = 0, inputAmount = 0, outputAmount = 0;

			for (size_t i = 0; i < _inputs.size(); ++i) {
				const TransactionPtr &tx = wallet->TransactionForHash(_inputs[i].GetTransctionHash());
				if (tx == nullptr) continue;
				inputAmount += tx->GetOutputs()[_inputs[i].GetIndex()].GetAmount();
			}

			for (size_t i = 0; i < _outputs.size(); ++i) {
				outputAmount += _outputs[i].GetAmount();
			}

			if (inputAmount >= outputAmount)
				fee = inputAmount - outputAmount;

			return fee;
		}

		nlohmann::json Transaction::GetSummary(const WalletPtr &wallet, uint32_t confirms, bool detail) {
			std::string remark = wallet->GetRemark(GetHash().GetHex());
			SetRemark(remark);

			std::string addr;
			nlohmann::json summary, outputPayload;
			std::vector<nlohmann::json> outputPayloads;
			std::string direction = "Received";
			uint64_t inputAmount = 0, outputAmount = 0, changeAmount = 0, fee = 0;

			std::map<std::string, uint64_t> inputList;
			for (size_t i = 0; i < _inputs.size(); i++) {
				TransactionPtr tx = wallet->TransactionForHash(_inputs[i].GetTransctionHash());
				if (tx) {
					uint64_t spentAmount = tx->GetOutputs()[_inputs[i].GetIndex()].GetAmount();
					addr = tx->GetOutputs()[_inputs[i].GetIndex()].GetAddress().String();

					if (detail) {
						if (inputList.find(addr) == inputList.end()) {
							inputList[addr] = spentAmount;
						} else {
							inputList[addr] += spentAmount;
						}
					}

					if (wallet->ContainsAddress(addr) && !wallet->IsVoteDepositAddress(addr)) {
						// sent or moved
						direction = "Sent";
						inputAmount += spentAmount;
					}
				}
			}

			std::map<std::string, uint64_t> outputList;
			for (size_t i = 0; i < _outputs.size(); ++i) {
				uint64_t oAmount = _outputs[i].GetAmount();
				addr = _outputs[i].GetAddress().String();

				if (_outputs[i].GetType() == TransactionOutput::VoteOutput) {
					outputPayload = _outputs[i].GetPayload()->ToJson();
					outputPayload["Amount"] = oAmount;
					outputPayloads.push_back(outputPayload);
				}

				if (wallet->IsVoteDepositAddress(addr)) {
					direction = "Deposit";
				}

				if (wallet->ContainsAddress(addr) && !wallet->IsVoteDepositAddress(addr)) {
					changeAmount += oAmount;
				} else {
					outputAmount += oAmount;
				}

				if (detail) {
					if (outputList.find(addr) == outputList.end()) {
						outputList[addr] = oAmount;
					} else {
						outputList[addr] += oAmount;
					}
				}
			}

			if (direction != "Deposit" && direction == "Sent" && outputAmount == 0) {
				direction = "Moved";
			}

			fee = inputAmount > (outputAmount + changeAmount) ? inputAmount - outputAmount - changeAmount : 0;
			uint64_t amount = 0;
			if (direction == "Received") {
				amount = changeAmount;
			} else if (direction == "Sent") {
				amount = outputAmount;
			} else if (direction == "Deposit") {
				if (inputAmount > 0) {
					amount = outputAmount;
				} else {
					amount = 0;
				}
			} else {
				amount = 0;
			}

			summary["TxHash"] = GetHash().GetHex();
			summary["Status"] = confirms <= 6 ? "Pending" : "Confirmed";
			summary["ConfirmStatus"] = confirms <= 6 ? std::to_string(confirms) : "6+";
			summary["Timestamp"] = GetTimestamp();
			summary["Direction"] = direction;
			summary["Amount"] = amount;
			summary["Type"] = GetTransactionType();
			summary["Height"] = GetBlockHeight();
			if (detail) {
				summary["Fee"] = fee;
				summary["Remark"] = GetRemark();
				summary["Inputs"] = inputList;
				summary["Outputs"] = outputList;
				summary["Payload"] = _payload->ToJson(_payloadVersion);
				summary["OutputPayload"] = outputPayloads;

				std::vector<nlohmann::json> attributes;
				for (int i = 0; i < _attributes.size(); ++i) {
					attributes.push_back(_attributes[i].ToJson());
				}
				summary["Attribute"] = attributes;
			}

			return summary;
		}

		uint256 Transaction::GetShaData() const {
			ByteStream stream;
			SerializeUnsigned(stream);
			return uint256(sha256(stream.GetBytes()));
		}

		void Transaction::InitPayloadFromType(Type type) {
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
			} else if (type == RechargeToSideChain) { // side chain payload
				_payload = PayloadPtr(new PayloadRechargeToSideChain());
			} else if (type == WithdrawFromSideChain) {
				_payload = PayloadPtr(new PayloadWithDrawAsset());
			} else if (type == TransferCrossChainAsset) {
				_payload = PayloadPtr(new PayloadTransferCrossChainAsset());
			} else if (type == RegisterProducer) {
				_payload = PayloadPtr(new PayloadRegisterProducer());
			} else if (type == CancelProducer) {
				_payload = PayloadPtr(new PayloadCancelProducer());
			} else if (type == UpdateProducer) {
				_payload = PayloadPtr(new PayloadUpdateProducer());
			} else if (type == ReturnDepositCoin) {
				_payload = PayloadPtr(new PayloadReturnDepositCoin());
			} else if (type == RegisterIdentification) { // ID chain payload
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

		uint8_t Transaction::GetPayloadVersion() const {
			return _payloadVersion;
		}

		void Transaction::SetPayloadVersion(uint8_t version) {
			_payloadVersion = version;
		}

		uint64_t Transaction::GetFee() const {
			return _fee;
		}

		void Transaction::SetFee(uint64_t f) {
			_fee = f;
		}

		bool Transaction::IsEqual(const Transaction *tx) const {
			return (tx == this || _txHash == tx->GetHash());
		}

		uint32_t Transaction::GetConfirms(uint32_t walletBlockHeight) const {
			if (_blockHeight == TX_UNCONFIRMED)
				return 0;

			return walletBlockHeight >= _blockHeight ? walletBlockHeight - _blockHeight + 1 : 0;
		}

		uint256 Transaction::GetAssetID() const {
			uint256 result;
			if (!_outputs.empty())
				result = _outputs.begin()->GetAssetId();
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
