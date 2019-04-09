// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoreSpvService.h"

#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Common/Log.h>
#include <SDK/Account/SingleSubAccount.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		CoreSpvService::CoreSpvService(const PluginType &pluginTypes, const ChainParams &chainParams) :
				PeerManager::Listener(pluginTypes),
				_wallet(nullptr),
				_walletListener(nullptr),
				_peerManager(nullptr),
				_peerManagerListener(nullptr),
				_subAccount(nullptr),
				_pluginTypes(pluginTypes),
				_chainParams(chainParams) {
		}

		CoreSpvService::~CoreSpvService() {

		}

		void CoreSpvService::init(const SubAccountPtr &subAccount, uint32_t earliestPeerTime, uint32_t reconnectSeconds) {
			_subAccount = subAccount;
			_earliestPeerTime = earliestPeerTime;
			_reconnectSeconds = reconnectSeconds;

			std::vector<TransactionPtr>  txs = loadTransactions();

			if (_wallet == nullptr) {
				_wallet = WalletPtr(new TransactionHub(loadAssets(), txs, _subAccount, createWalletListener()));
			}

			if (_peerManager == nullptr) {
				_peerManager = PeerManagerPtr(new PeerManager(
						_chainParams,
						getWallet(),
						_earliestPeerTime,
						_reconnectSeconds,
						loadBlocks(),
						loadPeers(),
						createPeerManagerListener(),
						_pluginTypes));
			}

			for (size_t i = 0; i < txs.size(); ++i) {
				if (txs[i]->GetBlockHeight() == TX_UNCONFIRMED) {
					_peerManager->PublishTransaction(txs[i]);
				}
			}
		}

		const WalletPtr &CoreSpvService::getWallet() {
			return _wallet;
		}

		const PeerManagerPtr &CoreSpvService::getPeerManager() {
			return _peerManager;
		}

		void CoreSpvService::balanceChanged(const uint256 &asset, uint64_t balance) {

		}

		void CoreSpvService::onTxAdded(const TransactionPtr &transaction) {

		}

		void CoreSpvService::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {

		}

		void CoreSpvService::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {

		}

		void CoreSpvService::syncStarted() {

		}

		void CoreSpvService::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight) {

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

		void CoreSpvService::blockHeightIncreased(uint32_t blockHeight) {

		}

		std::vector<TransactionPtr> CoreSpvService::loadTransactions() {
			//todo complete me
			return std::vector<TransactionPtr>();
		}

		std::vector<MerkleBlockPtr> CoreSpvService::loadBlocks() {
			//todo complete me
			return std::vector<MerkleBlockPtr>();
		}

		std::vector<PeerInfo> CoreSpvService::loadPeers() {
			//todo complete me
			return std::vector<PeerInfo>();
		}

		std::vector<Asset> CoreSpvService::loadAssets() {
			// todo complete me
			return std::vector<Asset>();
		}

		int CoreSpvService::getForkId() const {
			//todo complete me
			return -1;
		}

		const CoreSpvService::PeerManagerListenerPtr &CoreSpvService::createPeerManagerListener() {
			if (_peerManagerListener == nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(
						new WrappedExceptionPeerManagerListener(this, _pluginTypes));
			}
			return _peerManagerListener;
		}

		const CoreSpvService::WalletListenerPtr &CoreSpvService::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExceptionTransactionHubListener(this));
			}
			return _walletListener;
		}

		WrappedExceptionPeerManagerListener::WrappedExceptionPeerManagerListener(PeerManager::Listener *listener,
																				 const PluginType &pluginTypes) :
				PeerManager::Listener(pluginTypes),
				_listener(listener) {
		}

		void WrappedExceptionPeerManagerListener::syncStarted() {
			try {
				_listener->syncStarted();
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (syncStarted) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (syncStarted) error.");
			}
		}

		void WrappedExceptionPeerManagerListener::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight) {
			try {
				_listener->syncProgress(currentHeight, estimatedHeight);
			} catch (std::exception ex) {
				Log::getLogger()->error("Peer manager callback (syncProgress) error: {}", ex.what());
			} catch (...) {
				Log::getLogger()->error("Peer manager callback (syncProgress) error");
			}
		}

		void WrappedExceptionPeerManagerListener::syncStopped(const std::string &error) {
			try {
				_listener->syncStopped(error);
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (syncStopped) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (syncStopped) error.");
			}
		}

		void WrappedExceptionPeerManagerListener::txStatusUpdate() {
			try {
				_listener->txStatusUpdate();
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (txStatusUpdate) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (txStatusUpdate) error.");
			}
		}

		void WrappedExceptionPeerManagerListener::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

			try {
				_listener->saveBlocks(replace, blocks);
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (saveBlocks) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (saveBlocks) error.");
			}
		}

		void
		WrappedExceptionPeerManagerListener::savePeers(bool replace, const std::vector<PeerInfo> &peers) {

			try {
				_listener->savePeers(replace, peers);
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (savePeers) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (savePeers) error.");
			}
		}

		bool WrappedExceptionPeerManagerListener::networkIsReachable() {
			try {
				return _listener->networkIsReachable();
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (networkIsReachable) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (networkIsReachable) error.");
			}

			return true;
		}

		void WrappedExceptionPeerManagerListener::txPublished(const std::string &hash, const nlohmann::json &result) {
			try {
				_listener->txPublished(hash, result);
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (txPublished) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (txPublished) error.");
			}
		}

		void WrappedExceptionPeerManagerListener::blockHeightIncreased(uint32_t blockHeight) {
			try {
				_listener->blockHeightIncreased(blockHeight);
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (blockHeightIncreased) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (blockHeightIncreased) error.");
			}
		}

		void WrappedExceptionPeerManagerListener::syncIsInactive(uint32_t time) {
			try {
				_listener->syncIsInactive(time);
			}
			catch (std::exception ex) {
				Log::error("Peer manager callback (blockHeightIncreased) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Peer manager callback (blockHeightIncreased) error.");
			}
		}

		WrappedExecutorPeerManagerListener::WrappedExecutorPeerManagerListener(
				PeerManager::Listener *listener,
				Executor *executor,
				Executor *reconnectExecutor,
				const PluginType &pluginType) :
				PeerManager::Listener(pluginType),
				_listener(listener),
				_executor(executor),
				_reconnectExecutor(reconnectExecutor) {
		}

		void WrappedExecutorPeerManagerListener::syncStarted() {
			_executor->Execute(Runnable([this]() -> void {
				try {
					_listener->syncStarted();
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (syncStarted) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (syncStarted) error.");
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight) {
			_executor->Execute(Runnable([this, currentHeight, estimatedHeight]() -> void {
				try {
					_listener->syncProgress(currentHeight, estimatedHeight);
				} catch (std::exception ex) {
					Log::getLogger()->error("Peer manager callback (syncProgress) error: {}", ex.what());
				} catch (...) {
					Log::getLogger()->error("Peer manager callback (syncProgress) error");
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::syncStopped(const std::string &error) {
			_executor->Execute(Runnable([this, error]() -> void {
				try {
					_listener->syncStopped(error);
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (syncStopped) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (syncStopped) error.");
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::txStatusUpdate() {
			_executor->Execute(Runnable([this]() -> void {
				try {
					_listener->txStatusUpdate();
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (txStatusUpdate) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (txStatusUpdate) error.");
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			_executor->Execute(Runnable([this, replace, blocks]() -> void {
				try {
					_listener->saveBlocks(replace, blocks);
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (saveBlocks) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (saveBlocks) error.");
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::savePeers(bool replace, const std::vector<PeerInfo> &peers) {
			_executor->Execute(Runnable([this, replace, peers]() -> void {
				try {
					_listener->savePeers(replace, peers);
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (savePeers) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (savePeers) error.");
				}
			}));
		}

		bool WrappedExecutorPeerManagerListener::networkIsReachable() {

			bool result = true;
			_executor->Execute(Runnable([this, result]() -> void {
				try {
					_listener->networkIsReachable();
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (networkIsReachable) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (networkIsReachable) error.");
				}
			}));
			return result;
		}

		void WrappedExecutorPeerManagerListener::txPublished(const std::string &hash, const nlohmann::json &result) {

			_executor->Execute(Runnable([this, hash, result]() -> void {
				try {
					_listener->txPublished(hash, result);
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (txPublished) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (txPublished) error.");
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::blockHeightIncreased(uint32_t blockHeight) {
			_executor->Execute(Runnable([this, blockHeight]() -> void {
				try {
					if (_listener)
						_listener->blockHeightIncreased(blockHeight);
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (blockHeightIncreased) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (blockHeightIncreased) error.");
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::syncIsInactive(uint32_t time) {
			_reconnectExecutor->Execute(Runnable([this, time]() -> void {
				try {
					_listener->syncIsInactive(time);
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (blockHeightIncreased) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (blockHeightIncreased) error.");
				}
			}));
		}

		WrappedExceptionTransactionHubListener::WrappedExceptionTransactionHubListener(AssetTransactions::Listener *listener) :
				_listener(listener) {
		}

		void WrappedExceptionTransactionHubListener::balanceChanged(const uint256 &asset, uint64_t balance) {
			try {
				_listener->balanceChanged(asset, balance);
			}
			catch (std::exception ex) {
				Log::error("Wallet callback (balanceChanged) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Wallet callback (balanceChanged) error.");
			}
		}

		void WrappedExceptionTransactionHubListener::onTxAdded(const TransactionPtr &transaction) {
			try {
				return _listener->onTxAdded(transaction);
			}
			catch (std::exception ex) {
				Log::error("Wallet callback (onTxAdded) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Wallet callback (onTxAdded) error.");
			}
		}

		void WrappedExceptionTransactionHubListener::onTxUpdated(
				const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {

			try {
				_listener->onTxUpdated(hash, blockHeight, timeStamp);
			}
			catch (std::exception ex) {
				Log::error("Wallet callback (onTxUpdated) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Wallet callback (onTxUpdated) error.");
			}
		}

		void WrappedExceptionTransactionHubListener::onTxDeleted(
				const std::string &hash, const std::string &assetID, bool notifyUser, bool recommendRescan) {

			try {
				_listener->onTxDeleted(hash, assetID, notifyUser, recommendRescan);
			}
			catch (std::exception ex) {
				Log::error("Wallet callback (onTxDeleted) error: {}", ex.what());
			}
			catch (...) {
				Log::error("Wallet callback (onTxDeleted) error.");
			}
		}

		WrappedExecutorTransactionHubListener::WrappedExecutorTransactionHubListener(
				AssetTransactions::Listener *listener,
				Executor *executor) :
				_listener(listener),
				_executor(executor) {
		}

		void WrappedExecutorTransactionHubListener::balanceChanged(const uint256 &asset, uint64_t balance) {
			_executor->Execute(Runnable([this, asset, balance]() -> void {
				try {
					_listener->balanceChanged(asset, balance);
				}
				catch (std::exception ex) {
					Log::error("Wallet callback (balanceChanged) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Wallet callback (balanceChanged) error.");
				}
			}));
		}

		void WrappedExecutorTransactionHubListener::onTxAdded(const TransactionPtr &transaction) {
			_executor->Execute(Runnable([this, transaction]() -> void {
				try {
					_listener->onTxAdded(transaction);
				}
				catch (std::exception ex) {
					Log::error("Wallet callback (onTxAdded) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Wallet callback (onTxAdded) error.");
				}
			}));
		}

		void WrappedExecutorTransactionHubListener::onTxUpdated(
				const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			_executor->Execute(Runnable([this, hash, blockHeight, timeStamp]() -> void {
				try {
					_listener->onTxUpdated(hash, blockHeight, timeStamp);
				}
				catch (std::exception ex) {
					Log::error("Wallet callback (onTxUpdated) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Wallet callback (onTxUpdated) error.");
				}
			}));
		}

		void WrappedExecutorTransactionHubListener::onTxDeleted(
				const std::string &hash, const std::string &assetID, bool notifyUser, bool recommendRescan) {
			_executor->Execute(Runnable([this, hash, assetID, notifyUser, recommendRescan]() -> void {
				try {
					_listener->onTxDeleted(hash, assetID, notifyUser, recommendRescan);
				}
				catch (std::exception ex) {
					Log::error("Wallet callback (onTxDeleted) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Wallet callback (onTxDeleted) error.");
				}
			}));
		}
	}
}