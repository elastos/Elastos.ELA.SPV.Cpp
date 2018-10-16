// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERMANAGER_H__
#define __ELASTOS_SDK_PEERMANAGER_H__

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "Lockable.h"
#include "Peer.h"
#include "ChainParams.h"
#include "Wallet.h"
#include "WrapperList.h"
#include "CMemBlock.h"
#include "PublishedTransaction.h"
#include "TransactionPeerList.h"
#include "Plugin/PluginTypes.h"
#include "Plugin/Interface/IMerkleBlock.h"
#include "Plugin/Block/MerkleBlock.h"
#include "BloomFilter.h"

#define PEER_MAX_CONNECTIONS 3

namespace Elastos {
	namespace ElaWallet {

		class PeerManager :
				public Lockable,
				public Peer::Listener {
		public:

			class Listener {
			public:
				Listener(const PluginTypes &pluginTypes);

				virtual ~Listener() {}

				// func syncStarted()
				virtual void syncStarted() = 0;

				// func syncStopped(_ error: BRPeerManagerError?)
				virtual void syncStopped(const std::string &error) = 0;

				// func txStatusUpdate()
				virtual void txStatusUpdate() = 0;

				// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
				virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) = 0;

				// func savePeers(_ replace: Bool, _ peers: [BRPeer])
				virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers) = 0;

				// func networkIsReachable() -> Bool}
				virtual bool networkIsReachable() = 0;

				// Called on publishTransaction
				virtual void txPublished(const std::string &error) = 0;

				virtual void blockHeightIncreased(uint32_t blockHeight) = 0;

				virtual void syncIsInactive() = 0;

				const PluginTypes &getPluginTypes() const { return _pluginTypes; }

			protected:
				PluginTypes _pluginTypes;
			};

		public:
			PeerManager(const ChainParams &params,
						const WalletPtr &wallet,
						uint32_t earliestKeyTime,
						const std::vector<MerkleBlockPtr> &blocks,
						const std::vector<PeerInfo> &peers,
						const boost::shared_ptr<Listener> &listener,
						const PluginTypes &plugins);

			~PeerManager();

			/**
			* Connect to bitcoin peer-to-peer network (also call this whenever networkIsReachable()
			* status changes)
			*/
			void connect();

			/**
			* Disconnect from bitcoin peer-to-peer network (may cause syncFailed(), saveBlocks() or
			* savePeers() callbacks to fire)
			*/
			void disconnect();

			void rescan();

			uint32_t getSyncStartHeight() const;

			uint32_t getEstimatedBlockHeight() const;

			uint32_t getLastBlockHeight() const;

			uint32_t getLastBlockTimestamp() const;

			double getSyncProgress(uint32_t startHeight);

			Peer::ConnectStatus getConnectStatus() const;

			void setFixedPeer(UInt128 address, uint16_t port);

			void setFixedPeers(const std::vector<PeerInfo> &peers);

			bool useFixedPeer(const std::string &node, int port);

			std::string getCurrentPeerName() const;

			std::string getDownloadPeerName() const;

			const PeerPtr getDownloadPeer() const;

			size_t getPeerCount() const;

			void publishTransaction(const TransactionPtr &transaction);

			void publishTransaction(const TransactionPtr &transaction, const Peer::PeerCallback &callback);

			uint64_t getRelayCount(const UInt256 &txHash) const;

			void asyncConnect(const boost::system::error_code &error);

			const std::vector<PublishedTransaction> getPublishedTransaction() const;

			const std::vector<UInt256> getPublishedTransactionHashes() const;

			int reconnectTaskCount() const;

			int &reconnectTaskCount();

			const PluginTypes &GetPluginTypes() const;

		public:
			virtual void OnConnected(const PeerPtr &peer);

			virtual void OnDisconnected(const PeerPtr &peer, int error);

			virtual void OnRelayedPeers(const PeerPtr &peer, const std::vector<PeerInfo> &peers, size_t peersCount);

			virtual void OnRelayedTx(const PeerPtr &peer, const TransactionPtr &tx);

			virtual void OnHasTx(const PeerPtr &peer, const UInt256 &txHash);

			virtual void OnRejectedTx(const PeerPtr &peer, const UInt256 &txHash, uint8_t code);

			virtual void OnRelayedBlock(const PeerPtr &peer, const MerkleBlockPtr &block);

			virtual void OnRelayedPingMsg(const PeerPtr &peer);

			virtual void OnNotfound(const PeerPtr &peer, const std::vector<UInt256> &txHashes,
									const std::vector<UInt256> &blockHashes);

			virtual void OnSetFeePerKb(const PeerPtr &peer, uint64_t feePerKb);

			virtual const TransactionPtr &OnRequestedTx(const PeerPtr &peer, const UInt256 &txHash);

			virtual bool OnNetworkIsReachable(const PeerPtr &peer);

			virtual void OnThreadCleanup(const PeerPtr &peer);

		private:
			void fireSyncStarted();

			void fireSyncStopped(int error);

			void fireTxStatusUpdate();

			void fireSaveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			void fireSavePeers(bool replace, const std::vector<PeerInfo> &peers);

			bool fireNetworkIsReachable();

			void fireTxPublished(int error);

			void fireThreadCleanup();

			void fireBlockHeightIncreased(uint32_t height);

			void fireSyncIsInactive();

			int verifyDifficultyWrapper(const BRChainParams *params, const BRMerkleBlock *block, const BRSet *blockSet);

			int verifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet, uint32_t targetTimeSpan,
								 uint32_t targetTimePerBlock, const std::string &netType);

			int
			verifyDifficultyInner(const BRMerkleBlock *block, const BRMerkleBlock *previous, uint32_t transitionTime,
								  uint32_t targetTimeSpan, uint32_t targetTimePerBlock, const std::string &netType);

			void loadBloomFilter(const PeerPtr &peer);

			void updateBloomFilter();

			void findPeers();

			void sortPeers();

			void syncStopped();

			void addTxToPublishList(const TransactionPtr &tx, const Peer::PeerCallback &callback);

			void publishPendingTx(const PeerPtr &peer);

			size_t
			addPeerToList(const PeerPtr &peer, const UInt256 &txHash, std::vector<TransactionPeerList> &peerList);

			bool
			removePeerFromList(const PeerPtr &peer, const UInt256 &txHash, std::vector<TransactionPeerList> &peerList);

			void peerMisbehaving(const PeerPtr &peer);

			std::vector<UInt128> addressLookup(const std::string &hostname);

			bool verifyBlock(const MerkleBlockPtr &block, const MerkleBlockPtr &prev, const PeerPtr &peer);

			std::vector<UInt256> getBlockLocators(size_t locatorsCount);

			void loadMempools();


			void requestUnrelayedTx(const PeerPtr &peer);

			bool peerListHasPeer(const std::vector<TransactionPeerList> &peerList, const UInt256 &txhash,
								 const PeerPtr &peer);

			size_t PeerListCount(const std::vector<TransactionPeerList> &list, const UInt256 &txhash);

			void loadBloomFilterDone(const PeerPtr &peer, int success);

			void updateFilterPingDone(const PeerPtr &peer, int success);

			void mempoolDone(const PeerPtr &peer, int success);

			void requestUnrelayedTxGetDataDone(const PeerPtr &peer, int success);

			void publishTxInivDone(const PeerPtr &peer, int success);

		private:
			int isConnected, connectFailureCount, misbehavinCount, dnsThreadCount, maxConnectCount, _reconnectTaskCount;

			std::vector<PeerInfo> _peers;
			std::vector<PeerInfo> _fiexedPeers;
			PeerInfo fixedPeer;

			std::vector<PeerPtr> _connectedPeers;
			PeerPtr downloadPeer;

			mutable std::string downloadPeerName;
			uint32_t _earliestKeyTime, syncStartHeight, filterUpdateHeight, estimatedHeight;
			BloomFilterPtr bloomFilter;
			double fpRate, averageTxPerBlock;
			BlockSet _blocks;
			std::set<MerkleBlockPtr> _orphans;
			BlockSet _checkpoints;
			MerkleBlockPtr lastBlock, lastOrphan;
			std::vector<TransactionPeerList> txRelays, txRequests;
			std::vector<PublishedTransaction> publishedTx;
			std::vector<UInt256> publishedTxHashes;

			PluginTypes _pluginTypes;
			WalletPtr _wallet;
			ChainParams _chainParams;
			boost::weak_ptr<Listener> _listener;
		};

		typedef boost::shared_ptr<PeerManager> PeerManagerPtr;

	}
}

#endif //__ELASTOS_SDK_PEERMANAGER_H__
