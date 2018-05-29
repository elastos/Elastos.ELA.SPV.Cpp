// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <SDK/ELACoreExt/ELABRTxOutput.h>

#include "BRTransaction.h"
#include "BRWallet.h"
#include "BRKey.h"
#include "BRArray.h"

#include "SubWallet.h"
#include "MasterWallet.h"
#include "SubWalletCallback.h"
#include "Utils.h"
#include "Log.h"
#include "SubWalletType.h"
#include "TransactionOutput.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"
#define PEER_CONFIG_EXTENSION "_PeerConnection.json"

namespace Elastos {
	namespace SDK {

		SubWallet::SubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const std::string &payPassword,
							 MasterWallet *parent) :
				_parent(parent),
				_info(info) {

			fs::path subWalletDbPath = _parent->_dbRoot;
			subWalletDbPath /= info.getChainId() + DB_FILE_EXTENSION;

			fs::path peerConnectionPath = _parent->_dbRoot;
			peerConnectionPath /= info.getChainId() + PEER_CONFIG_EXTENSION;

			BRKey key;
			UInt256 chainCode;
			deriveKeyAndChain(&key, chainCode, payPassword);
			MasterPubKeyPtr masterPubKey(new MasterPubKey(key, chainCode));

			_walletManager = WalletManagerPtr(
					new WalletManager(masterPubKey, subWalletDbPath, peerConnectionPath, _info.getEarliestPeerTime(),
									  _info.getSingleAddress(), _info.getForkId(), chainParams));
			_walletManager->registerWalletListener(this);

			if (info.getFeePerKb() > 0) {
				_walletManager->getWallet()->setFeePerKb(info.getFeePerKb());
			}
		}

		SubWallet::~SubWallet() {

		}

		nlohmann::json SubWallet::GetBalanceInfo() {
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			assert(wallet != nullptr);

			size_t utxosCount = BRWalletUTXOs(wallet, nullptr, 0);
			BRUTXO utxos[utxosCount];
			BRWalletUTXOs(wallet, utxos, utxosCount);

			nlohmann::json j;

			BRTransaction *t;
			std::map<std::string, uint64_t> addressesBalanceMap;
			pthread_mutex_lock(&wallet->lock);
			for (size_t i = 0; i < utxosCount; ++i) {
				void *tempPtr = BRSetGet(wallet->allTx, &utxos[utxosCount].hash);
				if (tempPtr == nullptr) continue;
				t = static_cast<BRTransaction *>(tempPtr);

				if (addressesBalanceMap.find(t->outputs[utxos->n].address) != addressesBalanceMap.end()) {
					addressesBalanceMap[t->outputs[utxos->n].address] += t->outputs[utxos->n].amount;
				} else {
					addressesBalanceMap[t->outputs[utxos->n].address] = t->outputs[utxos->n].amount;
				}
			}
			pthread_mutex_unlock(&wallet->lock);

			std::vector<nlohmann::json> balances;
			std::for_each(addressesBalanceMap.begin(), addressesBalanceMap.end(),
						  [&addressesBalanceMap, &balances](const std::map<std::string, uint64_t>::value_type &item) {
							  nlohmann::json balanceKeyValue;
							  balanceKeyValue[item.first] = item.second;
							  balances.push_back(balanceKeyValue);
						  });

			j["Balances"] = balances;
			return j;
		}

		uint64_t SubWallet::GetBalance() {
			return _walletManager->getWallet()->getBalance();
		}

		std::string SubWallet::CreateAddress() {
			return _walletManager->getWallet()->getReceiveAddress();
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start,
												uint32_t count) {
			std::vector<std::string> addresses = _walletManager->getWallet()->getAllAddresses();
			uint32_t end = std::min(start + count, (uint32_t) addresses.size());
			std::vector<std::string> results(addresses.begin() + start, addresses.begin() + end);
			nlohmann::json j;
			j["addresses"] = addresses;
			return j;
		}

		uint64_t SubWallet::GetBalanceWithAddress(const std::string &address) {
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			assert(wallet != nullptr);

			size_t utxosCount = BRWalletUTXOs(wallet, nullptr, 0);
			BRUTXO utxos[utxosCount];
			BRWalletUTXOs(wallet, utxos, utxosCount);

			BRTransaction *t;
			uint64_t balance = 0;
			pthread_mutex_lock(&wallet->lock);
			for (size_t i = 0; i < utxosCount; ++i) {
				void *tempPtr = BRSetGet(wallet->allTx, &utxos[utxosCount].hash);
				if (tempPtr == nullptr) continue;
				t = static_cast<BRTransaction *>(tempPtr);
				if (BRAddressEq(t->outputs[utxos->n].address, address.c_str())) {
					balance += t->outputs[utxos->n].amount;
				}
			}
			pthread_mutex_unlock(&wallet->lock);

			return balance;
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		std::string
		SubWallet::SendTransaction(const std::string &fromAddress, const std::string &toAddress, uint64_t amount,
								   uint64_t fee, const std::string &payPassword, const std::string &memo) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Normal, fromAddress, toAddress, amount, fee, memo));
			TransactionPtr transaction = createTransaction(txParam.get());
			return sendTransactionInternal(transaction, payPassword);
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			assert(wallet != NULL);

			size_t realCount = count;
			pthread_mutex_lock(&wallet->lock);
			if (array_count(wallet->transactions) < start + realCount)
				realCount = array_count(wallet->transactions) - start;

			BRTransaction *transactions[realCount];
			for (size_t i = 0; i < realCount; i++) {
				transactions[i] = wallet->transactions[i + start];
			}
			pthread_mutex_unlock(&wallet->lock);

			std::vector<nlohmann::json> jsonList(realCount);
			for (size_t i = 0; i < realCount; ++i) {
				TransactionPtr transactionPtr(new Transaction(transactions[i]));
				jsonList[i] = transactionPtr->toJson();
			}
			return nlohmann::json(jsonList);
		}

		boost::shared_ptr<Transaction> SubWallet::createTransaction(TxParam *param) const {
			//todo consider the situation of from address and fee not null
			//todo initialize asset id if null
			BRTransaction *tmp = BRWalletCreateTransaction(_walletManager->getWallet()->getRaw(), param->getAmount(),
														   param->getToAddress().c_str());
			if (!tmp) return nullptr;

			TransactionPtr ptr(new Transaction(tmp));
			ptr->setTransactionType(Transaction::TransferAsset);
			SharedWrapperList<TransactionOutput, BRTxOutput *> outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](const SharedWrapperList<TransactionOutput, BRTxOutput *>::TPtr &output) {
							  ((ELABRTxOutput *) output->getRaw())->assetId = param->getAssetId();
						  });

			return ptr;
		}

		std::string SubWallet::sendTransactionInternal(const boost::shared_ptr<Transaction> &transaction,
													   const std::string &payPassword) {
			BRTransaction *rawTransaction = transaction->convertToRaw();
			signTransaction(rawTransaction, _info.getForkId(), payPassword);
			_walletManager->getPeerManager()->publishTransaction(transaction);

			Transaction txForHash(rawTransaction);
			return std::string(Utils::UInt256ToString(txForHash.getHash()), 32);
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {
			boost::shared_ptr<BRKey> rawKey = boost::shared_ptr<BRKey>(new BRKey);
			UInt256 chainCode;
			deriveKeyAndChain(rawKey.get(), chainCode, payPassword);
			Key key(rawKey);

			CMBlock signedData = key.sign(Utils::UInt256FromString(message));

			char *data = new char[signedData.GetSize()];
			memcpy(data, signedData, signedData.GetSize());
			std::string singedMsg(data, signedData.GetSize());
			return singedMsg;
		}

		nlohmann::json
		SubWallet::CheckSign(const std::string &publicKey, const std::string &message, const std::string &signature) {
			CMBlock signatureData(signature.size());
			memcpy(signatureData, signature.c_str(), signature.size());

			bool r = Key::verifyByPublicKey(publicKey, Utils::UInt256FromString(message), signatureData);
			//todo json return is correct ?
			nlohmann::json jsonData;
			jsonData["result"] = r;
			return jsonData;
		}

		void SubWallet::balanceChanged(uint64_t balance) {

		}

		void SubWallet::onTxAdded(const TransactionPtr &transaction) {
			std::for_each(_callbacks.begin(), _callbacks.end(), [transaction](ISubWalletCallback *callback) {
				callback->OnTransactionStatusChanged(std::string((char *) transaction->getHash().u8, 32),
													 SubWalletCallback::convertToString(SubWalletCallback::Added),
													 nlohmann::json(), transaction->getBlockHeight());
			});
		}

		void SubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, blockHeight, timeStamp](ISubWalletCallback *callback) {

							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Updated), nlohmann::json(), blockHeight);
						  });
		}

		void SubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, notifyUser, recommendRescan](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Added), nlohmann::json(), 0);
						  });
		}

		void SubWallet::recover(int limitGap) {
			_walletManager->recover(limitGap);
		}

		void SubWallet::deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const std::string &payPassword) {
			UInt512 seed = _parent->deriveSeed(payPassword);
			Key::deriveKeyAndChain(key, chainCode, &seed, sizeof(seed), 3, 44, _info.getIndex(), 0);
		}

		void SubWallet::signTransaction(BRTransaction *transaction, int forkId, const std::string &payPassword) {
			BRKey masterKey;
			UInt256 chainCode;
			deriveKeyAndChain(&masterKey, chainCode, payPassword);
			BRWallet *wallet = _walletManager->getWallet()->getRaw();

			uint32_t j, internalIdx[transaction->inCount], externalIdx[transaction->inCount];
			size_t i, internalCount = 0, externalCount = 0;
			int r = 0;

			assert(wallet != nullptr);
			assert(transaction != nullptr);

			pthread_mutex_lock(&wallet->lock);

			for (i = 0; i < transaction->inCount; i++) {
				for (j = (uint32_t) array_count(wallet->internalChain); j > 0; j--) {
					if (BRAddressEq(transaction->inputs[i].address, &wallet->internalChain[j - 1]))
						internalIdx[internalCount++] = j - 1;
				}

				for (j = (uint32_t) array_count(wallet->externalChain); j > 0; j--) {
					if (BRAddressEq(transaction->inputs[i].address, &wallet->externalChain[j - 1]))
						externalIdx[externalCount++] = j - 1;
				}
			}

			pthread_mutex_unlock(&wallet->lock);

			BRKey keys[internalCount + externalCount];
			Key::calculatePrivateKeyList(keys, internalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_INTERNAL_CHAIN, internalIdx);
			Key::calculatePrivateKeyList(&keys[internalCount], externalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_EXTERNAL_CHAIN, externalIdx);

			BRTransactionSign(transaction, forkId, keys, internalCount + externalCount);
			for (i = 0; i < internalCount + externalCount; i++) BRKeyClean(&keys[i]);
		}

		std::string SubWallet::CreateMultiSignAddress(const nlohmann::json &multiPublicKeyJson, uint32_t totalSignNum,
													  uint32_t requiredSignNum) {
			//todo complete me
			return "";
		}

		nlohmann::json
		SubWallet::GenerateMultiSignTransaction(const std::string &fromAddress, const std::string &toAddress,
												uint64_t amount, uint64_t fee, const std::string &payPassword,
												const std::string &memo) {
			//todo complete me
			return nlohmann::json();
		}

		std::string
		SubWallet::SendRawTransaction(const nlohmann::json &transactionJson, const nlohmann::json &signJson) {
			TransactionPtr transaction(new Transaction());

			transaction->fromJson(transactionJson);

			_walletManager->signAndPublishTransaction(transaction);

			std::string hash = Utils::UInt256ToString(transaction->getHash());

			return hash;
		}

	}
}