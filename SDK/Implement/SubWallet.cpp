// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubWalletCallback.h"
#include "MasterWallet.h"
#include "SubWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/Checker/TransactionChecker.h>
#include <SDK/Account/MultiSignSubAccount.h>
#include <SDK/Account/SubAccountGenerator.h>

#include <Core/BRTransaction.h>
#include <Core/BRTransaction.h>
#include <Core/BRKey.h>
#include <Core/BRArray.h>

#include <algorithm>
#include <boost/scoped_ptr.hpp>

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const PluginType &pluginTypes,
							 MasterWallet *parent) :
				PeerManager::Listener(pluginTypes),
				_parent(parent),
				_info(info),
				_syncStartHeight(0) {

			fs::path subWalletDbPath = _parent->_rootPath;
			subWalletDbPath /= parent->GetId();
			subWalletDbPath /= info.getChainId() + DB_FILE_EXTENSION;

			{
				SubAccountGenerator generator;
				generator.SetCoinInfo(_info);
				generator.SetParentAccount(_parent->_localStore.Account());
				generator.SetMasterPubKey(_parent->GetMasterPubKey(_info.getChainId()));
				_subAccount = SubAccountPtr(generator.Generate());

				_info.setChainCode(Utils::UInt256ToString(generator.GetResultChainCode()));
				if (generator.GetResultPublicKey().GetSize() > 0)
					_info.setPublicKey(Utils::encodeHex(generator.GetResultPublicKey()));
			}

			_walletManager = WalletManagerPtr(
					new SpvService(_subAccount, subWalletDbPath,
									  _info.getEarliestPeerTime(), _info.getReconnectSeconds(),
									  _info.getForkId(), pluginTypes,
									  chainParams));

			_walletManager->registerWalletListener(this);
			_walletManager->registerPeerManagerListener(this);

			WalletPtr wallet = _walletManager->getWallet();
			if (info.getFeePerKb() > 0) {
				wallet->setFeePerKb(Asset::GetELAAssetID(), info.getFeePerKb());
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
			return _walletManager->getWallet()->GetBalanceInfo(Asset::GetELAAssetID());
		}

		uint64_t SubWallet::GetBalance(BalanceType type) {
			return _walletManager->getWallet()->getBalance(Asset::GetELAAssetID(), AssetTransactions::BalanceType(type));
		}

		std::string SubWallet::CreateAddress() {
			return _walletManager->getWallet()->getReceiveAddress();
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start,
												uint32_t count) {
			nlohmann::json j;
			std::vector<std::string> addresses = _walletManager->getWallet()->getAllAddresses();
			if (start >= addresses.size()) {
				j["Addresses"] = {};
				j["MaxCount"] = addresses.size();
			} else {
				uint32_t end = std::min(start + count, (uint32_t) addresses.size());
				std::vector<std::string> results(addresses.begin() + start, addresses.begin() + end);
				j["Addresses"] = results;
				j["MaxCount"] = addresses.size();
			}
			return j;
		}

		uint64_t SubWallet::GetBalanceWithAddress(const std::string &address) {
			return _walletManager->getWallet()->GetBalanceWithAddress(Asset::GetELAAssetID(), address);
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			if (std::find(_callbacks.begin(), _callbacks.end(), subCallback) != _callbacks.end())
				return;
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		TransactionPtr SubWallet::CreateTx(const std::string &fromAddress, const std::string &toAddress,
													uint64_t amount, const UInt256 &assetID, const std::string &memo,
													const std::string &remark, bool useVotedUTXO) const {
			WalletPtr wallet = _walletManager->getWallet();
			TransactionPtr tx = wallet->createTransaction(fromAddress, amount, toAddress, _info.getMinFee(),
														  assetID, memo, remark, useVotedUTXO);

			ParamChecker::checkLogic(tx == nullptr, Error::CreateTransaction, "create tx error.");

			return tx;
		};

		nlohmann::json SubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
													uint64_t amount, const std::string &memo,
													const std::string &remark, bool useVotedUTXO) {
			TransactionPtr tx = CreateTx(fromAddress, toAddress, amount, Asset::GetELAAssetID(), memo, remark, useVotedUTXO);
			return tx->toJson();
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			const WalletPtr &wallet = _walletManager->getWallet();

			std::vector<TransactionPtr> allTxs = wallet->getAllTransactions();
			size_t fullTxCount = allTxs.size();
			size_t pageCount = count;
			nlohmann::json j;

			if (start >= fullTxCount) {
				j["Transactions"] = {};
				j["MaxCount"] = fullTxCount;
				return j;
			}

			if (fullTxCount < start + count)
				pageCount = fullTxCount - start;

			std::vector<TransactionPtr> transactions(pageCount);
			uint32_t realCount = 0;
			for (long i = fullTxCount - 1 - start; i >= 0 && realCount < pageCount; --i) {
				if (!filterByAddressOrTxId(allTxs[i], addressOrTxid))
					continue;
				transactions[realCount++] = allTxs[i];
			}

			std::vector<nlohmann::json> jsonList(realCount);
			for (size_t i = 0; i < realCount; ++i) {
				uint32_t confirms = 0;
				uint32_t txBlockHeight = transactions[i]->getBlockHeight();
				uint32_t lastBlockHeight = _walletManager->getPeerManager()->GetLastBlockHeight();

				if (txBlockHeight != TX_UNCONFIRMED) {
					confirms = lastBlockHeight >= txBlockHeight ? lastBlockHeight - txBlockHeight + 1 : 0;
				}

				jsonList[i] = transactions[i]->GetSummary(_walletManager->getWallet(), confirms, !addressOrTxid.empty());
			}
			j["Transactions"] = jsonList;
			j["MaxCount"] = fullTxCount;
			return j;
		}

		void SubWallet::publishTransaction(const TransactionPtr &transaction) {
			_walletManager->publishTransaction(transaction);
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {

			Key key = _subAccount->DeriveMainAccountKey(payPassword);
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
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&balance](ISubWalletCallback *callback) {
							  callback->OnBalanceChanged(balance);
						  });
		}

		void SubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction == nullptr)
				return;

			std::string txHash = Utils::UInt256ToString(transaction->getHash(), true);
			_confirmingTxs[txHash] = transaction;

			fireTransactionStatusChanged(txHash, SubWalletCallback::convertToString(SubWalletCallback::Added),
										 transaction->toJson(), 0);
		}

		void SubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			if (_walletManager->getAllTransactionsCount() == 1) {
				_info.setEaliestPeerTime(timeStamp);
				_parent->Save();
			}

			if (_confirmingTxs.find(hash) == _confirmingTxs.end()) {
				_confirmingTxs[hash] = _walletManager->getWallet()->transactionForHash(Utils::UInt256FromString(hash, true));
			}

			uint32_t txBlockHeight = _confirmingTxs[hash]->getBlockHeight();
			uint32_t confirm = txBlockHeight != TX_UNCONFIRMED &&
								blockHeight >= txBlockHeight ? blockHeight - txBlockHeight + 1 : 0;
			if (_walletManager->getPeerManager()->SyncSucceeded()) {
				fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Updated),
											 _confirmingTxs[hash]->toJson(), confirm);
			}
		}

		void SubWallet::onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser,
									bool recommendRescan) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, &assetID, &notifyUser, &recommendRescan](ISubWalletCallback *callback) {
				callback->OnTxDeleted(hash, notifyUser, recommendRescan);
			});
		}

		void SubWallet::recover(int limitGap) {
			_walletManager->recover(limitGap);
		}

		nlohmann::json SubWallet::CreateMultiSignTransaction(const std::string &fromAddress,
															 const std::string &toAddress, uint64_t amount,
															 const std::string &memo, bool useVotedUTXO) {
			return CreateTransaction(fromAddress, toAddress, amount, memo, "", useVotedUTXO);
		}

		nlohmann::json SubWallet::SignTransaction(const nlohmann::json &rawTransaction,
												  const std::string &payPassword) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(rawTransaction);
			_walletManager->getWallet()->signTransaction(transaction, payPassword);
			transaction->removeDuplicatePrograms();
			return transaction->toJson();
		}

		nlohmann::json SubWallet::PublishTransaction(const nlohmann::json &rawTransaction) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(rawTransaction);

			verifyRawTransaction(transaction);
			publishTransaction(transaction);

			nlohmann::json j;
			j["TxHash"] = Utils::UInt256ToString(transaction->getHash(), true);
			j["Fee"] = transaction->getFee();
			return j;
		}

		nlohmann::json
		SubWallet::UpdateTransactionFee(const nlohmann::json &transactionJson, uint64_t fee,
										const std::string &fromAddress, bool useVotedUTXO) {
			if (fee <= _info.getMinFee()) {
				return transactionJson;
			}

			TransactionPtr tx(new Transaction());
			tx->fromJson(transactionJson);

			WalletPtr wallet = _walletManager->getWallet();
			wallet->UpdateTxFee(tx, fee, fromAddress, useVotedUTXO);

			return tx->toJson();
		}

		void SubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			TransactionChecker checker(transaction, _walletManager->getWallet());
			checker.Check();
		}

		bool SubWallet::filterByAddressOrTxId(const TransactionPtr &transaction, const std::string &addressOrTxid) {

			if (addressOrTxid.empty()) {
				return true;
			}

			const WalletPtr &wallet = _walletManager->getWallet();
			for (size_t i = 0; i < transaction->getInputs().size(); ++i) {
				const TransactionPtr &tx = wallet->transactionForHash(transaction->getInputs()[i].getTransctionHash());
				if (tx->getOutputs()[transaction->getInputs()[i].getIndex()].getAddress() == addressOrTxid) {
					return true;
				}
			}
			for (size_t i = 0; i < transaction->getOutputs().size(); ++i) {
				if (transaction->getOutputs()[i].getAddress() == addressOrTxid) {
					return true;
				}
			}

			if (addressOrTxid.length() == sizeof(UInt256) * 2) {
				UInt256 Txid = Utils::UInt256FromString(addressOrTxid, true);
				if (UInt256Eq(&Txid, &transaction->getHash())) {
					return true;
				}
			}

			return false;
		}

		void SubWallet::syncStarted() {
			_syncStartHeight = _walletManager->getPeerManager()->getSyncStartHeight();
			if (_info.getEarliestPeerTime() == 0) {
				_info.setEaliestPeerTime(time(nullptr));
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStarted();
						  });
		}

		void SubWallet::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&currentHeight, &estimatedHeight](ISubWalletCallback *callback) {
							  callback->OnBlockSyncProgress(currentHeight, estimatedHeight);
						  });
		}

		void SubWallet::syncStopped(const std::string &error) {
			_syncStartHeight = 0;

			if (!error.empty()) {
				Log::error("syncStopped with error: {}", error);
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStopped();
						  });
		}

		void SubWallet::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			Log::debug("[{}] Save blocks count={}", _info.getChainId(), blocks.size());
		}

		void SubWallet::txPublished(const std::string &hash, const nlohmann::json &result) {
			std::for_each(_callbacks.begin(), _callbacks.end(), [&hash, &result](ISubWalletCallback *callback) {
				callback->OnTxPublished(hash, result);
			});
		}

		void SubWallet::blockHeightIncreased(uint32_t blockHeight) {
			if (_walletManager->getPeerManager()->SyncSucceeded()) {
				for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end(); ++it) {
					uint32_t txBlockHeight = it->second->getBlockHeight();
					uint32_t confirms = txBlockHeight != TX_UNCONFIRMED &&
										blockHeight >= txBlockHeight ? blockHeight - txBlockHeight + 1 : 0;

					if (confirms > 1) {
						fireTransactionStatusChanged(it->first, SubWalletCallback::convertToString(SubWalletCallback::Updated),
													 it->second->toJson(), confirms);
					}
				}
			}

			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end();) {
				uint32_t txBlockHeight = it->second->getBlockHeight();
				uint32_t confirms = txBlockHeight != TX_UNCONFIRMED &&
									blockHeight >= txBlockHeight ? blockHeight - txBlockHeight + 1 : 0;

				if (confirms >= 6)
					_confirmingTxs.erase(it++);
				else
					++it;
			}
		}

		void SubWallet::fireTransactionStatusChanged(const std::string &txid, const std::string &status,
													 const nlohmann::json &desc, uint32_t confirms) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&txid, &status, &desc, confirms](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(txid, status, desc, confirms);
						  });
		}

		const CoinInfo &SubWallet::getCoinInfo() {
			_info.setFeePerKb(_walletManager->getWallet()->getFeePerKb(Asset::GetELAAssetID()));
			return _info;
		}

		void SubWallet::StartP2P() {
			if (_info.getEnableP2P())
				_walletManager->start();
		}

		void SubWallet::StopP2P() {
			if (_info.getEnableP2P())
				_walletManager->stop();
		}

		std::string SubWallet::GetPublicKey() const {
			return _subAccount->GetMainAccountPublicKey();
		}

		nlohmann::json SubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Normal";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

		nlohmann::json SubWallet::GetTransactionSignedSigners(const nlohmann::json &rawTransaction) {
			nlohmann::json result;
			MultiSignSubAccount *subAccount = dynamic_cast<MultiSignSubAccount *>(_subAccount.get());
			if (subAccount != nullptr) {
				TransactionPtr transaction(new Transaction);
				transaction->fromJson(rawTransaction);
				result["Signers"] = subAccount->GetTransactionSignedSigners(transaction);
			}
			return result;
		}

		nlohmann::json SubWallet::GetAssetDetails(const std::string &assetID) const {
			Asset asset = _walletManager->FindAsset(assetID);
			return asset.toJson();
		}

	}
}
