// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/make_shared.hpp>
#include <cstring>
#include <BRTransaction.h>
#include <SDK/Common/Log.h>
#include <Core/BRTransaction.h>

#include "Transaction.h"
#include "Payload/PayloadCoinBase.h"
#include "Payload/PayloadIssueToken.h"
#include "Payload/PayloadWithDrawAsset.h"
#include "Payload/PayloadRecord.h"
#include "Payload/PayloadRegisterAsset.h"
#include "Payload/PayloadSideMining.h"
#include "Payload/PayloadTransferCrossChainAsset.h"
#include "Payload/PayloadTransferAsset.h"
#include "Payload/PayloadRegisterIdentification.h"
#include "BRCrypto.h"
#include "Utils.h"
#include "BRAddress.h"
#include "Wallet.h"

#define STANDARD_FEE_PER_KB 10000
#define DEFAULT_PAYLOAD_TYPE  TransferAsset
#define TX_VERSION           0x00000001
#define TX_LOCKTIME          0x00000000

namespace Elastos {
	namespace ElaWallet {

		Transaction::Transaction() :
				version(TX_VERSION),
				lockTime(TX_LOCKTIME),
				blockHeight(TX_UNCONFIRMED),
				payloadVersion(0),
				fee(0),
				payload(nullptr),
				type(DEFAULT_PAYLOAD_TYPE),
				_isRegistered(false) {
		}

		Transaction::Transaction(const Transaction &tx) {
			operator=(tx);
		}

		Transaction &Transaction::operator=(const Transaction &orig) {
			_isRegistered = orig._isRegistered;

			type = orig.type;
			payloadVersion = orig.payloadVersion;
			fee = orig.fee;

			initPayloadFromType(orig.type);

			ByteStream stream;
			orig.payload->Serialize(stream);
			stream.setPosition(0);
			payload->Deserialize(stream);

			inputs.clear();
			for (size_t i = 0; i < orig.inputs.size(); ++i) {
				inputs.push_back(orig.inputs[i]);
			}

			outputs.clear();
			for (size_t i = 0; i < orig.outputs.size(); ++i) {
				outputs.push_back(orig.outputs[i]);
			}

			attributes.clear();
			for (size_t i = 0; i < orig.attributes.size(); ++i) {
				attributes.push_back(orig.attributes[i]);
			}

			programs.clear();
			for (size_t i = 0; i < programs.size(); ++i) {
				programs.push_back(orig.programs[i]);
			}

			Remark = orig.Remark;
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
			UInt256Set(&txHash, UINT256_ZERO);
		}

		const UInt256 &Transaction::getHash() const {
			//fixme [refactor] complete me
//			UInt256 emptyHash = UINT256_ZERO;
//			if (UInt256Eq(&txHash, &emptyHash)) {
//				ByteStream ostream;
//				serializeUnsigned(ostream);
//				CMBlock buff = ostream.getBuffer();
//				BRSHA256_2(&txHash, buff, buff.GetSize());
//			}
//			return txHash;
			return UINT256_ZERO;
		}

		uint32_t Transaction::getVersion() const {
			return version;
		}

		void Transaction::setVersion(uint32_t v) {
			version = v;
		}

		void Transaction::setTransactionType(Type t) {
			if (type != t) {
				type = t;
				initPayloadFromType(type);
			}
		}

		Transaction::Type Transaction::getTransactionType() const {
			return type;
		}

		void Transaction::reinit() {
			Cleanup();
			type = DEFAULT_PAYLOAD_TYPE;
			initPayloadFromType(type);

			version = TX_VERSION;
			lockTime = TX_LOCKTIME;
			blockHeight = TX_UNCONFIRMED;
			payloadVersion = 0;
			fee = 0;
		}

		const std::vector<TransactionOutput> &Transaction::getOutputs() const {
			return outputs;
		}

		std::vector<TransactionOutput> &Transaction::getOutputs() {
			return outputs;
		}

		const std::vector<TransactionInput> &Transaction::getInputs() const {
			return inputs;
		}

		std::vector<std::string> Transaction::getOutputAddresses() {

			const std::vector<TransactionOutput> &outputs = getOutputs();
			ssize_t len = outputs.size();
			std::vector<std::string> addresses(len);
			for (int i = 0; i < len; i++)
				addresses[i] = outputs[i].getAddress();

			return addresses;
		}

		uint32_t Transaction::getLockTime() {

			return lockTime;
		}

		void Transaction::setLockTime(uint32_t t) {

			lockTime = t;
		}

		uint32_t Transaction::getBlockHeight() {
			return blockHeight;
		}

		void Transaction::setBlockHeight(uint32_t height) {
			blockHeight = height;
		}

		uint32_t Transaction::getTimestamp() {
			return timestamp;
		}

		void Transaction::setTimestamp(uint32_t t) {
			timestamp = t;
		}

		void Transaction::addOutput(const TransactionOutput &o) {
			outputs.push_back(o);
		}

		void Transaction::addInput(const Elastos::ElaWallet::TransactionInput &input) {
			inputs.push_back(input);
		}

		size_t Transaction::getSize() {
			size_t size;
			size = 8 + BRVarIntSize(inputs.size()) + BRVarIntSize(outputs.size());

			for (size_t i = 0; inputs.size(); i++) {
				size += inputs[i].getSize();
			}

			for (size_t i = 0; outputs.size(); i++) {
				size += outputs[i].GetSize();
			}

			return size;
		}

		bool Transaction::isSigned() const {
			if (type == Type::TransferAsset) {
				if (programs.size() <= 0) {
					return false;
				}
				for (size_t i = 0; i < programs.size(); ++i) {
					if (!programs[i].isValid(this)) {
						return false;
					}
				}
			} else if (type == Type::IssueToken) {
				// TODO verify merkle proof
				return true;
			} else if (type == Type::CoinBase) {
				return true;
			}

			return true;
		}

		bool Transaction::sign(const WrapperList<Key, BRKey> &keys, int forkId) {
			return transactionSign(forkId, keys);
		}

		bool Transaction::sign(const Key &key, int forkId) {

			WrapperList<Key, BRKey> keys(1);
			keys.push_back(key);
			return sign(keys, forkId);
		}

		bool Transaction::transactionSign(int forkId, const WrapperList<Key, BRKey> keys) {
			const int SIGHASH_ALL = 0x01; // default, sign all of the outputs
			size_t i, j, keysCount = keys.size();
			BRAddress addrs[keysCount], address;

			if (keysCount <= 0) {
				throw std::logic_error("transaction sign keysCount is 0.");
			}
			SPDLOG_DEBUG(Log::getLogger(), "Transaction transactionSign method begin, key counts = {}.", keysCount);

			for (i = 0; i < keysCount; i++) {
				addrs[i] = BR_ADDRESS_NONE;
				std::string tempAddr = keys[i].address();
				if (!tempAddr.empty()) {
					strncpy(addrs[i].s, tempAddr.c_str(), sizeof(BRAddress) - 1);
				}
			}
			//fixme [refactor] fix sign later
//
//			SPDLOG_DEBUG(Log::getLogger(), "Transaction transactionSign input sign begin.");
//			for (i = 0; i < inputs.size(); i++) {
//				const TransactionInput &input = inputs[i];
//
//				if (!BRAddressFromScriptPubKey(address.s, sizeof(address), input->script, input->scriptLen)) continue;
//				j = 0;
//				while (j < keysCount && !BRAddressEq(&addrs[j], &address)) j++;
//				if (j >= keysCount) continue;
//				Program *program = nullptr;
//				Address tempAddr(address.s);
//				int signType = tempAddr.getSignType();
//				std::string redeemScript = keys[j].keyToRedeemScript(signType);
//				CMBlock code = Utils::decodeHex(redeemScript);
//				if (type == Type::RegisterIdentification ||
//					i >= programs.size()) {
//
//					Program *newProgram(new Program());
//					newProgram->setCode(code);
//					programs.push_back(newProgram);
//					program = newProgram;
//
//				} else if (i < programs.size()) {
//					program = programs[i];
//				}
//
//				SPDLOG_DEBUG(Log::getLogger(), "Transaction transactionSign begin sign the {} input.", i);
//				const uint8_t *elems[BRScriptElements(NULL, 0, program->getCode(), program->getCode().GetSize())];
//				size_t elemsCount = BRScriptElements(elems, sizeof(elems) / sizeof(*elems), program->getCode(),
//													 program->getCode().GetSize());
//				CMBlock pubKey = keys[j].getPubkey();
//				size_t pkLen = pubKey.GetSize();
//				uint8_t sig[73], script[1 + sizeof(sig) + 1 + pkLen];
//				size_t sigLen, scriptLen;
//				UInt256 md = UINT256_ZERO;
//				ByteStream ostream;
//				serializeUnsigned(ostream);
//				CMBlock data = ostream.getBuffer();
//				if (elemsCount >= 2 && *elems[elemsCount - 2] == OP_EQUALVERIFY) { // pay-to-pubkey-hash
//					SPDLOG_DEBUG(Log::getLogger(), "Transaction transactionSign the {} input pay to pubkey hash.", i);
//
//					BRSHA256_2(&md, data, data.GetSize());
//					sigLen = BRKeySign(keys[j].getRaw(), sig, sizeof(sig) - 1, md);
//					sig[sigLen++] = forkId | SIGHASH_ALL;
//					scriptLen = BRScriptPushData(script, sizeof(script), sig, sigLen);
//					scriptLen += BRScriptPushData(&script[scriptLen], sizeof(script) - scriptLen, pubKey, pkLen);
//					BRTxInputSetSignature(input, script, scriptLen);
//				} else { // pay-to-pubkey
//					SPDLOG_DEBUG(Log::getLogger(), "Transaction transactionSign the {} input pay to pubkey.", i);
//
//					BRSHA256_2(&md, data, data.GetSize());
//					sigLen = BRKeySign(keys[j].getRaw(), sig, sizeof(sig) - 1, md);
//					sig[sigLen++] = forkId | SIGHASH_ALL;
//					scriptLen = BRScriptPushData(script, sizeof(script), sig, sigLen);
//					BRTxInputSetSignature(input, script, scriptLen);
//				}
//
//				CMBlock shaData(sizeof(UInt256));
//				BRSHA256(shaData, data, data.GetSize());
//				CMBlock signData = keys[j].compactSign(shaData);
//				program->setParameter(signData);
//				SPDLOG_DEBUG(Log::getLogger(), "Transaction transactionSign end sign the {} input.", i);
//			}

			return isSigned();
		}

		UInt256 Transaction::getReverseHash() {

			return UInt256Reverse(&txHash);
		}

		uint64_t Transaction::getMinOutputAmount() {

			return TX_MIN_OUTPUT_AMOUNT;
		}

		const IPayload *Transaction::getPayload() const {
			return payload.get();
		}

		IPayload *Transaction::getPayload() {
			return payload.get();
		}

		void Transaction::addAttribute(const Attribute &attribute) {
			attributes.push_back(attribute);
		}

		const std::vector<Attribute> &Transaction::getAttributes() const {
			return attributes;
		}

		void Transaction::addProgram(const Program &program) {
			programs.push_back(program);
		}

		const std::vector<Program> &Transaction::getPrograms() const {
			return programs;
		}

		void Transaction::clearPrograms() {
			programs.clear();
		}

		void Transaction::removeDuplicatePrograms() {
			std::set<std::string> programSet;

			for (std::vector<Program>::iterator iter = programs.begin(); iter != programs.end();) {
				std::string key = Utils::encodeHex(iter->getCode());
				if (programSet.find(key) == programSet.end()) {
					programSet.insert(key);
					++iter;
				} else {
					iter = programs.erase(iter);
				}
			}
		}

		const std::string Transaction::getRemark() const {
			return Remark;
		}

		void Transaction::setRemark(const std::string &remark) {
			Remark = remark;
		}

		void Transaction::Serialize(ByteStream &ostream) const {
			serializeUnsigned(ostream);

			ostream.writeVarUint(programs.size());
			for (size_t i = 0; i < programs.size(); i++) {
				programs[i].Serialize(ostream);
			}
		}

		void Transaction::serializeUnsigned(ByteStream &ostream) const {
			ostream.writeBytes(&type, 1);

			ostream.writeBytes(&payloadVersion, 1);

			if (payload == nullptr) {
				Log::getLogger()->error("payload should not be null, payload type = {}, version = {}",
										type, payloadVersion);
				throw std::logic_error("payload should not be null");
			}
			payload->Serialize(ostream);

			ostream.writeVarUint(attributes.size());
			for (size_t i = 0; i < attributes.size(); i++) {
				attributes[i].Serialize(ostream);
			}

			ostream.writeVarUint(inputs.size());
			for (size_t i = 0; i < inputs.size(); i++) {
				inputs[i].Serialize(ostream);
			}

			const std::vector<TransactionOutput> &outputs = getOutputs();
			ostream.writeVarUint(outputs.size());
			for (size_t i = 0; i < outputs.size(); i++) {
				outputs[i].Serialize(ostream);
			}

			ostream.writeUint32(lockTime);
		}

		bool Transaction::Deserialize(ByteStream &istream) {
			reinit();

			if (!istream.readBytes(&type, 1))
				return false;
			if (!istream.readBytes(&payloadVersion, 1))
				return false;

			initPayloadFromType(type);

			if (payload == nullptr) {
				Log::getLogger()->error("new payload with type={} when deserialize error", type);
				return false;
			}
			if (!payload->Deserialize(istream))
				return false;

			uint64_t attributeLength = 0;
			if (!istream.readVarUint(attributeLength))
				return false;

			for (size_t i = 0; i < attributeLength; i++) {
				Attribute attribute;
				if (!attribute.Deserialize(istream)) {
					Log::getLogger()->error("deserialize tx attribute[{}] error", i);
					return false;
				}
				attributes.push_back(attribute);
			}

			uint64_t inCount = 0;
			if (!istream.readVarUint(inCount)) {
				Log::getLogger()->error("deserialize tx inCount error");
				return false;
			}

			for (size_t i = 0; i < inCount; i++) {
				TransactionInput input;
				if (!input.Deserialize(istream)) {
					Log::getLogger()->error("deserialize tx input [{}] error", i);
					return false;
				}
				inputs.push_back(input);
			}

			uint64_t outputLength = 0;
			if (!istream.readVarUint(outputLength)) {
				Log::getLogger()->error("deserialize tx output len error");
				return false;
			}

			for (size_t i = 0; i < outputLength; i++) {
				TransactionOutput output;
				if (!output.Deserialize(istream)) {
					Log::getLogger()->error("deserialize tx output[{}] error", i);
					return false;
				}
				outputs.push_back(output);
			}

			if (!istream.readUint32(lockTime)) {
				Log::getLogger()->error("deserialize tx lock time error");
				return false;
			}

			uint64_t programLength = 0;
			if (!istream.readVarUint(programLength)) {
				Log::getLogger()->error("deserialize tx program length error");
				return false;
			}

			for (size_t i = 0; i < programLength; i++) {
				Program program;
				if (!program.Deserialize(istream)) {
					Log::getLogger()->error("deserialize program[{}] error", i);
					return false;
				}
				programs.push_back(program);
			}

			ByteStream ostream;
			serializeUnsigned(ostream);
			CMBlock buff = ostream.getBuffer();
			BRSHA256_2(&txHash, buff, buff.GetSize());

			return true;
		}

		nlohmann::json Transaction::toJson() const {
			nlohmann::json jsonData;

			jsonData["IsRegistered"] = _isRegistered;

			jsonData["TxHash"] = Utils::UInt256ToString(getHash(), true);
			jsonData["Version"] = version;
			jsonData["LockTime"] = lockTime;
			jsonData["BlockHeight"] = blockHeight;
			jsonData["Timestamp"] = timestamp;

			std::vector<nlohmann::json> inputsJson(inputs.size());
			for (size_t i = 0; i < inputs.size(); ++i) {
				inputsJson[i] = inputs[i].toJson();
			}
			jsonData["Inputs"] = inputsJson;

			jsonData["Type"] = (uint8_t) type;
			jsonData["PayloadVersion"] = payloadVersion;
			jsonData["PayLoad"] = payload->toJson();

			std::vector<nlohmann::json> attributesJson(attributes.size());
			for (size_t i = 0; i < attributes.size(); ++i) {
				attributesJson[i] = attributes[i].toJson();
			}
			jsonData["Attributes"] = attributesJson;

			std::vector<nlohmann::json> programsJson(programs.size());
			for (size_t i = 0; i < programs.size(); ++i) {
				programsJson[i] = programs[i].toJson();
			}
			jsonData["Programs"] = programsJson;

			const std::vector<TransactionOutput> &txOutputs = getOutputs();
			std::vector<nlohmann::json> outputs(txOutputs.size());
			for (size_t i = 0; i < txOutputs.size(); ++i) {
				outputs[i] = txOutputs[i].toJson();
			}
			jsonData["Outputs"] = outputs;

			jsonData["Fee"] = fee;

			jsonData["Remark"] = Remark;

			return jsonData;
		}

		void Transaction::fromJson(const nlohmann::json &jsonData) {
			reinit();

			_isRegistered = jsonData["IsRegistered"];

			txHash = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>(), true);
			version = jsonData["Version"].get<uint32_t>();
			lockTime = jsonData["LockTime"].get<uint32_t>();
			blockHeight = jsonData["BlockHeight"].get<uint32_t>();
			timestamp = jsonData["Timestamp"].get<uint32_t>();

			std::vector<nlohmann::json> inputJsons = jsonData["Inputs"];
			for (size_t i = 0; i < inputJsons.size(); ++i) {
				inputs.push_back(TransactionInput());
				inputs[i].fromJson(inputJsons[i]);
			}

			type = Type(jsonData["Type"].get<uint8_t>());
			payloadVersion = jsonData["PayloadVersion"];
			initPayloadFromType(type);

			if (payload == nullptr) {
				Log::getLogger()->error("payload is nullptr when convert from json");
			} else {
				payload->fromJson(jsonData["PayLoad"]);
			}

			std::vector<nlohmann::json> attributesJson = jsonData["Attributes"];
			for (size_t i = 0; i < attributes.size(); ++i) {
				Attribute attribute;
				attribute.fromJson(attributesJson[i]);
				attributes.push_back(attribute);
			}

			std::vector<nlohmann::json> programsJson = jsonData["Programs"];
			for (size_t i = 0; i < programs.size(); ++i) {
				Program program;
				program.fromJson(programsJson[i]);
				programs.push_back(program);
			}

			std::vector<nlohmann::json> outputsJson = jsonData["Outputs"];
			for (size_t i = 0; i < outputs.size(); ++i) {
				TransactionOutput output;
				output.fromJson(outputsJson[i]);
				outputs.push_back(output);
			}

			fee = jsonData["Fee"].get<uint64_t>();

			Remark = jsonData["Remark"].get<std::string>();
		}

		uint64_t Transaction::calculateFee(uint64_t feePerKb) {
			return ((getSize() + 999) / 1000) * feePerKb;
		}

		uint64_t Transaction::getTxFee(const boost::shared_ptr<Wallet> &wallet) {
			//fixme [refactor] complete me
//			uint64_t fee = 0, inputAmount = 0, outputAmount = 0;
//
//			for (size_t i = 0; i < inputs.size(); ++i) {
//				for (size_t j = 0; j < array_count(wallet->getRaw()->transactions); ++j) {
//					ELATransaction *tx = (ELATransaction *) wallet->getRaw()->transactions[j];
//					if (UInt256Eq(&inputs[i].getTransctionHash(), &tx->raw.txHash)) {
//						inputAmount += tx->outputs[inputs[i].getIndex()]->getAmount();
//					}
//				}
//			}
//
//			for (size_t i = 0; i < outputs.size(); ++i) {
//				outputAmount += outputs[i].getAmount();
//			}
//
//			if (inputAmount >= outputAmount)
//				fee = inputAmount - outputAmount;
//
//			return fee;
			return 0;
		}

		void
		Transaction::generateExtraTransactionInfo(nlohmann::json &rawTxJson, const boost::shared_ptr<Wallet> &wallet,
												  uint32_t blockHeight) {

			std::string remark = wallet->GetRemark(Utils::UInt256ToString(getHash()));
			setRemark(remark);

			nlohmann::json summary;
			summary["TxHash"] = rawTxJson["TxHash"].get<std::string>();
			summary["Status"] = getStatus(blockHeight);
			summary["ConfirmStatus"] = getConfirmInfo(blockHeight);
			summary["Remark"] = getRemark();
			summary["Fee"] = getTxFee(wallet);
			nlohmann::json jOut;
			nlohmann::json jIn;

			//fixme [refactor] get intput info from TransactionInput
//			if (inputs.size() > 0 && wallet->inputFromWallet(&inputs[0])) {
//				std::string toAddress = "";
//
//				if (wallet->containsAddress(outputs[0]->getAddress())) {
//					// transfer to my other address of wallet
//					jOut["Amount"] = outputs[0]->getAmount();
//					jOut["ToAddress"] = outputs[0]->getAddress();
//					summary["Outcoming"] = jOut;
//
//					jIn["Amount"] = outputs[0]->getAmount();
//					jIn["ToAddress"] = outputs[0]->getAddress();
//					summary["Incoming"] = jIn;
//				} else {
//					jOut["Amount"] = outputs[0]->getAmount();
//					jOut["ToAddress"] = outputs[0]->getAddress();
//					summary["Outcoming"] = jOut;
//
//					jIn["Amount"] = 0;
//					jIn["ToAddress"] = "";
//					summary["Incoming"] = jIn;
//				}
//			} else {
//				uint64_t inputAmount = 0;
//				std::string toAddress = "";
//
//				for (size_t i = 0; i < outputs.size(); ++i) {
//					if (wallet->containsAddress(outputs[i]->getAddress())) {
//						inputAmount = outputs[i]->getAmount();
//						toAddress = outputs[i]->getAddress();
//					}
//				}
//
//				jOut["Amount"] = 0;
//				jOut["ToAddress"] = "";
//				summary["Outcoming"] = jOut;
//
//				jIn["Amount"] = inputAmount;
//				jIn["ToAddress"] = toAddress;
//				summary["Incoming"] = jIn;
//			}

			rawTxJson["Summary"] = summary;
			Log::getLogger()->info("transaction summary = {}", summary.dump());
		}

		std::string Transaction::getConfirmInfo(uint32_t blockHeight) {
			if (getBlockHeight() == TX_UNCONFIRMED)
				return std::to_string(0);

			uint32_t confirmCount = blockHeight >= getBlockHeight() ? blockHeight - getBlockHeight() + 1 : 0;
			return confirmCount <= 6 ? std::to_string(confirmCount) : "6+";
		}

		std::string Transaction::getStatus(uint32_t blockHeight) {
			if (getBlockHeight() == TX_UNCONFIRMED)
				return "Unconfirmed";

			uint32_t confirmCount = blockHeight >= getBlockHeight() ? blockHeight - getBlockHeight() + 1 : 0;
			return confirmCount <= 6 ? "Pending" : "Confirmed";
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
				payload = PayloadPtr(new PayloadCoinBase());
			} else if (type == RegisterAsset) {
				payload = PayloadPtr(new PayloadRegisterAsset());
			} else if (type == TransferAsset) {
				payload = PayloadPtr(new PayloadTransferAsset());
			} else if (type == Record) {
				payload = PayloadPtr(new PayloadRecord());
			} else if (type == Deploy) {
				//todo add deploy payload
				//payload = boost::shared_ptr<PayloadDeploy>(new PayloadDeploy());
			} else if (type == SideMining) {
				payload = PayloadPtr(new PayloadSideMining());
			} else if (type == IssueToken) {
				payload = PayloadPtr(new PayloadIssueToken());
			} else if (type == WithdrawAsset) {
				payload = PayloadPtr(new PayloadWithDrawAsset());
			} else if (type == TransferCrossChainAsset) {
				payload = PayloadPtr(new PayloadTransferCrossChainAsset());
			} else if (type == RegisterIdentification) {
				payload = PayloadPtr(new PayloadRegisterIdentification());
			}
		}

		void Transaction::Cleanup() {
			inputs.clear();
			outputs.clear();
			attributes.clear();
			programs.clear();
			payload.reset();
		}

		uint8_t Transaction::getPayloadVersion() const {
			return payloadVersion;
		}

		void Transaction::setPayloadVersion(uint8_t version) {
			payloadVersion = version;
		}

		uint64_t Transaction::getFee() const {
			return fee;
		}

		void Transaction::setFee(uint64_t f) {
			fee = f;
		}

		bool Transaction::IsEqual(const Transaction *tx) const {
			return (tx == this || UInt256Eq(&(((BRTransaction *) tx)->txHash), &(((BRTransaction *) tx)->txHash)));
		}

	}
}
