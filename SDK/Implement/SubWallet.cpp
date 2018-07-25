// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <SDK/ELACoreExt/ELATxOutput.h>
#include <Core/BRTransaction.h>

#include "BRTransaction.h"
#include "BRWallet.h"
#include "BRKey.h"
#include "BRArray.h"

#include "SubWallet.h"
#include "MasterWallet.h"
#include "SubWalletCallback.h"
#include "Utils.h"
#include "Log.h"
#include "ErrorCode.h"
#include "ElaPeerConfig.h"
#include "ParamChecker.h"
#include "Transaction/TransactionOutput.h"
#include "Transaction/TransactionChecker.h"
#include "Transaction/TransactionCompleter.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const std::string &payPassword,
							 const PluginTypes &pluginTypes,
							 MasterWallet *parent) :
				PeerManager::Listener(pluginTypes),
				_parent(parent),
				_info(info),
				_syncStartHeight(0) {

			fs::path subWalletDbPath = _parent->_rootPath;
			subWalletDbPath /= parent->GetId();
			subWalletDbPath /= info.getChainId() + DB_FILE_EXTENSION;

			nlohmann::json peerConfig =
					info.getWalletType() == Mainchain || info.getWalletType() == Normal ? ElaPeerConfig
																						: readPeerConfig();

			CMBlock encryptedKey;
			UInt256 chainCode = UINT256_ZERO;
			MasterPubKeyPtr masterPubKey = nullptr;
			if (!payPassword.empty()) {
				BRKey key;
				deriveKeyAndChain(&key, chainCode, payPassword);

				char rawKey[BRKeyPrivKey(&key, nullptr, 0)];
				BRKeyPrivKey(&key, rawKey, sizeof(rawKey));

				CMBlock ret(sizeof(rawKey));
				memcpy(ret, &rawKey, sizeof(rawKey));
				encryptedKey = Utils::encrypt(ret, payPassword);

				Key wrapperKey(key.secret, key.compressed);
				CMBlock pubKey = wrapperKey.getPubkey();

				_info.setEncryptedKey(Utils::encodeHex(encryptedKey));
				_info.setChainCode(Utils::UInt256ToString(chainCode));
				_info.setPublicKey(Utils::encodeHex(pubKey));

				masterPubKey.reset(new MasterPubKey(key, chainCode));

			} else {
				ParamChecker::checkNotEmpty(_info.getEncryptedKey(), false);
				ParamChecker::checkNotEmpty(_info.getPublicKey(), false);
				ParamChecker::checkNotEmpty(_info.getChainCode(), false);

				chainCode = Utils::UInt256FromString(_info.getChainCode());
				CMBlock pubKey = Utils::decodeHex(_info.getPublicKey());

				masterPubKey.reset(new MasterPubKey(pubKey, Utils::UInt256FromString(_info.getChainCode())));
			}

			_walletManager = WalletManagerPtr(
					new WalletManager(masterPubKey, subWalletDbPath, peerConfig, _info.getEarliestPeerTime(),
									  _info.getSingleAddress(), _info.getForkId(), pluginTypes, chainParams));

			_walletManager->registerWalletListener(this);
			_walletManager->registerPeerManagerListener(this);

			if (info.getFeePerKb() > 0) {
				_walletManager->getWallet()->setFeePerKb(info.getFeePerKb());
			}
		}

		SubWallet::~SubWallet() {

		}

		std::string SubWallet::GetChainId() const {
			return _info.getChainId();
		}

		const SubWallet::WalletManagerPtr &SubWallet::GetWalletManager() const {
			return _walletManager;
		}

		nlohmann::json SubWallet::GetBalanceInfo() {
			return _walletManager->getWallet()->GetBalanceInfo();
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
			j["Addresses"] = addresses;
			return j;
		}

		uint64_t SubWallet::GetBalanceWithAddress(const std::string &address) {
			return _walletManager->getWallet()->GetBalanceWithAddress(address);
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			if (std::find(_callbacks.begin(), _callbacks.end(), subCallback) != _callbacks.end())
				return;
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		nlohmann::json SubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
													uint64_t amount, const std::string &memo,
													const std::string &remark) {
			boost::scoped_ptr<TxParam> txParam(TxParamFactory::createTxParam(Normal, fromAddress, toAddress, amount,
																			 _info.getMinFee(), memo, remark));
			TransactionPtr transaction = createTransaction(txParam.get());
			if (!transaction)
				throw std::logic_error("create transaction error.");
			return transaction->toJson();
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			assert(wallet != nullptr);

			size_t fullTxCount = array_count(wallet->transactions);
			size_t pageCount = count;
			pthread_mutex_lock(&wallet->lock);
			if (fullTxCount < start + count)
				pageCount = fullTxCount - start;

			BRTransaction *transactions[pageCount];
			uint32_t realCount = 0;
			for (size_t i = 0; i < pageCount; i++) {
				if (!filterByAddressOrTxId(
						wallet->transactions[start + pageCount - i - 1], addressOrTxid))
					continue;
				transactions[realCount] =
						wallet->transactions[start + pageCount - i - 1];
				realCount++;
			}
			pthread_mutex_unlock(&wallet->lock);

			std::vector<nlohmann::json> jsonList(realCount);
			for (size_t i = 0; i < realCount; ++i) {
				TransactionPtr transactionPtr(new Transaction((ELATransaction *) transactions[i], false));
				nlohmann::json txJson = transactionPtr->toJson();
				transactionPtr->generateExtraTransactionInfo(txJson, _walletManager->getWallet(),
															 _walletManager->getPeerManager()->getLastBlockHeight());
				jsonList[i] = txJson;
			}
			nlohmann::json j;
			j["Transactions"] = jsonList;
			return j;
		}

		boost::shared_ptr<Transaction>
		SubWallet::createTransaction(TxParam *param) const {
			//todo consider the situation of from address and fee not null
			//todo initialize asset id if null
			TransactionPtr ptr = _walletManager->getWallet()->
					createTransaction(param->getFromAddress(), std::max(param->getFee(), _info.getMinFee()),
									  param->getAmount(), param->getToAddress(), param->getRemark(),
									  param->getMemo());
			if (!ptr) return nullptr;

			ptr->setTransactionType(ELATransaction::TransferAsset);
			const std::vector<TransactionOutput *> &outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](TransactionOutput *output) {
							  ((ELATxOutput *) output->getRaw())->assetId = param->getAssetId();
						  });

			return ptr;
		}

		nlohmann::json SubWallet::sendTransactionInternal(const boost::shared_ptr<Transaction> &transaction,
														  const std::string &payPassword) {
			signTransaction(transaction, _info.getForkId(), payPassword);
			transaction->removeDuplicatePrograms();
			publishTransaction(transaction);

			nlohmann::json j;
			j["TxHash"] = Utils::UInt256ToString(transaction->getHash(), true);
			j["Fee"] = transaction->getStandardFee();
			return j;
		}

		void SubWallet::publishTransaction(const TransactionPtr &transaction) {
			_walletManager->publishTransaction(transaction);
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {

			Key key = deriveKey(payPassword);
			return key.compactSign(message);
		}

		nlohmann::json
		SubWallet::CheckSign(const std::string &publicKey, const std::string &message, const std::string &signature) {
			return _parent->CheckSign(publicKey, message, signature);
		}

		uint64_t SubWallet::CalculateTransactionFee(const nlohmann::json &rawTransaction, uint64_t feePerKb) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(rawTransaction);
			return std::max(transaction->calculateFee(feePerKb), _info.getMinFee());
		}

		void SubWallet::balanceChanged(uint64_t balance) {
			SPDLOG_DEBUG(Log::getLogger(), "Tx callback (balanceChanged): balance={}", balance);
		}

		void SubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction == nullptr)
				return;


			std::string txHash = Utils::UInt256ToString(transaction->getHash());
			SPDLOG_DEBUG(Log::getLogger(), "Tx callback (onTxAdded): Tx hash={}", txHash);
			_confirmingTxs[txHash] = transaction;

			fireTransactionStatusChanged(txHash, SubWalletCallback::convertToString(SubWalletCallback::Added),
										 transaction->toJson(), 0);
			SPDLOG_DEBUG(Log::getLogger(), "Tx callback (onTxAdded) finished. Details: txHash={}, confirm count={}.",
						 txHash, 0);
		}

		void SubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			SPDLOG_DEBUG(Log::getLogger(), "Tx callback (onTxUpdated)");
			if (_confirmingTxs.find(hash) == _confirmingTxs.end()) {
				_confirmingTxs[hash] = _walletManager->getWallet()->transactionForHash(Utils::UInt256FromString(hash));
			}

			SPDLOG_DEBUG(Log::getLogger(), "Tx callback (onTxUpdated): Tx hash={}", hash);
			uint32_t confirm = blockHeight - _confirmingTxs[hash]->getBlockHeight() + 1;
			fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Updated),
										 _confirmingTxs[hash]->toJson(), confirm);
			SPDLOG_DEBUG(Log::getLogger(), "Tx callback (onTxUpdated) finished. Details: txHash={}, confirm count={}.",
						 hash, confirm);
		}

		void SubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			SPDLOG_DEBUG(Log::getLogger(), "Tx callback (onTxDeleted) begin");
			fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Deleted),
										 nlohmann::json(), 0);
			SPDLOG_DEBUG(Log::getLogger(), "Tx callback (onTxDeleted) finished.");
		}

		void SubWallet::recover(int limitGap) {
			_walletManager->recover(limitGap);
		}

		Key SubWallet::deriveKey(const std::string &payPassword) {
			CMBlock raw = Utils::decodeHex(_info.getEncryptedKey());
			CMBlock keyData = Utils::decrypt(raw, payPassword);
			if (keyData.GetSize() == 0)
				ErrorCode::StandardLogicError(ErrorCode::PasswordError, "Invalid password.");

			Key key;
			char stmp[keyData.GetSize()];
			memcpy(stmp, keyData, keyData.GetSize());
			std::string secret(stmp, keyData.GetSize());
			key.setPrivKey(secret);
			return key;
		}

		void SubWallet::deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const std::string &payPassword) {
			ParamChecker::checkNullPointer(key);

			UInt512 seed = _parent->deriveSeed(payPassword);
			Key wrapperKey;
			wrapperKey.deriveKeyAndChain(chainCode, &seed, sizeof(seed), 5, 1 | BIP32_HARD, 0,
										 44, _info.getIndex(), 0);
			UInt256Set(&key->secret, wrapperKey.getSecret());
			key->compressed = wrapperKey.getCompressed();
			CMBlock pubKey = wrapperKey.getPubkey();
			memcpy(key->pubKey, pubKey, pubKey.GetSize());
		}

		void SubWallet::signTransaction(const boost::shared_ptr<Transaction> &transaction, int forkId,
										const std::string &payPassword) {
			SPDLOG_DEBUG(Log::getLogger(), "SubWallet signTransaction method begin.");

			ParamChecker::checkNullPointer(transaction.get());
			BRKey masterKey;
			UInt256 chainCode = UINT256_ZERO;
			deriveKeyAndChain(&masterKey, chainCode, payPassword);
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			ParamChecker::checkNullPointer(wallet);
			SPDLOG_DEBUG(Log::getLogger(), "SubWallet signTransaction derive key down.");

			BRTransaction *tx = transaction->getRaw();
			uint32_t j, internalIdx[tx->inCount], externalIdx[tx->inCount];
			size_t i, internalCount = 0, externalCount = 0;


			SPDLOG_DEBUG(Log::getLogger(), "SubWallet signTransaction begin get indices.");
			pthread_mutex_lock(&wallet->lock);
			for (i = 0; i < tx->inCount; i++) {
				if (wallet->internalChain) {
					for (j = (uint32_t) array_count(wallet->internalChain); j > 0; j--) {
						if (BRAddressEq(tx->inputs[i].address, &wallet->internalChain[j - 1])) {
							internalIdx[internalCount++] = j - 1;
						}
					}
				}

				for (j = (uint32_t) array_count(wallet->externalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &wallet->externalChain[j - 1])) {
						externalIdx[externalCount++] = j - 1;
					}

				}
			}
			pthread_mutex_unlock(&wallet->lock);
			SPDLOG_DEBUG(Log::getLogger(), "SubWallet signTransaction end get indices.");

			BRKey keys[internalCount + externalCount];
			Key::calculatePrivateKeyList(keys, internalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_INTERNAL_CHAIN, internalIdx);
			Key::calculatePrivateKeyList(&keys[internalCount], externalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_EXTERNAL_CHAIN, externalIdx);
			SPDLOG_DEBUG(Log::getLogger(), "SubWallet signTransaction calculate private key list done.");

			if (tx) {
				SPDLOG_DEBUG(Log::getLogger(), "SubWallet signTransaction begin sign method.");
				WrapperList<Key, BRKey> keyList;
				for (i = 0; i < internalCount + externalCount; ++i) {
					Key key(keys[i].secret, keys[i].compressed);
					keyList.push_back(key);
				}
				if (!transaction->sign(keyList, forkId)) {
					throw std::logic_error("Transaction Sign error!");
				}
				SPDLOG_DEBUG(Log::getLogger(), "SubWallet signTransaction end sign method.");
			}

			for (i = 0; i < internalCount + externalCount; i++) BRKeyClean(&keys[i]);
		}

		std::string SubWallet::CreateMultiSignAddress(const nlohmann::json &multiPublicKeyJson, uint32_t totalSignNum,
													  uint32_t requiredSignNum) {
			//todo complete me
			return "";
		}

		nlohmann::json SubWallet::CreateMultiSignTransaction(const std::string &fromAddress,
															 const std::string &toAddress, uint64_t amount,
															 const std::string &memo) {
			//todo complete me
			return nlohmann::json();
		}

		nlohmann::json
		SubWallet::SendRawTransaction(const nlohmann::json &transactionJson, uint64_t fee,
									  const std::string &payPassword) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(transactionJson);

			verifyRawTransaction(transaction);
			completeTransaction(transaction, fee);

			return sendTransactionInternal(transaction, payPassword);
		}

		void SubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			TransactionChecker checker(transaction, _walletManager->getWallet());
			checker.Check();
		}

		TransactionPtr SubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			TransactionCompleter completer(transaction, _walletManager->getWallet());
			return completer.Complete(actualFee);
		}

		bool SubWallet::filterByAddressOrTxId(BRTransaction *transaction, const std::string &addressOrTxid) {
			ELATransaction *tx = (ELATransaction *) transaction;

			if (addressOrTxid == "") {
				return true;
			}

			for (size_t i = 0; i < tx->raw.inCount; ++i) {
				BRTxInput *input = &tx->raw.inputs[i];
				std::string addr(input->address);
				if (addr == addressOrTxid) {
					return true;
				}
			}
			for (size_t i = 0; i < tx->outputs.size(); ++i) {
				TransactionOutput *output = tx->outputs[i];
				if (output->getAddress() == addressOrTxid) {
					return true;
				}
			}

			if (addressOrTxid.length() == sizeof(UInt256) * 2) {
				Transaction txn(tx, false);
				UInt256 Txid = Utils::UInt256FromString(addressOrTxid, true);
				if (UInt256Eq(&Txid, &tx->raw.txHash)) {
					return true;
				}
			}

			return false;
		}

		void SubWallet::syncStarted() {
			_syncStartHeight = _walletManager->getPeerManager()->getSyncStartHeight();

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStarted();
						  });
		}

		void SubWallet::syncStopped(const std::string &error) {
			_syncStartHeight = 0;

			if (!error.empty()) {
				Log::error(error);
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStopped();
						  });
		}

		void SubWallet::saveBlocks(bool replace, const SharedWrapperList<IMerkleBlock, BRMerkleBlock *> &blocks) {
			SPDLOG_TRACE(Log::getLogger(), "Saving blocks: block count = {}, chain id = {}", blocks.size(),
						 _info.getChainId());
		}

		void SubWallet::blockHeightIncreased(uint32_t blockHeight) {
			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end(); ++it) {
				SPDLOG_DEBUG(Log::getLogger(), "Transaction height increased: txHash = {}, confirms = {}",
							 it->first, blockHeight - it->second->getBlockHeight());
				fireTransactionStatusChanged(it->first, SubWalletCallback::convertToString(SubWalletCallback::Updated),
											 it->second->toJson(), blockHeight - it->second->getBlockHeight());
			}

			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end();) {
				if (blockHeight - it->second->getBlockHeight() >= 6)
					_confirmingTxs.erase(it++);
				else
					++it;
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [blockHeight, this](ISubWalletCallback *callback) {
							  callback->OnBlockHeightIncreased(
									  blockHeight, _walletManager->getPeerManager()->getSyncProgress(_syncStartHeight));
						  });
		}

		void SubWallet::fireTransactionStatusChanged(const std::string &txid, const std::string &status,
													 const nlohmann::json &desc, uint32_t confirms) {
			std::string reversedId(txid.rbegin(), txid.rend());
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&reversedId, &status, &desc, confirms](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(reversedId, status, desc, confirms);
						  });
		}

		nlohmann::json SubWallet::readPeerConfig() {
			//fixme read from main chain(ela)
			nlohmann::json idChainPeerConfig =
					R"(
						  {
							"MagicNumber": 201806271,
							"KnowingPeers":
							[
								{
									"Address": "18.208.17.78",
									"Port": 22608,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								},
								{
									"Address": "18.208.238.27",
									"Port": 22608,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								},
								{
									"Address": "18.208.78.1",
									"Port": 22608,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								},
								{
									"Address": "18.208.99.72",
									"Port": 22608,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								},
								{
									"Address": "34.193.157.94",
									"Port": 22608,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								}
							]
						}
					)"_json;
			return idChainPeerConfig;
		}

	}
}
