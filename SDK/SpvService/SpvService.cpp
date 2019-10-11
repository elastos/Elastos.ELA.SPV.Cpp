// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "SpvService.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Wallet/UTXO.h>

#include <Core/BRMerkleBlock.h>
#include <Core/BRTransaction.h>

#include <boost/thread.hpp>

#define BACKGROUND_THREAD_COUNT 1

#define ISO_OLD "ela"
#define ISO "ela1"

namespace Elastos {
	namespace ElaWallet {

		SpvService::SpvService(const std::string &walletID,
							   const SubAccountPtr &subAccount,
							   const boost::filesystem::path &dbPath,
							   time_t earliestPeerTime,
							   uint32_t reconnectSeconds,
							   const PluginType &pluginTypes,
							   const ChainParamsPtr &chainParams) :
				CoreSpvService(pluginTypes, chainParams),
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(dbPath) {
			init(walletID, subAccount, earliestPeerTime, reconnectSeconds);
		}

		SpvService::~SpvService() {
			_executor.StopThread();
		}

		void SpvService::SyncStart() {
			_peerManager->SetKeepAliveTimestamp(time(NULL));
			_peerManager->ResetReconnectStep();
			_peerManager->ConnectLaster(0);
		}

		void SpvService::SyncStop() {
			_peerManager->CancelTimer();
			_peerManager->Disconnect();
		}

		void SpvService::ExecutorStop() {
			_executor.StopThread();
		}

		void SpvService::PublishTransaction(const TransactionPtr &tx) {
			if (GetPeerManager()->GetConnectStatus() != Peer::Connected) {
				GetPeerManager()->ConnectLaster(0);
			}

			GetPeerManager()->PublishTransaction(tx);
		}

		void SpvService::DatabaseFlush() {
			_databaseManager.flush();
		}

		//override Wallet listener
		void SpvService::balanceChanged(const uint256 &asset, const BigInt &balance) {
			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&asset, &balance](Wallet::Listener *listener) {
							  listener->balanceChanged(asset, balance);
						  });
		}

		void SpvService::onCoinBaseTxAdded(const UTXOPtr &cb) {

			_databaseManager.PutCoinBase(cb);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&cb](Wallet::Listener *listener) {
							  listener->onCoinBaseTxAdded(cb);
						  });
		}

		void SpvService::onCoinBaseUpdatedAll(const UTXOArray &cbs) {
			_databaseManager.DeleteAllCoinBase();
			_databaseManager.PutCoinBase(cbs);
		}

		void SpvService::onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight,
											 time_t timestamp) {
			_databaseManager.UpdateCoinBase(hashes, blockHeight, timestamp);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hashes, &blockHeight, &timestamp](Wallet::Listener *listener) {
							  listener->onCoinBaseTxUpdated(hashes, blockHeight, timestamp);
						  });
		}

		void SpvService::onCoinBaseSpent(const std::vector<uint256> &spentHashes) {
			_databaseManager.UpdateSpentCoinBase(spentHashes);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&spentHashes](Wallet::Listener *listener) {
							  listener->onCoinBaseSpent(spentHashes);
						  });
		}

		void SpvService::onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			_databaseManager.DeleteCoinBase(hash);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, &notifyUser, &recommendRescan](Wallet::Listener *listener) {
							  listener->onCoinBaseTxDeleted(hash, notifyUser, recommendRescan);
						  });
		}

		void SpvService::onTxAdded(const TransactionPtr &tx) {
			_databaseManager.PutTransaction(ISO, tx);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&tx](Wallet::Listener *listener) {
							  listener->onTxAdded(tx);
						  });
		}

		void SpvService::onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {
			_databaseManager.UpdateTransaction(hashes, blockHeight, timestamp);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hashes, &blockHeight, &timestamp](Wallet::Listener *listener) {
							  listener->onTxUpdated(hashes, blockHeight, timestamp);
						  });
		}

		void SpvService::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			_databaseManager.DeleteTxByHash(hash);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, &notifyUser, &recommendRescan](Wallet::Listener *listener) {
							  listener->onTxDeleted(hash, notifyUser, recommendRescan);
						  });
		}

		void SpvService::onTxUpdatedAll(const std::vector<TransactionPtr> &txns) {
			_databaseManager.DeleteAllTransactions();
			_databaseManager.PutTransactions(ISO, txns);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&txns](Wallet::Listener *listener) {
								listener->onTxUpdatedAll(txns);
						  });
		}

		void SpvService::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			std::string assetID = asset->GetHash().GetHex();
			ByteStream stream;
			asset->Serialize(stream);
			AssetEntity assetEntity(assetID, amount, stream.GetBytes());
			_databaseManager.PutAsset(asset->GetName(), assetEntity);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&asset, &amount, &controller](Wallet::Listener *listener) {
							  listener->onAssetRegistered(asset, amount, controller);
						  });
		}

		//override PeerManager listener
		void SpvService::syncStarted() {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [](PeerManager::Listener *listener) {
							  listener->syncStarted();
						  });
		}

		void SpvService::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&currentHeight, &estimatedHeight, &lastBlockTime](PeerManager::Listener *listener) {
				listener->syncProgress(currentHeight, estimatedHeight, lastBlockTime);
			});
		}

		void SpvService::syncStopped(const std::string &error) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&error](PeerManager::Listener *listener) {
							  listener->syncStopped(error);
						  });
		}

		void SpvService::txStatusUpdate() {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [](PeerManager::Listener *listener) {
							  listener->txStatusUpdate();
						  });
		}

		void SpvService::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

			if (replace) {
				_databaseManager.DeleteAllBlocks(ISO);
			}

#ifndef NDEBUG
			if (blocks.size() == 1) {
				Log::debug("{} checkpoint ====> ({},  \"{}\", {}, {}),",
				           _peerManager->GetID(),
				           blocks[0]->GetHeight(),
				           blocks[0]->GetHash().GetHex(),
				           blocks[0]->GetTimestamp(),
				           blocks[0]->GetTarget());
			}
#endif

			_databaseManager.PutMerkleBlocks(ISO, blocks);

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &blocks](PeerManager::Listener *listener) {
							  listener->saveBlocks(replace, blocks);
						  });
		}

		void SpvService::savePeers(bool replace, const std::vector<PeerInfo> &peers) {

			if (replace) {
				_databaseManager.DeleteAllPeers();
			}

			std::vector<PeerEntity> peerEntityList;
			PeerEntity peerEntity;
			for (size_t i = 0; i < peers.size(); ++i) {
				peerEntity.address = peers[i].Address;
				peerEntity.port = peers[i].Port;
				peerEntity.timeStamp = peers[i].Timestamp;
				peerEntityList.push_back(peerEntity);
			}
			_databaseManager.PutPeers(ISO, peerEntityList);

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &peers](PeerManager::Listener *listener) {
							  listener->savePeers(replace, peers);
						  });
		}

		bool SpvService::networkIsReachable() {

			bool reachable = true;
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&reachable](PeerManager::Listener *listener) {
							  reachable |= listener->networkIsReachable();
						  });
			return reachable;
		}

		void SpvService::txPublished(const std::string &hash, const nlohmann::json &result) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&hash, &result](PeerManager::Listener *listener) {
							  listener->txPublished(hash, result);
						  });
		}

		void SpvService::connectStatusChanged(const std::string &status) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&status](PeerManager::Listener *listener) {
							  listener->connectStatusChanged(status);
						  });
		}

		TransactionPtr SpvService::GetTransaction(const uint256 &hash) {
			return _databaseManager.GetTransaction(hash);
		}

		size_t SpvService::GetAllTransactionsCount() {
			return _databaseManager.GetAllTransactionsCount();
		}

		std::vector<UTXOPtr> SpvService::loadCoinBaseUTXOs() {
			return _databaseManager.GetAllCoinBase();
		}

		// override protected methods
		std::vector<TransactionPtr> SpvService::loadTransactions() {
			return _databaseManager.GetAllTransactions();
		}

		std::vector<MerkleBlockPtr> SpvService::loadBlocks() {
			return _databaseManager.GetAllMerkleBlocks(ISO, _pluginTypes);
		}

		std::vector<PeerInfo> SpvService::loadPeers() {
			std::vector<PeerInfo> peers;

			std::vector<PeerEntity> peersEntity = _databaseManager.GetAllPeers(ISO);

			for (size_t i = 0; i < peersEntity.size(); ++i) {
				peers.push_back(PeerInfo(peersEntity[i].address, peersEntity[i].port, peersEntity[i].timeStamp));
			}

			return peers;
		}

		std::vector<AssetPtr> SpvService::loadAssets() {
			std::vector<AssetPtr> assets;

			std::vector<AssetEntity> assetsEntity = _databaseManager.GetAllAssets();

			for (size_t i = 0; i < assetsEntity.size(); ++i) {
				ByteStream stream(assetsEntity[i].Asset);
				AssetPtr asset(new Asset());
				if (asset->Deserialize(stream)) {
					asset->SetHash(uint256(assetsEntity[i].AssetID));
					assets.push_back(asset);
				}
			}

			return assets;
		}

		const CoreSpvService::PeerManagerListenerPtr &SpvService::createPeerManagerListener() {
			if (_peerManagerListener == nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(
						new WrappedExecutorPeerManagerListener(this, &_executor, _pluginTypes));
			}
			return _peerManagerListener;
		}

		const CoreSpvService::WalletListenerPtr &SpvService::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExecutorWalletListener(this, &_executor));
			}
			return _walletListener;
		}

		void SpvService::RegisterWalletListener(Wallet::Listener *listener) {
			_walletListeners.push_back(listener);
		}

		void SpvService::RegisterPeerManagerListener(PeerManager::Listener *listener) {
			_peerManagerListeners.push_back(listener);
		}

	}
}
