// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstdlib>
#include <cstring>
//#include <string.h>

#include <BRTransaction.h>

#include "BRArray.h"
#include "ELABRTransaction.h"

namespace Elastos {
	namespace SDK {
		ELABRTransaction &ELABRTransaction::operator = (const _ELABRTransaction &ebt) {
			if (0 < raw.inCount && raw.inputs) {
				for (size_t i = 0; i < raw.inCount; i++) {
					BRTxInputSetScript(&raw.inputs[i], NULL, 0);
					BRTxInputSetSignature(&raw.inputs[i], NULL, 0);
				}
				raw.inCount = 0;
				raw.inputs = NULL;
				array_free(raw.inputs);
				array_new(raw.inputs, 1);
			}

			if (0 < raw.outCount && raw.outputs) {
				for (size_t i = 0; i < raw.outCount; i++) {
					BRTxOutputSetScript(&raw.outputs[i], NULL, 0);
				}
				raw.outCount = 0;
				raw.outputs = NULL;
				array_free(raw.outputs);
				array_new(raw.outputs, 2);
			}

			raw = ebt.raw;
			for (size_t i = 0; i < ebt.raw.inCount; i++) {
				BRTransactionAddInput(&raw, ebt.raw.inputs[i].txHash, ebt.raw.inputs[i].index, ebt.raw.inputs[i].amount,
									  ebt.raw.inputs[i].script, ebt.raw.inputs[i].scriptLen,
									  ebt.raw.inputs[i].signature, ebt.raw.inputs[i].sigLen, ebt.raw.inputs[i].sequence);
			}

			for (size_t i = 0; i < ebt.raw.outCount; i++) {
				BRTransactionAddOutput(&raw, ebt.raw.outputs[i].amount, ebt.raw.outputs[i].script, ebt.raw.outputs[i].scriptLen);
			}
			type = ebt.type;
			payloadVersion = ebt.payloadVersion;
			if (payloadData.data && 0 < payloadData.length) {
				delete[] payloadData.data;
				payloadData.data = nullptr;
				payloadData.length = 0;
			}
			uint8_t *buf = new uint8_t[ebt.payloadData.length];
			memcpy(buf, ebt.payloadData.data, ebt.payloadData.length);
			payloadData = ByteData(buf, ebt.payloadData.length);

			size_t count = attributeData.size();
			for (size_t i = 0; i < count; i++) {
				ByteData bd = attributeData[i];
				if (bd.data) delete[] bd.data;
			}
			attributeData.clear();
			count = programData.size();
			for (size_t i = 0; i < count; i++) {
				ByteData bd = programData[i];
				if (bd.data) delete[] bd.data;
			}
			programData.clear();
		}

		// returns a newly allocated empty transaction that must be freed by calling ELABRTransactionFree()
		ELABRTransaction *ELABRTransactionNew(void) {
			ELABRTransaction *tx = (ELABRTransaction *) calloc(1, sizeof(ELABRTransaction));
			memset(tx, 0, sizeof(ELABRTransaction));
			array_new(tx->raw.inputs, 1);
			array_new(tx->raw.outputs, 2);
			tx->type = 0;
			tx->payloadVersion = 0;
			tx->programData.clear();
			tx->attributeData.clear();
			return tx;
		}

		ELABRTransaction *ELABRTransactioCopy(const ELABRTransaction *tx) {
			ELABRTransaction *elabrTransaction = ELABRTransactionNew();
			elabrTransaction->raw.timestamp = tx->raw.timestamp;
			elabrTransaction->raw.version = tx->raw.version;
			UInt256Set(&elabrTransaction->raw.txHash, tx->raw.txHash);
			elabrTransaction->raw.blockHeight = tx->raw.blockHeight;
			elabrTransaction->raw.inCount = tx->raw.inCount;
			elabrTransaction->raw.lockTime = tx->raw.lockTime;
			elabrTransaction->raw.outCount = tx->raw.outCount;

			for (size_t i = 0; i < tx->raw.inCount; i++) {
				BRTransactionAddInput(&elabrTransaction->raw, tx->raw.inputs[i].txHash, tx->raw.inputs[i].index, tx->raw.inputs[i].amount,
									  tx->raw.inputs[i].script, tx->raw.inputs[i].scriptLen,
									  tx->raw.inputs[i].signature, tx->raw.inputs[i].sigLen, tx->raw.inputs[i].sequence);
			}

			for (size_t i = 0; i < tx->raw.outCount; i++) {
				BRTransactionAddOutput(&elabrTransaction->raw, tx->raw.outputs[i].amount, tx->raw.outputs[i].script, tx->raw.outputs[i].scriptLen);
			}


			elabrTransaction->type = tx->type;
			elabrTransaction->payloadVersion = tx->payloadVersion;
			for (size_t i = 0; i < tx->programData.size(); ++i) {
				ByteData byteData(new uint8_t[tx->programData[i].length], tx->programData[i].length);
				memcpy(byteData.data, tx->programData[i].data, tx->programData[i].length);
				elabrTransaction->programData.push_back(byteData);
			}
			for (size_t i = 0; i < tx->attributeData.size(); ++i) {
				ByteData byteData(new uint8_t[tx->attributeData[i].length], tx->attributeData[i].length);
				memcpy(byteData.data, tx->attributeData[i].data, tx->attributeData[i].length);
				elabrTransaction->attributeData.push_back(byteData);
			}
			return elabrTransaction;
		}

		// frees memory allocated for tx
		void ELABRTransactionFree(ELABRTransaction *tx) {
			if (0 < tx->raw.inCount && tx->raw.inputs) {
				for (size_t i = 0; i < tx->raw.inCount; i++) {
					BRTxInputSetScript(&tx->raw.inputs[i], NULL, 0);
					BRTxInputSetSignature(&tx->raw.inputs[i], NULL, 0);
				}
				array_free(tx->raw.inputs);
			}

			if (0 < tx->raw.outCount && tx->raw.outputs) {
				for (size_t i = 0; i < tx->raw.outCount; i++) {
					BRTxOutputSetScript(&tx->raw.outputs[i], NULL, 0);
				}
				array_free(tx->raw.outputs);
			}

			if (tx->payloadData.data && 0 < tx->payloadData.length) {
				delete[] tx->payloadData.data;
			}

			size_t count = tx->attributeData.size();
			for (size_t i = 0; i < count; i++) {
				ByteData bd = tx->attributeData[i];
				if (bd.data) delete[] bd.data;
			}

			count = tx->programData.size();
			for (size_t i = 0; i < count; i++) {
				ByteData bd = tx->programData[i];
				if (bd.data) delete[] bd.data;
			}

			free(tx);
		}
	}
}