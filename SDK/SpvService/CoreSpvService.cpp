// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoreSpvService.h"

#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/SpvService/Config.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		CoreSpvService::CoreSpvService() {

		}

		void CoreSpvService::Init(const std::string &walletID,
								  const std::string &chainID,
								  const SubAccountPtr &subAccount,
								  time_t earliestPeerTime,
								  const ChainConfigPtr &config) {

			if (chainID != CHAINID_MAINCHAIN &&
				chainID != CHAINID_IDCHAIN &&
				chainID != CHAINID_TOKENCHAIN) {
				ErrorChecker::ThrowParamException(Error::InvalidChainID, "invalid chain ID");
			}

			std::vector<TransactionPtr>  txs = loadTransactions(chainID);
			std::vector<UTXOPtr> cbs = loadCoinBaseUTXOs();

			if (_peerManager == nullptr) {
				_peerManager = PeerManagerPtr(new PeerManager(
						config->ChainParameters(),
						nullptr,
						earliestPeerTime,
						config->DisconnectionTime(),
						loadBlocks(chainID),
						loadPeers(),
						createPeerManagerListener(),
						chainID));
			}

			if (_wallet == nullptr) {
				_wallet = WalletPtr(new Wallet(_peerManager->GetLastBlockHeight(), walletID + ":" + chainID,
											   loadAssets(), txs, cbs, subAccount, createWalletListener()));
				_peerManager->SetWallet(_wallet);
			}
		}

		CoreSpvService::~CoreSpvService() {

		}

		const WalletPtr &CoreSpvService::GetWallet() const {
			return _wallet;
		}

		const PeerManagerPtr &CoreSpvService::GetPeerManager() const {
			return _peerManager;
		}

		void CoreSpvService::balanceChanged(const uint256 &asset, const BigInt &balance) {

		}

		void CoreSpvService::onCoinBaseTxAdded(const UTXOPtr &cb) {

		}

		void CoreSpvService::onCoinBaseUpdatedAll(const UTXOArray &cbs) {

		}

		void CoreSpvService::onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight,
												 time_t timestamp) {

		}

		void CoreSpvService::onCoinBaseSpent(const std::vector<uint256> &spentHashes) {

		}

		void CoreSpvService::onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {

		}

		void CoreSpvService::onTxAdded(const TransactionPtr &transaction) {

		}

		void CoreSpvService::onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp) {

		}

		void CoreSpvService::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {

		}

		void CoreSpvService::onTxUpdatedAll(const std::vector<TransactionPtr> &txns) {

		}

		void CoreSpvService::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {

		}

		void CoreSpvService::syncStarted() {

		}

		void CoreSpvService::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime) {

		}

		void CoreSpvService::syncStopped(const std::string &error) {

		}

		void CoreSpvService::txStatusUpdate() {

		}

		void
		CoreSpvService::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

		}

		void CoreSpvService::savePeers(bool replace, const std::vector<PeerInfo> &peers) {

		}

		bool CoreSpvService::networkIsReachable() {
			return true;
		}

		void CoreSpvService::txPublished(const std::string &hash, const nlohmann::json &result) {

		}

		void CoreSpvService::connectStatusChanged(const std::string &status) {

		}

		std::vector<UTXOPtr> CoreSpvService::loadCoinBaseUTXOs() {
			return std::vector<UTXOPtr>();
		}

		std::vector<TransactionPtr> CoreSpvService::loadTransactions(const std::string &chainID) {
			//todo complete me
			return std::vector<TransactionPtr>();
		}

		std::vector<MerkleBlockPtr> CoreSpvService::loadBlocks(const std::string &chainID) {
			//todo complete me
			return std::vector<MerkleBlockPtr>();
		}

		std::vector<PeerInfo> CoreSpvService::loadPeers() {
			//todo complete me
			return std::vector<PeerInfo>();
		}

		std::vector<AssetPtr> CoreSpvService::loadAssets() {
			// todo complete me
			return std::vector<AssetPtr>();
		}

		const CoreSpvService::PeerManagerListenerPtr &CoreSpvService::createPeerManagerListener() {
			if (_peerManagerListener == nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(
						new WrappedExceptionPeerManagerListener(this));
			}
			return _peerManagerListener;
		}

		const CoreSpvService::WalletListenerPtr &CoreSpvService::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExceptionWalletListener(this));
			}
			return _walletListener;
		}

		WrappedExceptionPeerManagerListener::WrappedExceptionPeerManagerListener(PeerManager::Listener *listener) :
				_listener(listener) {
		}

		void WrappedExceptionPeerManagerListener::syncStarted() {
			try {
				_listener->syncStarted();
			} catch (const std::exception &ex) {
				Log::error("syncStarted exception: {}", ex.what());
			}
		}

		void WrappedExceptionPeerManagerListener::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight,
															   time_t lastBlockTime) {
			try {
				_listener->syncProgress(currentHeight, estimatedHeight, lastBlockTime);
			} catch (const std::exception &e) {
				Log::error("syncProgress exception: {}", e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::syncStopped(const std::string &error) {
			try {
				_listener->syncStopped(error);
			} catch (const std::exception &e) {
				Log::error("syncStopped exception: {}", e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::txStatusUpdate() {
			try {
				_listener->txStatusUpdate();
			} catch (const std::exception &e) {
				Log::error("txStatusUpdate exception: {}", e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

			try {
				_listener->saveBlocks(replace, blocks);
			} catch (const std::exception &e) {
				Log::error("saveBlocks exception: {}", e.what());
			}
		}

		void
		WrappedExceptionPeerManagerListener::savePeers(bool replace, const std::vector<PeerInfo> &peers) {

			try {
				_listener->savePeers(replace, peers);
			} catch (const std::exception &e) {
				Log::error("savePeers exception: {}", e.what());
			}
		}

		bool WrappedExceptionPeerManagerListener::networkIsReachable() {
			try {
				return _listener->networkIsReachable();
			} catch (const std::exception &e) {
				Log::error("networkIsReachable exception: {}", e.what());
			}

			return true;
		}

		void WrappedExceptionPeerManagerListener::txPublished(const std::string &hash, const nlohmann::json &result) {
			try {
				_listener->txPublished(hash, result);
			} catch (const std::exception &e) {
				Log::error("txPublished exception: {}", e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::connectStatusChanged(const std::string &status) {
			try {
				_listener->connectStatusChanged(status);
			} catch (const std::exception &e) {
				Log::error("connectStatusChanged exception: {}", e.what());
			}
		}

		WrappedExecutorPeerManagerListener::WrappedExecutorPeerManagerListener(
				PeerManager::Listener *listener,
				Executor *executor) :
				_listener(listener),
				_executor(executor) {
		}

		void WrappedExecutorPeerManagerListener::syncStarted() {
			_executor->Execute(Runnable([this]() -> void {
				try {
					_listener->syncStarted();
				} catch (const std::exception &e) {
					Log::error("syncStarted exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight,
															  time_t lastBlockTime) {
			_executor->Execute(Runnable([this, currentHeight, estimatedHeight, lastBlockTime]() -> void {
				try {
					_listener->syncProgress(currentHeight, estimatedHeight, lastBlockTime);
				} catch (const std::exception &e) {
					Log::error("syncProgress exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::syncStopped(const std::string &error) {
			_executor->Execute(Runnable([this, error]() -> void {
				try {
					_listener->syncStopped(error);
				} catch (const std::exception &e) {
					Log::error("syncStopped exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::txStatusUpdate() {
			_executor->Execute(Runnable([this]() -> void {
				try {
					_listener->txStatusUpdate();
				} catch (const std::exception &e) {
					Log::error("txStatusUpdate exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			_executor->Execute(Runnable([this, replace, blocks]() -> void {
				try {
					_listener->saveBlocks(replace, blocks);
				} catch (const std::exception &e) {
					Log::error("saveBlocks exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::savePeers(bool replace, const std::vector<PeerInfo> &peers) {
			_executor->Execute(Runnable([this, replace, peers]() -> void {
				try {
					_listener->savePeers(replace, peers);
				} catch (const std::exception &e) {
					Log::error("savePeers exception: {}", e.what());
				}
			}));
		}

		bool WrappedExecutorPeerManagerListener::networkIsReachable() {

			bool result = true;
			_executor->Execute(Runnable([this, result]() -> void {
				try {
					_listener->networkIsReachable();
				} catch (const std::exception &e) {
					Log::error("networkIsReachable exception: {}", e.what());
				}
			}));
			return result;
		}

		void WrappedExecutorPeerManagerListener::txPublished(const std::string &hash, const nlohmann::json &result) {

			_executor->Execute(Runnable([this, hash, result]() -> void {
				try {
					_listener->txPublished(hash, result);
				} catch (const std::exception &e) {
					Log::error("txPublished exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::connectStatusChanged(const std::string &status) {
			_executor->Execute(Runnable([this, status]() -> void {
				try {
					_listener->connectStatusChanged(status);
				} catch (const std::exception &e) {
					Log::error("connectStatusChanged exception: {}", e.what());
				}
			}));
		}

		WrappedExceptionWalletListener::WrappedExceptionWalletListener(Wallet::Listener *listener) :
				_listener(listener) {
		}

		void WrappedExceptionWalletListener::balanceChanged(const uint256 &asset, const BigInt &balance) {
			try {
				_listener->balanceChanged(asset, balance);
			} catch (const std::exception &e) {
				Log::error("balanceChanged exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinBaseTxAdded(const UTXOPtr &cb) {
			try {
				_listener->onCoinBaseTxAdded(cb);
			} catch (const std::exception &e) {
				Log::error("onCoinBaseTxAdded exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinBaseUpdatedAll(const UTXOArray &cbs) {
			try {
				_listener->onCoinBaseUpdatedAll(cbs);
			} catch (const std::exception &e) {
				Log::error("onCoinBaseUpdatedAll exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinBaseTxUpdated(const std::vector<uint256> &hashes,
																 uint32_t blockHeight,
																 time_t timestamp) {
			try {
				_listener->onCoinBaseTxUpdated(hashes, blockHeight, timestamp);
			} catch (const std::exception &e) {
				Log::error("onCoinBaseTxUpdated exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinBaseSpent(const std::vector<uint256> &spentHashes) {
			try {
				_listener->onCoinBaseSpent(spentHashes);
			} catch (const std::exception &e) {
				Log::error("onCoinBaseSpent exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser,
																 bool recommendRescan) {
			try {
				_listener->onCoinBaseTxDeleted(hash, notifyUser, recommendRescan);
			} catch (const std::exception &e) {
				Log::error("onCoinBaseTxDeleted exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onTxAdded(const TransactionPtr &transaction) {
			try {
				_listener->onTxAdded(transaction);
			} catch (const std::exception &e) {
				Log::error("onTxAdded exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onTxUpdated(
			const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp) {

			try {
				_listener->onTxUpdated(hashes, blockHeight, timeStamp);
			} catch (const std::exception &e) {
				Log::error("onTxUpdated exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onTxDeleted(const uint256 &hash, bool notifyUser,
																 bool recommendRescan) {
			try {
				_listener->onTxDeleted(hash, notifyUser, recommendRescan);
			} catch (const std::exception &e) {
				Log::error("onTxDeleted exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onTxUpdatedAll(const std::vector<TransactionPtr> &txns) {
			try {
				_listener->onTxUpdatedAll(txns);
			} catch (const std::exception &e) {
				Log::error("onTxUpdatedAll exception: {}", e.what());
			}
		}

		void WrappedExceptionWalletListener::onAssetRegistered(const AssetPtr &asset,
																	   uint64_t amount,
																	   const uint168 &controller) {
			try {
				_listener->onAssetRegistered(asset, amount, controller);
			} catch (const std::exception &e) {
				Log::error("onTxDeleted exception: {}", e.what());
			}
		}

		WrappedExecutorWalletListener::WrappedExecutorWalletListener(
				Wallet::Listener *listener,
				Executor *executor) :
				_listener(listener),
				_executor(executor) {
		}

		void WrappedExecutorWalletListener::balanceChanged(const uint256 &asset, const BigInt &balance) {
			_executor->Execute(Runnable([this, asset, balance]() -> void {
				try {
					_listener->balanceChanged(asset, balance);
				} catch (const std::exception &e) {
					Log::error("balanceChanged exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinBaseTxAdded(const UTXOPtr &cb) {
			_executor->Execute(Runnable([this, cb]() -> void {
				try {
					_listener->onCoinBaseTxAdded(cb);
				} catch (const std::exception &e) {
					Log::error("onCoinBaseTxAdded exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinBaseUpdatedAll(const UTXOArray &cbs) {
			_executor->Execute(Runnable([this, cbs]() -> void {
				try {
					_listener->onCoinBaseUpdatedAll(cbs);
				} catch (const std::exception &e) {
					Log::error("onCoinBaseUpdatedAll exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinBaseTxUpdated(const std::vector<uint256> &hashes,
																uint32_t blockHeight,
																time_t timestamp) {
			_executor->Execute(Runnable([this, hashes, blockHeight, timestamp]() -> void {
				try {
					_listener->onCoinBaseTxUpdated(hashes, blockHeight, timestamp);
				} catch (const std::exception &e) {
					Log::error("onCoinBaseTxUpdated exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinBaseSpent(const std::vector<uint256> &spentHashes) {
			_executor->Execute(Runnable([this, spentHashes]() -> void {
				try {
					_listener->onCoinBaseSpent(spentHashes);
				} catch (const std::exception &e) {
					Log::error("onCoinBaseSpent exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser,
																bool recommendRescan) {
			_executor->Execute(Runnable([this, hash, notifyUser, recommendRescan]() -> void {
				try {
					_listener->onCoinBaseTxDeleted(hash, notifyUser, recommendRescan);
				} catch (const std::exception &e) {
					Log::error("onCoinBaseTxDeleted exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onTxAdded(const TransactionPtr &tx) {
			_executor->Execute(Runnable([this, tx]() -> void {
				try {
					_listener->onTxAdded(tx);
				} catch (const std::exception &e) {
					Log::error("onTxAdded exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onTxUpdated(
			const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp) {
			_executor->Execute(Runnable([this, hashes, blockHeight, timeStamp]() -> void {
				try {
					_listener->onTxUpdated(hashes, blockHeight, timeStamp);
				} catch (const std::exception &e) {
					Log::error("onTxUpdated exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onTxDeleted(
				const uint256 &hash, bool notifyUser, bool recommendRescan) {
			_executor->Execute(Runnable([this, hash, notifyUser, recommendRescan]() -> void {
				try {
					_listener->onTxDeleted(hash, notifyUser, recommendRescan);
				} catch (const std::exception &e) {
					Log::error("onTxDeleted exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onTxUpdatedAll(const std::vector<TransactionPtr> &txns) {
			_executor->Execute(Runnable([this, txns]() -> void {
				try {
					_listener->onTxUpdatedAll(txns);
				} catch (const std::exception &e) {
					Log::error("onTxUpdatedAll exception: {}", e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onAssetRegistered(const AssetPtr &asset,
																	  uint64_t amount,
																	  const uint168 &controller) {
			_executor->Execute(Runnable([this, asset, amount, controller]() -> void {
				try {
					_listener->onAssetRegistered(asset, amount, controller);
				} catch (const std::exception &e) {
					Log::error("onAssetRegistered exception: {}", e.what());
				}
			}));
		}

	}
}