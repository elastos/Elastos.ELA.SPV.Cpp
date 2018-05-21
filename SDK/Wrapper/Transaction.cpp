// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"

#include <cstring>
#include <BRTransaction.h>

#include "Transaction.h"
#include "Payload/PayloadCoinBase.h"
#include "Payload/PayloadIssueToken.h"
#include "Payload/PayloadWithDrawAsset.h"
#include "Payload/PayloadRecord.h"
#include "Payload/PayloadRegisterAsset.h"
#include "Payload/PayloadSideMining.h"
#include "Payload/PayloadTransferCrossChainAsset.h"
#include "Payload/PayloadTransferAsset.h"
#include "ELABRTransaction.h"
#include "BRCrypto.h"
#include "ELABRTxOutput.h"

namespace Elastos {
	namespace SDK {

		Transaction::Transaction() :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = (BRTransaction *) ELABRTransactionNew();
			convertFrom(_transaction);
		}

		Transaction::Transaction(BRTransaction *transaction) :
				_transaction(transaction),
				_isRegistered(false),
				_payload(nullptr) {
			convertFrom(transaction);
		}

		Transaction::Transaction(const CMBlock &buffer) :
				_isRegistered(false),
				_payload(nullptr) {
			_transaction = BRTransactionParse(buffer, buffer.GetSize());
			assert (nullptr != _transaction);
			convertFrom(_transaction);
		}

		Transaction::Transaction(const CMBlock &buffer, uint32_t blockHeight, uint32_t timeStamp) :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = BRTransactionParse(buffer, buffer.GetSize());
			assert (nullptr != _transaction);
			setPayloadByTransactionType();
			convertFrom(_transaction);

			_transaction->blockHeight = blockHeight;
			_transaction->timestamp = timeStamp;
		}

		Transaction::~Transaction() {
			if (_transaction != nullptr)
				ELABRTransactionFree((ELABRTransaction *) _transaction);
		}

		std::string Transaction::toString() const {
			//todo complete me
			return "";
		}

		BRTransaction *Transaction::getRaw() const {
			return _transaction;
		}

		bool Transaction::isRegistered() const {
			return _isRegistered;
		}

		bool &Transaction::isRegistered() {
			return _isRegistered;
		}

		void Transaction::resetHash() {
			UInt256Set(&_transaction->txHash, UINT256_ZERO);
		}

		UInt256 Transaction::getHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_transaction->txHash, &zero)) {
				ByteStream ostream;
				serializeUnsigned(ostream);
				uint8_t *buff = ostream.getBuf();
				UInt256 hash = UINT256_ZERO;
				BRSHA256_2(&hash, buff, ostream.position());
				UInt256Set(&_transaction->txHash, hash);
				delete buff;
			}
			return _transaction->txHash;
		}

		uint32_t Transaction::getVersion() const {
			return _transaction->version;
		}

		void Transaction::setTransactionType(Transaction::Type type) {
			if (_type != type) {
				_type = type;
				setPayloadByTransactionType();
			}
		}

		Transaction::Type Transaction::getTransactionType() const {
			return _type;
		}

		void Transaction::setPayloadByTransactionType() {
			//todo initializing payload other than just creating
			if (_type == CoinBase) {
				_payload = boost::shared_ptr<PayloadCoinBase>(new PayloadCoinBase());

			} else if (_type == RegisterAsset) {
				_payload = boost::shared_ptr<PayloadRegisterAsset>(new PayloadRegisterAsset());

			} else if (_type == TransferAsset) {
				_payload = boost::shared_ptr<PayloadTransferAsset>(new PayloadTransferAsset());

			} else if (_type == Record) {
				_payload = boost::shared_ptr<PayloadRecord>(new PayloadRecord());
			} else if (_type == Deploy) {
				//todo add deploy payload
			    //_payload = boost::shared_ptr<

			} else if (_type == SideMining) {
				_payload = boost::shared_ptr<PayloadSideMining>(new PayloadSideMining());

			} else if (_type == IssueToken) {
				_payload = boost::shared_ptr<PayloadIssueToken>(new PayloadIssueToken());

			} else if (_type == WithdrawAsset) {
				_payload = boost::shared_ptr<PayloadWithDrawAsset>(new PayloadWithDrawAsset());

			} else if (_type == TransferCrossChainAsset) {
				_payload = boost::shared_ptr<PayloadTransferCrossChainAsset>(new PayloadTransferCrossChainAsset());
			}
		}

		const SharedWrapperList<TransactionInput, BRTxInput *> &Transaction::getInputs() const {

			if (_inputs.empty()) {
				for (int i = 0; i < _transaction->inCount; i++) {
					BRTxInput *input = new BRTxInput;
					transactionInputCopy(input, &_transaction->inputs[i]);
					TransactionInputPtr inputPtr = TransactionInputPtr(new TransactionInput(input));
					_inputs.push_back(inputPtr);
				}
			}

			return _inputs;
		}

		void Transaction::transactionInputCopy(BRTxInput *target, const BRTxInput *source) const {
			assert (target != nullptr);
			assert (source != nullptr);
			*target = *source;

			target->script = nullptr;
			BRTxInputSetScript(target, source->script, source->scriptLen);

			target->signature = nullptr;
			BRTxInputSetSignature(target, source->signature, source->sigLen);
		}

		void Transaction::transactionOutputCopy(BRTxOutput *target, const BRTxOutput *source) const {
			assert (target != nullptr);
			assert (source != nullptr);
			*target = *source;
			target->script = nullptr;
			BRTxOutputSetScript(target, source->script, source->scriptLen);
		}

		std::vector<std::string> Transaction::getInputAddresses() {

			SharedWrapperList<TransactionInput, BRTxInput *> inputs = getInputs();
			ssize_t len = inputs.size();
			std::vector<std::string> addresses(len);
			for (int i = 0; i < len; i++)
				addresses[i] = inputs[i]->getAddress();

			return addresses;
		}

		const SharedWrapperList<TransactionOutput, BRTxOutput *> &Transaction::getOutputs() const {

			ELABRTransaction *elabrTransaction = (ELABRTransaction *)_transaction;
			if (_outputs.empty()) {
				for (int i = 0; i < _transaction->outCount; i++) {
					ELABRTxOutput *output = new ELABRTxOutput;
					transactionOutputCopy((BRTxOutput *)output, &_transaction->outputs[i]);

					UInt256Set(&output->assetId, elabrTransaction->outputAssetIDList[i]);
					output->outputLock = elabrTransaction->outputLockList[i];
					UInt168Set(&output->programHash, elabrTransaction->outputProgramHashList[i]);

					_outputs.push_back(TransactionOutputPtr(new TransactionOutput((BRTxOutput *)output)));
				}
			}

			return _outputs;
		}

		std::vector<std::string> Transaction::getOutputAddresses() {

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			ssize_t len = outputs.size();
			std::vector<std::string> addresses(len);
			for (int i = 0; i < len; i++)
				addresses[i] = outputs[i]->getAddress();

			return addresses;
		}

		uint32_t Transaction::getLockTime() {

			return _transaction->lockTime;
		}

		void Transaction::setLockTime(uint32_t lockTime) {

			_transaction->lockTime = lockTime;
		}

		uint32_t Transaction::getBlockHeight() {

			return _transaction->blockHeight;
		}

		uint32_t Transaction::getTimestamp() {

			return _transaction->timestamp;
		}

		void Transaction::setTimestamp(uint32_t timestamp) {

			_transaction->timestamp = timestamp;
		}

		CMBlock Transaction::serialize() {
			BRTransaction *pbrtx = convertToRaw();

			CMBlock result;
			result.Resize(BRTransactionSerialize(pbrtx, nullptr, 0));
			BRTransactionSerialize(pbrtx, result, result.GetSize());

			ByteStream ostream;
			ELABRTransaction *pelatx = (ELABRTransaction*)pbrtx;
			ostream.put(pelatx->type);
			ostream.put(pelatx->payloadVersion);
			ostream.putVarUint(pelatx->payloadData.GetSize());
			ostream.putBytes(pelatx->payloadData, pelatx->payloadData.GetSize());

			size_t count = pelatx->attributeData.size();
			ostream.putVarUint((uint64_t)count);
			for (size_t i = 0; i < count; i++) {
				CMBlock bd = pelatx->attributeData[i];
				ostream.putVarUint(bd.GetSize());
				if (bd) ostream.putBytes(bd, bd.GetSize());
			}

			count = pelatx->programData.size();
			ostream.putVarUint(count);
			for (size_t i = 0; i < count; i++) {
				CMBlock bd = pelatx->programData[i];
				ostream.putVarUint(bd.GetSize());
				if (bd) ostream.putBytes(bd, bd.GetSize());
			}

			uint8_t * buf = ostream.getBuf();
			uint64_t len = ostream.length();
			uint8_t *buf_tmp = new uint8_t[result.GetSize() + len];
			memcpy(buf_tmp, result, result.GetSize());
			memcpy(buf_tmp + result.GetSize(), buf, len);
			delete[] buf;

			len += result.GetSize();
			result.Resize(len);
			memcpy(result, buf_tmp, len);

			return result;
		}

		void Transaction::addInput(const TransactionInput &input) {

			BRTransactionAddInput(_transaction, input.getHash(), input.getIndex(), input.getAmount(),
			                      input.getScript(), input.getScript().GetSize(),
			                      input.getSignature(), input.getSignature().GetSize(),
			                      input.getSequence());
		}

		void Transaction::addOutput(const TransactionOutput &output) {

			BRTransactionAddOutput(_transaction, output.getAmount(),
			                       output.getScript(), output.getScript().GetSize());

			ELABRTransaction *elabrTransaction = (ELABRTransaction *)_transaction;
			const UInt256 assetID = output.getAssetId();
			elabrTransaction->outputAssetIDList.push_back(assetID);
			elabrTransaction->outputLockList.push_back(output.getOutputLock());
			const UInt168 programHash = output.getProgramHash();
			elabrTransaction->outputProgramHashList.push_back(programHash);
		}

		void Transaction::shuffleOutputs() {

			BRTransactionShuffleOutputs(_transaction);
		}

		size_t Transaction::getSize() {

			return BRTransactionSize(_transaction);
		}

		uint64_t Transaction::getStandardFee() {

			return BRTransactionStandardFee(_transaction);
		}

		bool Transaction::isSigned() {

			return BRTransactionIsSigned(_transaction) != 0;
		}

		void Transaction::sign(const WrapperList<Key, BRKey> &keys, int forkId) {

			BRTransactionSign(_transaction, forkId, keys.getRawArray().data(), keys.size());
		}

		void Transaction::sign(const Key &key, int forkId) {

			WrapperList<Key, BRKey> keys(1);
			keys.push_back(key);
			sign(keys, forkId);
		}

		bool Transaction::isStandard() {

			return BRTransactionIsStandard(_transaction) != 0;
		}

		UInt256 Transaction::getReverseHash() {

			return UInt256Reverse(&_transaction->txHash);
		}

		uint64_t Transaction::getMinOutputAmount() {

			return TX_MIN_OUTPUT_AMOUNT;
		}

		void Transaction::Serialize(ByteStream &ostream) const {
			serializeUnsigned(ostream);

			ostream.putVarUint(_programs.size());
			for (size_t i = 0; i < _programs.size(); i++) {
				_programs[i]->Serialize(ostream);
			}
		}

		void Transaction::serializeUnsigned(ByteStream &ostream) const{
			ostream.put((uint8_t)_type);

			ostream.put(_payloadVersion);

			assert(_payload != nullptr);
			_payload->Serialize(ostream);

			ostream.putVarUint(_attributes.size());
			for (size_t i = 0; i < _attributes.size(); i++) {
				_attributes[i]->Serialize(ostream);
			}

			SharedWrapperList<TransactionInput, BRTxInput *> inputs = getInputs();
			ostream.putVarUint(inputs.size());
			for (size_t i = 0; i < inputs.size(); i++) {
				inputs[i]->Serialize(ostream);
			}

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			ostream.putVarUint(outputs.size());
			for (size_t i = 0; i < outputs.size(); i++) {
				outputs[i]->Serialize(ostream);
			}

			uint8_t lockTimeData[32 / 8];
			memset(lockTimeData, 0, sizeof(lockTimeData));
			UInt32SetLE(lockTimeData, _transaction->lockTime);
			ostream.putBytes(lockTimeData, sizeof(lockTimeData));
		}

		void Transaction::Deserialize(ByteStream &istream) {
			_type = Type(istream.get());
			setPayloadByTransactionType();

			_payloadVersion = istream.get();

			_payload->Deserialize(istream);

			uint64_t attributeLength = istream.getVarUint();
			_attributes.resize(attributeLength);
			for (size_t i = 0; i < attributeLength; i++) {
				_attributes[i] = AttributePtr(new Attribute);
				_attributes[i]->Deserialize(istream);
			}

			uint64_t inputLength = istream.getVarUint();
			_inputs.resize(inputLength);
			for (size_t i = 0; i < inputLength; i++) {
				_inputs[i] = TransactionInputPtr(new TransactionInput);
				_inputs[i]->Deserialize(istream);
			}

			uint64_t outputLength = istream.getVarUint();
			_outputs.resize(outputLength);
			for (size_t i = 0; i < outputLength; i++) {
				_outputs[i] = TransactionOutputPtr(new TransactionOutput);
				_outputs[i]->Deserialize(istream);
			}

			uint8_t lockTimeData[32 / 8];
			istream.getBytes(lockTimeData, sizeof(lockTimeData));
			_transaction->lockTime = UInt32GetLE(lockTimeData);

			uint64_t programLength = istream.getVarUint();
			_programs.resize(programLength);
			for (size_t i = 0; i < programLength; i++) {
				_programs[i] = ProgramPtr(new Program());
				_programs[i]->Deserialize(istream);
			}
		}

		BRTransaction *Transaction::convertToRaw() const {
			ELABRTransaction *transaction = ELABRTransactionNew();
			transaction->raw.txHash = getHash();
			transaction->raw.version = _transaction->version;
			transaction->raw.blockHeight = _transaction->blockHeight;
			transaction->raw.inCount = _transaction->inCount;
			transaction->raw.lockTime = _transaction->lockTime;
			transaction->raw.timestamp = _transaction->timestamp;
			transaction->raw.outCount = _transaction->outCount;

			SharedWrapperList<TransactionInput, BRTxInput *> inputs = getInputs();
			size_t len = inputs.size();
			transaction->raw.inCount = len;
			TransactionInput *input = nullptr;
			for (ssize_t i = 0; i < len; ++i) {
				input = inputs[i].get();
				BRTransactionAddInput(&transaction->raw, input->getHash(), input->getIndex(), input->getAmount(),
				                      input->getScript(), input->getScript().GetSize(), input->getSignature(),
				                      input->getSignature().GetSize(), input->getSequence());
			}

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			transaction->raw.outCount = len = outputs.size();
			TransactionOutput *output = nullptr;
			uint8_t *scriptPubkey = nullptr;

			for (ssize_t i = 0; i < len; i++) {
				output = outputs[i].get();
				BRTransactionAddOutput(&transaction->raw, output->getAmount(), output->getScript(),
				                       output->getScript().GetSize());
				const UInt256 assetID = output->getAssetId();
				transaction->outputAssetIDList.push_back(assetID);
				transaction->outputLockList.push_back(output->getOutputLock());
				const UInt168 programHash = output->getProgramHash();
				transaction->outputProgramHashList.push_back(programHash);
			}

			transaction->type = _type;
			transaction->payloadVersion = _payloadVersion;
			ByteStream byteStream;
			_payload->Serialize(byteStream);
			transaction->payloadData.Resize(byteStream.length());
			if (transaction->payloadData.GetSize() > 0) {
				uint8_t *tmp = byteStream.getBuf();
				memcpy(transaction->payloadData, tmp, byteStream.length());
				delete []tmp;
			}

			transaction->attributeData.clear();
			len = _attributes.size();
			for (ssize_t i = 0; i < len; i++) {
				AttributePtr attr = _attributes[i];
				byteStream.reSet();
				attr->Serialize(byteStream);
				CMBlock pd(byteStream.length());
				uint8_t *tmp = byteStream.getBuf();
				memcpy(pd, tmp, pd.GetSize());
				delete []tmp;
				transaction->attributeData.push_back(pd);
			}

			transaction->programData.clear();
			len = _programs.size();
			for (ssize_t i = 0; i < len; i++) {

				ProgramPtr programPtr = _programs[i];
				byteStream.reSet();
				programPtr->Serialize(byteStream);
				CMBlock pd(byteStream.length());
				uint8_t *tmp = byteStream.getBuf();
				memcpy(pd, tmp, pd.GetSize());
				delete []tmp;
				transaction->programData.push_back(pd);
			}

			return (BRTransaction *) transaction;
		}

		void Transaction::convertFrom(const BRTransaction *raw) {
			assert(raw != nullptr);
			ELABRTransaction *elabrTransaction = ELABRTransactioCopy((ELABRTransaction *) raw);

			//ELABRTransaction *elabrTransaction = ELABRTransactionNew();

			//memcpy(elabrTransaction, raw, sizeof(*elabrTransaction));
			//*elabrTransaction = *((ELABRTransaction*)raw);

			_inputs.clear();
			getInputs();

			_outputs.clear();
			getOutputs();

			_type = (Type) elabrTransaction->type;
			_payloadVersion = elabrTransaction->payloadVersion;

			setPayloadByTransactionType();
			if (elabrTransaction->payloadData && elabrTransaction->payloadData.GetSize() > 0) {
				uint8_t *data = new uint8_t[elabrTransaction->payloadData.GetSize()];
				memcpy(data, elabrTransaction->payloadData, elabrTransaction->payloadData.GetSize());
				ByteStream byteStream1(data, elabrTransaction->payloadData.GetSize());
				_payload->Deserialize(byteStream1);
			}

			_attributes.clear();
			ssize_t len = elabrTransaction->attributeData.size();
			if (len > 0) {
				for (ssize_t i = 0; i < len; ++i) {
					CMBlock byteData = elabrTransaction->attributeData[i];
					uint8_t *data = new uint8_t[byteData.GetSize()];
					memcpy(data, byteData, byteData.GetSize());
					ByteStream byteStream1(data, byteData.GetSize());
					AttributePtr attr(new Attribute());
					attr->Deserialize(byteStream1);
					_attributes.push_back(attr);
				}
			}

			_programs.clear();
			len = elabrTransaction->programData.size();
			if (len > 0) {
				for (ssize_t i = 0; i < len; i++) {
					CMBlock byteData = elabrTransaction->programData[i];
					uint8_t *data = new uint8_t[byteData.GetSize()];
					memcpy(data, byteData, byteData.GetSize());
					ByteStream byteStream(data, byteData.GetSize());
					ProgramPtr programPtr(new Program);
					programPtr->Deserialize(byteStream);
					_programs.push_back(programPtr);
				}
			}

			ELABRTransactionFree(elabrTransaction);
		}
	}
}
