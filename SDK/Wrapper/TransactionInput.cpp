// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRTransaction.h>
#include <cstring>
#include "TransactionInput.h"

namespace Elastos {
	namespace SDK {

		TransactionInput::TransactionInput() {
			_input = boost::shared_ptr<BRTxInput>(new BRTxInput);
			memset(_input.get(), 0, sizeof(BRTxInput));
		}

		TransactionInput::TransactionInput(BRTxInput *input) {
			_input = boost::shared_ptr<BRTxInput>(input);
		}

		TransactionInput::TransactionInput(UInt256 hash, uint32_t index, uint64_t amount, CMBlock script,
										   CMBlock signature, uint32_t sequence) {
			_input = boost::shared_ptr<BRTxInput>(new BRTxInput);
			_input->txHash = hash;
			_input->index = index;
			_input->amount = amount;
			_input->signature = nullptr;
			_input->script = nullptr;
			BRTxInputSetScript(_input.get(), script, script.GetSize());
			BRTxInputSetSignature(_input.get(), signature, signature.GetSize());
			_input->sequence = sequence;
		}

		std::string TransactionInput::toString() const {
			//todo complete me
			return "";
		}

		BRTxInput *TransactionInput::getRaw() const {
			return _input.get();
		}

		std::string TransactionInput::getAddress() const {
			return _input->address;
		}

		void TransactionInput::setAddress(std::string address) {
			BRTxInputSetAddress(_input.get(), address.c_str());
		}

		UInt256 TransactionInput::getHash() const {
			return _input->txHash;
		}

		uint32_t TransactionInput::getIndex() const {
			return _input->index;
		}

		uint64_t TransactionInput::getAmount() const {
			return _input->amount;
		}

		CMBlock TransactionInput::getScript() const {
			CMBlock mb(_input->scriptLen);
			memcpy(mb, _input->script, _input->scriptLen);
			return mb;
		}

		CMBlock TransactionInput::getSignature() const {
			CMBlock data;
			if (_input->sigLen > 0) {
				data.Resize(_input->sigLen);
				memcpy(data, _input->signature, _input->sigLen);
			}
			return data;
		}

		uint32_t TransactionInput::getSequence() const {
			return _input->sequence;
		}

		void TransactionInput::Serialize(ByteStream &ostream) const {
			uint8_t transactionHashData[256 / 8];
			UInt256Set(transactionHashData, _input->txHash);
			ostream.putBytes(transactionHashData, 256 / 8);

			uint8_t indexData[16 / 8];
			UInt16SetLE(indexData, uint16_t(_input->index));
			ostream.putBytes(indexData, 16 / 8);

			uint8_t sequenceData[32 / 8];
			UInt32SetLE(sequenceData, _input->sequence);
			ostream.putBytes(sequenceData, 32 / 8);
		}

		void TransactionInput::Deserialize(ByteStream &istream) {
			uint8_t transactionHashData[256 / 8];
			istream.getBytes(transactionHashData, 256 / 8);
			UInt256Get(&_input->txHash, transactionHashData);

			uint8_t indexData[16 / 8];
			istream.getBytes(indexData, 16 / 8);
			_input->index = UInt16GetLE(indexData);

			uint8_t sequenceData[32 / 8];
			istream.getBytes(sequenceData, 32 / 8);
			_input->sequence = UInt32GetLE(sequenceData);
		}

	}
}
