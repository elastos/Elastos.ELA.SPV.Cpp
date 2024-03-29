/*
 * EthereumTransaction
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 3/20/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 * Copyright (c) 2020 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Common/Log.h>
#include "EthereumTransfer.h"
#include "EthereumEWM.h"

namespace Elastos {
	namespace ElaWallet {

		EthereumTransfer::EthereumTransfer(EthereumEWM *ewm, BREthereumTransfer transfer, EthereumAmount::Unit unit) :
			ReferenceWithDefaultUnit(ewm, transfer, unit) {
		}

		EthereumTransfer::~EthereumTransfer() {
            transferRelease ((BREthereumTransfer) _identifier);
		}

		BREthereumTransfer EthereumTransfer::getRaw() const {
			return (BREthereumTransfer) _identifier;
		}

		bool EthereumTransfer::isConfirmed() const {
			return ETHEREUM_BOOLEAN_TRUE == ewmTransferIsConfirmed(_ewm->getRaw(), getRaw());
		}

		bool EthereumTransfer::isSubmitted() const {
			return ETHEREUM_BOOLEAN_TRUE == ewmTransferIsSubmitted(_ewm->getRaw(), getRaw());
		}

		bool EthereumTransfer::isErrored() const {
			return TRANSFER_STATUS_ERRORED == ewmTransferGetStatus(_ewm->getRaw(), getRaw());
		}

		std::string EthereumTransfer::getSourceAddress() const {
			BREthereumAddress source = ewmTransferGetSource(_ewm->getRaw(), getRaw());
			return GetCString(addressGetEncodedString(&source, 1));
		}

		std::string EthereumTransfer::getTargetAddress() const {
			BREthereumAddress *target = ewmTransferGetTarget(_ewm->getRaw(), getRaw());
			return GetCString(addressGetEncodedString(target, 1));
		}

		std::string EthereumTransfer::getIdentifier() const {
			BREthereumHash h = ewmTransferGetIdentifier(_ewm->getRaw(), getRaw());
			return GetCString(hashAsString(h));
		}

		std::string EthereumTransfer::getOriginationTransactionHash() const {
			BREthereumHash h = ewmTransferGetOriginatingTransactionHash(_ewm->getRaw(), getRaw());
			return GetCString(hashAsString(h));
		}

		// Amount
		std::string EthereumTransfer::getAmount() const {
			return getAmount(_defaultUnit);
		}

		std::string EthereumTransfer::getAmount(EthereumAmount::Unit unit) const {
			validUnitOrException(unit);
			BREthereumAmount amount = ewmTransferGetAmount(_ewm->getRaw(), getRaw());
			BREthereumBoolean holdsToken = ewmTransferHoldsToken(_ewm->getRaw(), getRaw(), NULL);

			std::string amountString;
			if (ETHEREUM_BOOLEAN_TRUE == holdsToken) {
				amountString = GetCString(ewmCoerceEtherAmountToString(_ewm->getRaw(), amount.u.ether,
																	   (BREthereumEtherUnit) unit));
			} else {
				amountString = GetCString(ewmCoerceTokenAmountToString(_ewm->getRaw(), amount.u.tokenQuantity,
																	   (BREthereumTokenQuantityUnit) unit));
			}

			return amountString;
		}

		std::string EthereumTransfer::getFee() const {
			return getFee(EthereumAmount::Unit::ETHER_WEI);
		}

		std::string EthereumTransfer::getFee(EthereumAmount::Unit unit) const {
			int overflow = 0;
			BREthereumEther fee = ewmTransferGetFee(_ewm->getRaw(), getRaw(), &overflow);
			std::string feeString;
			if (0 == overflow) {
				feeString = GetCString(ewmCoerceEtherAmountToString(_ewm->getRaw(), fee,
																	(BREthereumEtherUnit) unit));
			}
			return feeString;
		}

		std::string EthereumTransfer::getGasPrice() const {
			return getGasPrice(EthereumAmount::Unit::ETHER_WEI);
		}

		std::string EthereumTransfer::getGasPrice(EthereumAmount::Unit unit) const {
			BREthereumGasPrice price = ewmTransferGetGasPrice(_ewm->getRaw(), getRaw(),
															  (BREthereumEtherUnit) unit);
			return GetCString(ewmCoerceEtherAmountToString(_ewm->getRaw(),
														   price.etherPerGas,
														   (BREthereumEtherUnit) unit));
		}

		uint64_t EthereumTransfer::getGasLimit() const {
			BREthereumGas limit = ewmTransferGetGasLimit(_ewm->getRaw(), getRaw());
			return limit.amountOfGas;
		}

		uint64_t EthereumTransfer::getGasUsed() const {
			BREthereumGas gas = ewmTransferGetGasUsed(_ewm->getRaw(), getRaw());
			return gas.amountOfGas;
		}

		// Nonce
		uint64_t EthereumTransfer::getNonce() const {
			return ewmTransferGetNonce(_ewm->getRaw(), getRaw());
		}

		// Block Number, Timestamp
		uint64_t EthereumTransfer::getBlockNumber() const {
			return ewmTransferGetBlockNumber(_ewm->getRaw(), getRaw());
		}

		uint64_t EthereumTransfer::getBlockTimestamp() const {
			return ewmTransferGetBlockTimestamp(_ewm->getRaw(), getRaw());
		}

		uint64_t EthereumTransfer::getBlockConfirmations() const {
			return ewmTransferGetBlockConfirmations(_ewm->getRaw(), getRaw());
		}

		std::string EthereumTransfer::getErrorDescription() const {
			std::string desc;
			char *errorDescription = ewmTransferStatusGetError(_ewm->getRaw(), getRaw());
			if (errorDescription != NULL)
				desc = GetCString(errorDescription);

			return desc;
		}

        std::string EthereumTransfer::RlpEncode(BREthereumNetwork network, BREthereumRlpType type) const {
            BREthereumTransaction tx = transferGetOriginatingTransaction (getRaw());

            char *p = transactionGetRlpHexEncoded (tx, network, type, NULL);
            std::string txEncoded(p);

            free(p);

            return txEncoded;
		}

		nlohmann::json EthereumTransfer::ToJson() const {
			nlohmann::json j;
			j["IsConfirmed"] = isConfirmed();
			j["IsSubmitted"] = isSubmitted();
			j["IsErrored"] = isErrored();
			j["ErrorDesc"] = isErrored() ? getErrorDescription() : "";
			j["Hash"] = getIdentifier();
			j["OrigTxHash"] = getOriginationTransactionHash();
			j["Amount"] = getAmount(EthereumAmount::ETHER_WEI);
			j["Timestamp"] = getBlockTimestamp();
			j["Fee"] = getFee(EthereumAmount::ETHER_WEI);
			j["Confirmations"] = getBlockConfirmations();
			j["GasPrice"] = getGasPrice();
			j["GasLimit"] = getGasLimit();
			j["GasUsed"] = getGasUsed();
			j["BlockNumber"] = getBlockNumber();
			j["SourceAddress"] = getSourceAddress();
			j["TargetAddress"] = getTargetAddress();
			j["Nonce"] = getNonce();

			BREthereumTransfer rawTransfer = getRaw();
			BREthereumTransaction rawTx = transferGetBasisTransaction(rawTransfer);

			if (rawTx != NULL) {
				BREthereumContractFunction function = contractLookupFunctionForEncoding(contractERC20, transactionGetData( rawTx));
				if (NULL != function && functionERC20Transfer == function) {
					BRCoreParseStatus status;
					UInt256 funcAmount = functionERC20TransferDecodeAmount(function, transactionGetData(rawTx), &status);
					char *funcAddr = functionERC20TransferDecodeAddress(function, transactionGetData(rawTx));
					char *funcAmt = coerceString(funcAmount, 10);
					j["Token"] = getTargetAddress();
					j["TokenFunction"] = "ERC20Transfer";
					j["TokenAmount"] = funcAmt;
					j["TokenAddress"] = funcAddr;
					free(funcAmt);
					free(funcAddr);
				}
			} else {
				BREthereumLog rawLog = transferGetBasisLog(rawTransfer);
				if (rawLog == nullptr) {
					Log::warn("Transaction & Log is null");
				} else {
					BREthereumHash logHash = logGetHash (rawLog);
					char *logHashString = hashAsString(logHash);

					BREthereumAddress logAddress = logGetAddress(rawLog);
					char *logAddressString = addressGetEncodedString(&logAddress, 1);

					nlohmann::json topicArray = nlohmann::json::array();
					size_t topicCount = logGetTopicsCount(rawLog);
					for (size_t i = 0; i < topicCount; ++i) {
						BREthereumLogTopic topic = logGetTopic(rawLog, i);
						BREthereumLogTopicString topicString = logTopicAsString (topic);
						topicArray.push_back(topicString.chars);
					}

					j["LogHash"] = logHashString;
					j["LogAddress"] = logAddressString;
					j["LogTopics"] = topicArray;

					free(logHashString);
					free(logAddressString);
				}
			}
			return j;
		}

	}
}