// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerManager.h"
#include "Message/PingMessage.h"
#include "Message/GetBlocksMessage.h"
#include "Message/BloomFilterMessage.h"
#include "Message/MempoolMessage.h"
#include "Message/GetDataMessage.h"
#include "Message/InventoryMessage.h"
#include "Message/GetHeadersMessage.h"

#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Block/ELAMerkleBlock.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/arith_uint256.h>
#include <SDK/Base/BloomFilter.h>

#include <Core/BRArray.h>
#include <Core/BRChainParams.h>
#include <Core/BRAddress.h>

#include <netdb.h>
#include <netinet/in.h>
#include <boost/bind.hpp>
#include <arpa/inet.h>

#define PROTOCOL_TIMEOUT      50.0
#define MAX_CONNECT_FAILURES  20 // notify user of network problems after this many connect failures in a row
#define PEER_FLAG_SYNCED      0x01
#define PEER_FLAG_NEEDSUPDATE 0x02

namespace Elastos {
	namespace ElaWallet {

		void PeerManager::fireSyncStarted() {
			if (!_listener.expired()) {
				_listener.lock()->syncStarted();
			}
		}

		void PeerManager::fireSyncProgress(uint32_t currentHeight, uint32_t estimatedHeight) {
			if (!_listener.expired()) {
				_listener.lock()->syncProgress(currentHeight, estimatedHeight);
			}
		}

		void PeerManager::fireSyncStopped(int error) {
			if (!_listener.expired()) {
				_listener.lock()->syncStopped(error == 0 ? "" : strerror(error));
			}
		}

		void PeerManager::fireTxStatusUpdate() {
			if (!_listener.expired()) {
				_listener.lock()->txStatusUpdate();
			}
		}

		void PeerManager::fireSaveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			if (!_listener.expired()) {
				_listener.lock()->saveBlocks(replace, blocks);
			}
		}

		void PeerManager::fireSavePeers(bool replace, const std::vector<PeerInfo> &peers) {
			if (!_listener.expired()) {
				_listener.lock()->savePeers(replace, peers);
			}
		}

		bool PeerManager::fireNetworkIsReachable() {
			bool result = false;
			if (!_listener.expired()) {
				result = _listener.lock()->networkIsReachable();
			}
			return result;
		}

		void PeerManager::fireTxPublished(const UInt256 &hash, int code, const std::string &reason) {
			nlohmann::json result;
			result["Code"] = code;
			result["Reason"] = reason;
			std::string txID = Utils::UInt256ToString(hash, true);

			if (!_listener.expired()) {
				_listener.lock()->txPublished(txID, result);
			}
		}

		void PeerManager::fireThreadCleanup() {
			if (!_listener.expired()) {
			}
		}

		void PeerManager::fireBlockHeightIncreased(uint32_t height) {
			if (!_listener.expired()) {
				_listener.lock()->blockHeightIncreased(height);
			}
		}

		void PeerManager::fireSyncIsInactive(uint32_t time) {
			if (!_listener.expired()) {
				_listener.lock()->syncIsInactive(time);
			}
		}

		PeerManager::Listener::Listener(const PluginType &pluginTypes) :
				_pluginTypes(pluginTypes) {
		}

		PeerManager::PeerManager(const ChainParams &params,
								 const WalletPtr &wallet,
								 uint32_t earliestKeyTime,
								 uint32_t reconnectSeconds,
								 const std::vector<MerkleBlockPtr> &blocks,
								 const std::vector<PeerInfo> &peers,
								 const boost::shared_ptr<PeerManager::Listener> &listener,
								 const PluginType &plugin) :
				_wallet(wallet),
				_lastBlock(nullptr),
				_lastOrphan(nullptr),
				_pluginType(plugin),
				_chainParams(params),

				_syncSucceeded(false),
				_enableReconnect(true),

				_isConnected(0),
				_connectFailureCount(0),
				_misbehavinCount(0),
				_dnsThreadCount(0),
				_maxConnectCount(PEER_MAX_CONNECTIONS),
				_reconnectTaskCount(0),

				_keepAliveTimestamp(0),
				_earliestKeyTime(earliestKeyTime),
				_reconnectSeconds(reconnectSeconds),
				_syncStartHeight(0),
				_filterUpdateHeight(0),
				_estimatedHeight(0),

				_fpRate(0),
				_averageTxPerBlock(1400) {

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			_peers = peers;
			sortPeers();

			time_t now = time(nullptr);
			const std::vector<CheckPoint> &Checkpoints = params.GetCheckpoints();
			for (size_t i = 0; i < Checkpoints.size(); i++) {
				MerkleBlockPtr checkBlock = Registry::Instance()->CreateMerkleBlock(_pluginType);
				checkBlock->setHeight(Checkpoints[i].GetHeight());
				checkBlock->setHash(Checkpoints[i].GetHash());
				checkBlock->setTimestamp(Checkpoints[i].GetTimestamp());
				checkBlock->setTarget(Checkpoints[i].GetTarget());
				_checkpoints.Insert(checkBlock);
				_blocks.Insert(checkBlock);
				if (i == 0 || checkBlock->getTimestamp() + 1 * 24 * 60 * 60 < earliestKeyTime ||
					(earliestKeyTime == 0 && checkBlock->getTimestamp() + 1 * 24 * 60 * 60 < now))
					_lastBlock = checkBlock;
			}

			MerkleBlockPtr block;
			for (size_t i = 0; i < blocks.size(); i++) {
				assert(blocks[i]->getHeight() !=
					   BLOCK_UNKNOWN_HEIGHT); // height must be saved/restored along with serialized block
				_orphans.insert(blocks[i]);

				if ((blocks[i]->getHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0 &&
					(block == nullptr || blocks[i]->getHeight() > block->getHeight()))
					block = blocks[i]; // find last transition block
			}

			MerkleBlockPtr orphan = Registry::Instance()->CreateMerkleBlock(_pluginType);
			while (block != nullptr) {
				_blocks.Insert(block);
				_lastBlock = block;

				orphan->setPrevBlockHash(block->getPrevBlockHash());
				for (std::set<MerkleBlockPtr>::const_iterator it = _orphans.cbegin(); it != _orphans.cend();) {
					if (UInt256Eq(&orphan->getPrevBlockHash(), &(*it)->getPrevBlockHash())) {
						it = _orphans.erase(it);
						break;
					} else {
						++it;
					}
				}

				orphan->setPrevBlockHash(block->getHash());
				block = nullptr;
				for (std::set<MerkleBlockPtr>::const_iterator it = _orphans.cbegin(); it != _orphans.cend(); ++it) {
					if (UInt256Eq(&orphan->getPrevBlockHash(), &(*it)->getPrevBlockHash())) {
						block = *it;
					}
				}
			}
		}

		PeerManager::~PeerManager() {
		}

		Peer::ConnectStatus PeerManager::getConnectStatus() const {
			Peer::ConnectStatus status = Peer::Disconnected;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				if (_isConnected != 0) status = Peer::Connected;

				for (size_t i = _connectedPeers.size(); i > 0 && status == Peer::Disconnected; i--) {
					if (_connectedPeers[i - 1]->GetConnectStatus() == Peer::Disconnected) continue;
					status = Peer::Connecting;
				}
			}
			return status;
		}

		bool PeerManager::SyncSucceeded() const {
			lock.lock();
			bool status = _syncSucceeded;
			lock.unlock();

			return status;
		}

		void PeerManager::SetSyncSucceeded(bool succeeded) {
			lock.lock();
			_syncSucceeded = succeeded;
			lock.unlock();
		}

		bool PeerManager::IsReconnectEnable() const {
			lock.lock();
			bool status = _enableReconnect;
			lock.unlock();

			return status;
		}

		void PeerManager::SetReconnectEnableStatus(bool status) {
			lock.lock();
			_enableReconnect = status;
			lock.unlock();
		}

		void PeerManager::connect() {
			lock.lock();
			if (_connectFailureCount >= MAX_CONNECT_FAILURES) _connectFailureCount = 0; //this is a manual retry

			if ((!_downloadPeer || _lastBlock->getHeight() < _estimatedHeight) && _syncStartHeight == 0) {
				_syncStartHeight = _lastBlock->getHeight() + 1;
				lock.unlock();
				fireSyncStarted();
				lock.lock();
			}

			for (size_t i = _connectedPeers.size(); i > 0; i--) {
				if (_connectedPeers[i]->GetConnectStatus() == Peer::Connecting)
					_connectedPeers[i]->Connect();
			}

			if (_connectedPeers.size() < _maxConnectCount) {
				time_t now = time(NULL);
				std::vector<PeerInfo> peers;

				if (_peers.size() < _maxConnectCount ||
					_peers[_maxConnectCount - 1].Timestamp + 3 * 24 * 60 * 60 < now) {
					findPeers();
				}

				peers.insert(peers.end(), _peers.begin(), _peers.end());

				while (!peers.empty() && _connectedPeers.size() < _maxConnectCount) {
					size_t i = BRRand((uint32_t) peers.size()); // index of random peer

					i = i * i / peers.size(); // bias random peer selection toward peers with more recent timestamp

					for (size_t j = _connectedPeers.size(); i != SIZE_MAX && j > 0; j--) {
						if (peers[i] != _connectedPeers[j - 1]->GetPeerInfo()) continue;
						peers.erase(peers.begin() + i);
						i = SIZE_MAX;
					}

					if (i != SIZE_MAX) {
						PeerPtr newPeer = PeerPtr(new Peer(this, _chainParams.GetMagicNumber()));
						newPeer->initDefaultMessages();
						newPeer->SetPeerInfo(peers[i]);
						newPeer->setEarliestKeyTime(_earliestKeyTime);
						peers.erase(peers.begin() + i);

						_connectedPeers.push_back(newPeer);
						newPeer->Connect();
					}
				}
			}

			if (_connectedPeers.empty()) {
				Log::warn("sync failed");
				syncStopped();
				lock.unlock();
				fireSyncStopped(ENETUNREACH);
				fireSyncIsInactive(60);
			} else {
				lock.unlock();
			}
		}

		void PeerManager::disconnect() {
			struct timespec ts;
			size_t peerCount, dnsThreadCount;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				peerCount = _connectedPeers.size();
				dnsThreadCount = this->_dnsThreadCount;

				for (size_t i = peerCount; i > 0; i--) {
					_connectFailureCount = MAX_CONNECT_FAILURES; // prevent futher automatic reconnect attempts
					_connectedPeers[i - 1]->Disconnect();
				}
			}

			ts.tv_sec = 0;
			ts.tv_nsec = 1;

			while (peerCount > 0 || dnsThreadCount > 0) {
				nanosleep(&ts, NULL); // pthread_yield() isn't POSIX standard :(
				{
					boost::mutex::scoped_lock scoped_lock(lock);
					peerCount = _connectedPeers.size();
					dnsThreadCount = this->_dnsThreadCount;
				}
			}
		}

		void PeerManager::rescan() {
			lock.lock();

			if (_isConnected) {
				// start the chain download from the most recent checkpoint that's at least a week older than earliestKeyTime
				const std::vector<CheckPoint> &checkpoints = _chainParams.GetCheckpoints();
				for (size_t i = checkpoints.size(); i > 0; i--) {
					if (i - 1 == 0 ||
						checkpoints[i - 1].GetTimestamp() + 7 * 24 * 60 * 60 < _earliestKeyTime) {
						UInt256 hash = checkpoints[i - 1].GetHash();
						_lastBlock = _blocks.Get(hash);
						break;
					}
				}

				if (_downloadPeer) { // disconnect the current download peer so a new random one will be selected
					for (size_t i = _peers.size(); i > 0; i--) {
						if (_peers[i - 1] == _downloadPeer->GetPeerInfo())
							_peers.erase(_peers.begin() + i - 1);
					}

					_downloadPeer->Disconnect();
				}

				_syncStartHeight = 0; // a syncStartHeight of 0 indicates that syncing hasn't started yet
				lock.unlock();
				connect();
			} else lock.unlock();
		}

		uint32_t PeerManager::getSyncStartHeight() const {
			return _syncStartHeight;
		}

		uint32_t PeerManager::getEstimatedBlockHeight() const {
			uint32_t height;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = (_lastBlock->getHeight() < _estimatedHeight) ? _estimatedHeight : _lastBlock->getHeight();
			}
			return height;
		}

		uint32_t PeerManager::GetLastBlockHeight() const {
			uint32_t height;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = _lastBlock->getHeight();
			}
			return height;
		}

		uint32_t PeerManager::GetLastBlockTimestamp() const {
			uint32_t timestamp;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				timestamp = _lastBlock->getTimestamp();
			}
			return timestamp;
		}

		time_t PeerManager::getKeepAliveTimestamp() const {
			time_t t;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				t = _keepAliveTimestamp;
			}
			return t;
		}

		void PeerManager::SetKeepAliveTimestamp(time_t t) {
			_keepAliveTimestamp = t;
		}

		double PeerManager::getSyncProgress(uint32_t startHeight) {
			double progress;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				if (startHeight == 0) startHeight = _syncStartHeight;

				if (!_downloadPeer && _syncStartHeight == 0) {
					progress = 0.0;
				} else if (!_downloadPeer || _lastBlock->getHeight() < _estimatedHeight) {
					if (_lastBlock->getHeight() > startHeight && _estimatedHeight > startHeight) {
						progress = 0.1 + 0.9 * (_lastBlock->getHeight() - startHeight) / (_estimatedHeight - startHeight);
					} else progress = 0.05;
				} else progress = 1.0;
			}
			return progress;
		}

		void PeerManager::setFixedPeers(const std::vector<PeerInfo> &peers) {
			disconnect();
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				_fiexedPeers = peers;
			}
		}

		void PeerManager::setFixedPeer(UInt128 address, uint16_t port) {
			disconnect();
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				_maxConnectCount = UInt128IsZero(&address) ? PEER_MAX_CONNECTIONS : 1;
				_fixedPeer = PeerInfo(address, port, 0, 0);
				_peers.clear();
			}
		}

		bool PeerManager::useFixedPeer(const std::string &node, int port) {
			UInt128 address = UINT128_ZERO;
			uint16_t _port = (uint16_t) port;

			if (!node.empty()) {
				struct in_addr addr;
				if (inet_pton(AF_INET, node.c_str(), &addr) != 1) return false;
				address.u16[5] = 0xffff;
				address.u32[3] = addr.s_addr;
				if (port == 0)
					_port = _chainParams.GetStandardPort();
			} else {
				_port = 0;
			}

			setFixedPeer(address, _port);
			return true;
		}

		std::string PeerManager::getCurrentPeerName() const {
			return getDownloadPeerName();
		}

		std::string PeerManager::getDownloadPeerName() const {
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				if (_downloadPeer) {
					std::stringstream ss;
					ss << _downloadPeer->getHost() << ":" << _downloadPeer->GetPort();
					_downloadPeerName = ss.str();
				} else _downloadPeerName = "";
			}
			return _downloadPeerName;
		}

		const PeerPtr PeerManager::getDownloadPeer() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _downloadPeer;
		}

		size_t PeerManager::getPeerCount() const {
			size_t count = 0;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->GetConnectStatus() != Peer::Disconnected) count++;
				}
			}
			return count;
		}

		void PeerManager::publishTransaction(const TransactionPtr &tx) {
			publishTransaction(tx, boost::bind(&PeerManager::fireTxPublished, this, _1, _2, _3));
		}

		void PeerManager::publishTransaction(const TransactionPtr &tx,
											 const Peer::PeerPubTxCallback &callback) {

			assert(tx != NULL && tx->isSigned());
			bool txValid = (tx != nullptr);
			if (tx) lock.lock();

			if (tx && !tx->isSigned()) {
				lock.unlock();
				if (!callback.empty()) callback(tx->getHash(), EINVAL, "tx not signed"); // transaction not signed
				txValid = false;
//			} else if (tx && !_isConnected) {
//				int connectFailureCount = connectFailureCount;
//				lock.unlock();
//
//				if (connectFailureCount >= MAX_CONNECT_FAILURES ||
//					(!fireNetworkIsReachable())) {
//					if (!callback.empty()) callback(ENOTCONN); // not connected to bitcoin network
//				} else lock.lock();
			}

			if (txValid) {
				size_t i, count = 0;

				tx->setTimestamp((uint32_t) time(NULL)); // set timestamp to publish time
				addTxToPublishList(tx, callback);

				for (i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->GetConnectStatus() == Peer::Connected) count++;
				}

				for (i = _connectedPeers.size(); i > 0; i--) {
					const PeerPtr &peer = _connectedPeers[i - 1];

					if (peer->GetConnectStatus() != Peer::Connected) continue;

					// instead of publishing to all peers, leave out downloadPeer to see if tx propogates/gets relayed back
					// TODO: XXX connect to a random peer with an empty or fake bloom filter just for publishing
					if (peer != _downloadPeer || count == 1) {
						publishPendingTx(peer);

						PingParameter pingParameter;
						pingParameter.callback = boost::bind(&PeerManager::publishTxInivDone, this, peer, _1);
						peer->SendMessage(MSG_PING, pingParameter);
					}
				}

				lock.unlock();
			}
		}

		uint64_t PeerManager::getRelayCount(const UInt256 &txHash) const {
			size_t count = 0;

			assert(!UInt256IsZero(&txHash));

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = _txRelays.size(); i > 0; i--) {
					if (!UInt256Eq(&_txRelays[i - 1].GetTransactionHash(), &txHash)) continue;
					count = _txRelays[i - 1].GetPeers().size();
					break;
				}
			}

			return count;
		}

		int PeerManager::verifyDifficulty(const ChainParams &params, const MerkleBlockPtr &block,
										  const BlockSet &blockSet) {
			MerkleBlockPtr previous, b = nullptr;
			uint32_t i;
			const uint32_t &targetTimeSpan = params.GetTargetTimeSpan();
			const uint32_t &targetTimePerBlock = params.GetTargetTimePerBlock();

			assert(block != nullptr);

			uint64_t blocksPerRetarget = targetTimeSpan / targetTimePerBlock;

			// check if we hit a difficulty transition, and find previous transition block
			if ((block->getHeight() % blocksPerRetarget) == 0) {
				for (i = 0, b = block; b && i < blocksPerRetarget; i++) {
					b = blockSet.Get(b->getPrevBlockHash());
				}
			}

			previous = blockSet.Get(block->getPrevBlockHash());
			return verifyDifficultyInner(block, previous, (b != nullptr) ? b->getTimestamp() : 0, targetTimeSpan,
										 targetTimePerBlock);
		}

		int PeerManager::verifyDifficultyInner(const MerkleBlockPtr &block, const MerkleBlockPtr &previous,
											   uint32_t transitionTime, uint32_t targetTimeSpan,
											   uint32_t targetTimePerBlock) {
			int r = 1;

			assert(block != nullptr);
			assert(previous != nullptr);

			uint64_t blocksPerRetarget = targetTimeSpan / targetTimePerBlock;

			if (!previous || !UInt256Eq(&(block->getPrevBlockHash()), &(previous->getHash())) ||
				block->getHeight() != previous->getHeight() + 1)
				r = 0;
			if (r && (block->getHeight() % blocksPerRetarget) == 0 && transitionTime == 0) r = 0;

			if (r && (block->getHeight() % blocksPerRetarget) == 0) {
				uint32_t timespan = previous->getTimestamp() - transitionTime;

				arith_uint256 target;
				target.SetCompact(previous->getTarget());

				// limit difficulty transition to -75% or +400%
				if (timespan < targetTimeSpan / 4) timespan = uint32_t(targetTimeSpan) / 4;
				if (timespan > targetTimeSpan * 4) timespan = uint32_t(targetTimeSpan) * 4;

				// TARGET_TIMESPAN happens to be a multiple of 256, and since timespan is at least TARGET_TIMESPAN/4, we don't
				// lose precision when target is multiplied by timespan and then divided by TARGET_TIMESPAN/256
				target *= timespan;
				target /= targetTimeSpan;

				uint32_t actualTargetCompact = target.GetCompact();
				if (block->getTarget() != actualTargetCompact) r = 0;
			} else if (r && previous->getHeight() != 0 && block->getTarget() != previous->getTarget()) r = 0;

			return r;
		}

		void PeerManager::loadBloomFilter(const PeerPtr &peer) {
			// every time a new wallet address is added, the bloom filter has to be rebuilt, and each address is only used
			// for one transaction, so here we generate some spare addresses to avoid rebuilding the filter each time a
			// wallet transaction is encountered during the chain sync
			_wallet->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			_wallet->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);

			_orphans.clear(); // clear out orphans that may have been received on an old filter
			_lastOrphan = nullptr;
			_filterUpdateHeight = _lastBlock->getHeight();
			_fpRate = BLOOM_REDUCED_FALSEPOSITIVE_RATE;

			std::vector<std::string> addrs = _wallet->getAllAddresses();
			std::vector<UTXO> utxos = _wallet->getAllUTXOsSafe();
			uint32_t blockHeight = (_lastBlock->getHeight() > 100) ? _lastBlock->getHeight() - 100 : 0;

			std::vector<TransactionPtr> transactions = _wallet->TxUnconfirmedBefore(blockHeight);
			BloomFilterPtr filter = BloomFilterPtr(
					new BloomFilter(_fpRate, addrs.size() + _wallet->getListeningAddrs().size() +
											 utxos.size() + transactions.size() + 100,
									(uint32_t) peer->GetPeerInfo().GetHash(),
									BLOOM_UPDATE_ALL)); // BUG: XXX txCount not the same as number of spent wallet outputs

			for (size_t i = 0; i < addrs.size(); i++) { // add addresses to watch for tx receiveing money to the wallet
				UInt168 hash = UINT168_ZERO;
				BRAddressHash168(&hash, addrs[i].c_str());
				CMBlock hashData(sizeof(hash));
				memcpy(hashData, hash.u8, sizeof(hash));

				if (!UInt168IsZero(&hash) && !filter->ContainsData(hashData)) {
					filter->insertData(hashData);
				}
			}

			for (size_t i = 0; i < _wallet->getListeningAddrs().size(); ++i) {
				UInt168 hash = UINT168_ZERO;
				BRAddressHash168(&hash, _wallet->getListeningAddrs()[i].c_str());
				CMBlock hashData(sizeof(hash));
				memcpy(hashData, hash.u8, sizeof(hash));

				if (!UInt168IsZero(&hash) && !filter->ContainsData(hashData)) {
					filter->insertData(hashData);
				}
			}

			for (size_t i = 0; i < utxos.size(); i++) { // add UTXOs to watch for tx sending money from the wallet
				CMBlock o(sizeof(UInt256) + sizeof(uint32_t));

				UInt256Set(o, utxos[i].hash);
				UInt32SetLE(&o[sizeof(UInt256)], utxos[i].n);
				if (!filter->ContainsData(o)) filter->insertData(o);
			}

			for (size_t i = 0; i < transactions.size(); i++) { // also add TXOs spent within the last 100 blocks
				for (size_t j = 0; j < transactions[i]->getInputs().size(); j++) {
					const TransactionInput &input = transactions[i]->getInputs()[j];
					const TransactionPtr &tx = _wallet->transactionForHash(input.getTransctionHash());
					CMBlock o(sizeof(UInt256) + sizeof(uint32_t));

					if (tx && input.getIndex() < tx->getOutputs().size() &&
						_wallet->containsAddress(tx->getOutputs()[input.getIndex()].getAddress())) {
						UInt256Set(o, input.getTransctionHash());
						UInt32SetLE(&o[sizeof(UInt256)], input.getIndex());
						if (!filter->ContainsData(o))
							filter->insertData(o);
					}
				}
			}

			_bloomFilter = filter;
			// TODO: XXX if already synced, recursively add inputs of unconfirmed receives
			BloomFilterParameter bloomFilterParameter;
			bloomFilterParameter.Filter = filter;
			peer->SendMessage(MSG_FILTERLOAD, bloomFilterParameter);
		}

		void PeerManager::sortPeers() {
			// comparator for sorting peers by timestamp, most recent first
			std::sort(_peers.begin(), _peers.end(), [](const PeerInfo &first, const PeerInfo &second) {
				return first.Timestamp > second.Timestamp;
			});
		}

		void PeerManager::findPeers() {
			uint64_t services = SERVICES_NODE_NETWORK | SERVICES_NODE_BLOOM | _chainParams.GetServices();
			time_t now = time(NULL);
			struct timespec ts;

			_peers.clear();
			size_t peersCount = _fiexedPeers.size();
			if (peersCount > 0) {
				for (int i = 0; i < peersCount; ++i) {
					_peers.push_back(_fiexedPeers[i]);
					_peers[i].Timestamp = now;
				}
			} else if (!UInt128IsZero(&_fixedPeer.Address)) {
				_peers.push_back(_fixedPeer);
				_peers[0].Services = services;
				_peers[0].Timestamp = now;
			} else {
				std::vector<UInt128> addrList;
				const std::vector<std::string> &dnsSeeds = _chainParams.GetDNSSeeds();
				for (size_t i = 0; i < dnsSeeds.size(); i++) {
					addrList = addressLookup(dnsSeeds[i]);
					for (std::vector<UInt128>::iterator addr = addrList.begin();
						 addr != addrList.end() && !UInt128IsZero(&(*addr)); addr++) {
						_peers.emplace_back(*addr, _chainParams.GetStandardPort(), now, services);
					}
				}

				ts.tv_sec = 0;
				ts.tv_nsec = 1;

				do {
					Unlock();
					nanosleep(&ts, NULL); // pthread_yield() isn't POSIX standard :(
					Lock();
				} while (_dnsThreadCount > 0 && _peers.size() < PEER_MAX_CONNECTIONS);

				sortPeers();

				Log::debug("found {} peers", _peers.size());
			}
		}

		void PeerManager::syncStopped() {
			_syncStartHeight = 0;

			if (_downloadPeer != nullptr) {
				// don't cancel timeout if there's a pending tx publish callback
				for (size_t i = _publishedTx.size(); i > 0; i--) {
					if (_publishedTx[i - 1].HasCallback()) return;
				}

				_downloadPeer->scheduleDisconnect(-1); // cancel sync timeout
			}
		}

		void PeerManager::addTxToPublishList(const TransactionPtr &tx, const Peer::PeerPubTxCallback &callback) {
			if (tx && tx->getBlockHeight() == TX_UNCONFIRMED) {
				for (size_t i = _publishedTx.size(); i > 0; i--) {
					if (_publishedTx[i - 1].GetTransaction()->IsEqual(tx.get())) return;
				}

				_publishedTx.emplace_back(tx, callback);
				_publishedTxHashes.push_back(tx->getHash());

				for (size_t i = 0; i < tx->getInputs().size(); i++) {
					addTxToPublishList(_wallet->transactionForHash(tx->getInputs()[i].getTransctionHash()),
									   Peer::PeerPubTxCallback());
				}
			}
		}

		void PeerManager::asyncConnect(const boost::system::error_code &error) {
			if (error.value() == 0) {
				if (getConnectStatus() != Peer::Connected) {
					Log::info("async connecting...");
					connect();
				}
			} else {
				Log::warn("asyncConnect err: {}", error.message());
			}

			if (_reconnectTaskCount > 0) {
				{
					boost::mutex::scoped_lock scoped_lock(lock);
					_reconnectTaskCount = 0;
				}
			}
		}

		void PeerManager::OnConnected(const PeerPtr &peerPtr) {
			time_t now = time(nullptr);

			boost::mutex::scoped_lock scopedLock(lock);
			PeerPtr peer = peerPtr;
			if (peer->GetTimestamp() > now + 2 * 60 * 60 || peer->GetTimestamp() < now - 2 * 60 * 60)
				peer->SetTimestamp(now); // sanity check

			// TODO: XXX does this work with 0.11 pruned nodes?
			if ((peer->GetServices() & _chainParams.GetServices()) != _chainParams.GetServices()) {
				peer->warn("unsupported node type");
				peer->Disconnect();
			} else if ((peer->GetServices() & SERVICES_NODE_NETWORK) != SERVICES_NODE_NETWORK &&
				(peer->GetServices() & BTC_SERVICES_NODE_NETWORK) != BTC_SERVICES_NODE_NETWORK) {
				peer->warn("peer->services: {} != SERVICES_NODE_NETWORK", peer->GetServices());
				peer->warn("node doesn't carry full blocks");
				peer->Disconnect();
			} else if (peer->GetLastBlock() + 10 < _lastBlock->getHeight()) {
				peer->warn("peer->lastBlock: {} !=  lastBlock->height: {}", peer->GetLastBlock(),
						   _lastBlock->getHeight());
				peer->warn("node isn't synced");
				peer->Disconnect();
//			} else if (peer->GetVersion() >= 70011 &&
//					   (peer->GetServices() & SERVICES_NODE_BLOOM) != SERVICES_NODE_BLOOM) {
//				peer->warn("node doesn't support SPV mode");
//				peer->Disconnect();
			} else if (_downloadPeer && // check if we should stick with the existing download peer
					   (_downloadPeer->GetLastBlock() >= peer->GetLastBlock() ||
						_lastBlock->getHeight() >= peer->GetLastBlock())) {
				if (_lastBlock->getHeight() >= peer->GetLastBlock()) { // only load bloom filter if we're done syncing
					_connectFailureCount = 0; // also reset connect failure count if we're already synced
					loadBloomFilter(peer);
					publishPendingTx(peer);
					PingParameter pingParameter;
					pingParameter.callback = boost::bind(&PeerManager::loadBloomFilterDone, this, peer, _1);
					peer->SendMessage(MSG_PING, pingParameter);
				}
			} else { // select the peer with the lowest ping time to download the chain from if we're behind
				// BUG: XXX a malicious peer can report a higher lastblock to make us select them as the download peer, if
				// two peers agree on lastblock, use one of those two instead
				for (size_t i = _connectedPeers.size(); i > 0; i--) {
					const PeerPtr &p = _connectedPeers[i - 1];

					if (p->GetConnectStatus() != Peer::Connected) continue;
					if ((p->GetPingTime() < peer->GetPingTime() && p->GetLastBlock() >= peer->GetLastBlock()) ||
						p->GetLastBlock() > peer->GetLastBlock())
						peer = p;
				}

				if (_downloadPeer) {
					peer->info("selecting new download peer with higher reported lastblock");
					_downloadPeer->Disconnect();
				}

				bool havePendingTx = false;
				for (size_t i = 0; i < _publishedTx.size(); ++i) {
					if (_publishedTx[i].HasCallback()) {
						havePendingTx = true;
						break;
					}
				}

				_downloadPeer = peer;
				_syncSucceeded = false;
				_keepAliveTimestamp = time(nullptr);
				_isConnected = 1;
				if (_estimatedHeight < peer->GetLastBlock())
					_estimatedHeight = peer->GetLastBlock();
				loadBloomFilter(peer);
				peer->SetCurrentBlockHeight(_lastBlock->getHeight());
				publishPendingTx(peer);
				if (!havePendingTx) {
					peer->SendMessage(MSG_GETADDR, Message::DefaultParam);
				}

				if (_lastBlock->getHeight() < peer->GetLastBlock()) { // start blockchain sync

					peer->scheduleDisconnect(PROTOCOL_TIMEOUT); // schedule sync timeout

					// request just block headers up to a week before earliestKeyTime, and then merkleblocks after that
					// we do not reset connect failure count yet incase this request times out
					if (_lastBlock->getTimestamp() + 7 * 24 * 60 * 60 >= _earliestKeyTime) {
						peer->SendMessage(MSG_GETBLOCKS, GetBlocksParameter(getBlockLocators(), UINT256_ZERO));
					} else {
						peer->SendMessage(MSG_GETHEADERS, GetHeadersParameter(getBlockLocators(), UINT256_ZERO));
					}
				} else { // we're already synced
					_connectFailureCount = 0; // reset connect failure count
					loadMempools();
				}
			}

			_wallet->UpdateBalance();
		}

		void PeerManager::OnDisconnected(const PeerPtr &peer, int error) {
			int willSave = 0, willReconnect = 0, txError = 0;
			TransactionPeerList *peerList;
			std::vector<PublishedTransaction> pubTx;
			int reconnectSeconds = 60;

			{
				boost::mutex::scoped_lock scopedLock(lock);

				if (error == EPROTO) { // if it's protocol error, the peer isn't following standard policy
					peerMisbehaving(peer);
				} else if (error) { // timeout or some non-protocol related network error
					for (size_t i = _peers.size(); i > 0; i--) {
						if (_peers[i - 1] == peer->GetPeerInfo())
							_peers.erase(_peers.begin() + i - 1);
					}

					_connectFailureCount++;

					// if it's a timeout and there's pending tx publish callbacks, the tx publish timed out
					// BUG: XXX what if it's a connect timeout and not a publish timeout?
					if (error == ETIMEDOUT &&
						(peer != _downloadPeer || _syncStartHeight == 0 || _connectedPeers.size() == 1))
						txError = ETIMEDOUT;
				}

				for (size_t i = _txRelays.size(); i > 0; i--) {
					peerList = &_txRelays[i - 1];

					for (size_t j = peerList->GetPeers().size(); j > 0; j--) {
						if (peerList->GetPeers()[j - 1]->IsEqual(peer.get()))
							peerList->RemovePeerAt(j - 1);
					}
				}

				if (peer == _downloadPeer) { // download peer disconnected
					_isConnected = 0;
					_downloadPeer = NULL;
					if (_connectFailureCount > MAX_CONNECT_FAILURES)
						_connectFailureCount = MAX_CONNECT_FAILURES;
				}

				if (!_isConnected && _connectFailureCount == MAX_CONNECT_FAILURES) {
					syncStopped();

					// clear out stored peers so we get a fresh list from DNS on next connect attempt
					_peers.clear();
					txError = ENOTCONN; // trigger any pending tx publish callbacks
					willSave = 1;
					peer->warn("sync failed");
				} else if (_connectFailureCount < MAX_CONNECT_FAILURES) {
					willReconnect = 1;
				}

				if (txError) {
					for (size_t i = _publishedTx.size(); i > 0; i--) {
						if (!_publishedTx[i - 1].HasCallback()) continue;
						peer->error("transaction canceled: {}", strerror(txError));
						pubTx.push_back(_publishedTx[i - 1]);
//						_publishedTx[i - 1].ResetCallback();
					}
				}

				for (size_t i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1] != peer)
						continue;
					_connectedPeers.erase(_connectedPeers.begin() + i - 1);
					break;
				}

				bool havePendingTx = false;
				for (size_t i = 0; i < _publishedTx.size(); ++i) {
					if (_publishedTx[i].HasCallback()) {
						havePendingTx = true;
						break;
					}
				}

				if (_reconnectTaskCount == 0 && (!_isConnected || _connectedPeers.empty())) {
					willReconnect = 1;
					if (havePendingTx) {
						reconnectSeconds = 3;
						willReconnect = 1;
					}
					peer->debug("will reconnect {}s later...", reconnectSeconds);
				} else {
					willReconnect = 0;
				}
			}

//			for (size_t i = 0; i < pubTx.size(); i++) {
//				pubTx[i].FireCallback(txError, "tx canceled");
//			}

			//if (willSave) fireSavePeers(true, {});
			if (willSave) fireSyncStopped(error);
			if (willReconnect && _enableReconnect) fireSyncIsInactive(reconnectSeconds);
			fireTxStatusUpdate();
		}

		void PeerManager::OnRelayedPeers(const PeerPtr &peer, const std::vector<PeerInfo> &peers) {
			time_t now = time(NULL);
			bool willReconnect = false;
			size_t peersCount;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				peer->info("relayed {} peer(s)", peers.size());

				if (_chainParams.GetDNSSeeds().size() == _peers.size() || _peers.size() == 0) {
					willReconnect = true;
				}

				_peers.insert(_peers.end(), peers.begin(), peers.end());
				sortPeers();

				std::vector<PeerInfo> uniquePeers;
				bool found;
				for (size_t i = 0; i < _peers.size(); ++i) {
					found = false;
					for (size_t ui = 0; ui < uniquePeers.size(); ++ui) {
						if (UInt128Eq(&_peers[i].Address, &uniquePeers[ui].Address)) {
							found = true;
							break;
						}
					}

					if (!found) {
						uniquePeers.push_back(_peers[i]);
					}
				}
				_peers = uniquePeers;

				// limit total to 2500 peers
				if (_peers.size() > 2500) _peers.resize(2500);
				peersCount = _peers.size();

				// remove peers more than 3 hours old, or until there are only 1000 left
				while (peers.size() > 1000 && _peers.back().Timestamp + 3 * 60 * 60 < now) peersCount--;
				_peers.resize(peersCount);
			}

			// peer relaying is complete when we receive <1000
			if (peers.size() > 1 && peers.size() < 1000) {
				peer->info("save {} peers", peers.size());
				for (size_t i = 0; i < peers.size(); i++) {
					peer->info("peer[{}] = {}, timestamp = {}, port = {}, services = {}", i, peers[i].GetHost(),
							   peers[i].Timestamp, peers[i].Port, peers[i].Services);
				}
				fireSavePeers(true, _peers);
			}

			if (willReconnect) {
				fireSyncIsInactive(2);
			}
		}

		void PeerManager::OnRelayedTx(const PeerPtr &peer, const TransactionPtr &transaction) {
			int isWalletTx = 0, hasPendingCallbacks = 0;
			size_t relayCount = 0;
			TransactionPtr tx = transaction;
			PublishedTransaction pubTx;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				peer->info("relayed tx: {}", Utils::UInt256ToString(tx->getHash(), true));

				for (size_t i = _publishedTx.size(); i > 0; i--) { // see if tx is in list of published tx
					if (UInt256Eq(&_publishedTxHashes[i - 1], &tx->getHash())) {
						pubTx = _publishedTx[i - 1];
						_publishedTx[i - 1].ResetCallback();
						relayCount = addPeerToList(peer, tx->getHash(), _txRelays);
					} else if (_publishedTx[i - 1].HasCallback()) hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (_syncStartHeight == 0 || peer != _downloadPeer)) {
					peer->scheduleDisconnect(-1); // cancel publish tx timeout
				}

				if (_syncStartHeight == 0 || _wallet->containsTransaction(tx)) {
					isWalletTx = _wallet->registerTransaction(tx);
					if (isWalletTx) tx = _wallet->transactionForHash(tx->getHash());
				} else {
					tx = nullptr;
				}

				if (tx && isWalletTx) {
					// reschedule sync timeout
					if (_syncStartHeight > 0 && peer == _downloadPeer) {
						peer->scheduleDisconnect(PROTOCOL_TIMEOUT);
					}

					if (_wallet->getTransactionAmountSent(tx) > 0 &&
						_wallet->transactionIsValid(tx)) {
						addTxToPublishList(tx, Peer::PeerPubTxCallback());  // add valid send tx to mempool
					}

					// keep track of how many peers have or relay a tx, this indicates how likely the tx is to confirm
					// (we only need to track this after syncing is complete)
					if (_syncStartHeight == 0)
						relayCount = addPeerToList(peer, tx->getHash(), _txRelays);

					removePeerFromList(peer, tx->getHash(), _txRequests);

					if (_bloomFilter != nullptr) { // check if bloom filter is already being updated

						// the transaction likely consumed one or more wallet addresses, so check that at least the next <gap limit>
						// unused addresses are still matched by the bloom filter
						std::vector<Address> externalAddrs = _wallet->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
						std::vector<Address> internalAddrs = _wallet->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);

						UInt168 hash;
						CMBlock hashData(sizeof(UInt168));
						for (std::vector<Address>::iterator externalIt = externalAddrs.begin(), internalIt = internalAddrs.begin();
							 externalIt != externalAddrs.end() || internalIt != internalAddrs.end();) {
							if (BRAddressHash168(&hash, (*externalIt).GetChar())) {
								memcpy(hashData, hash.u8, sizeof(UInt168));
								if (!_bloomFilter->ContainsData(hashData)) {
									_bloomFilter.reset();
									updateBloomFilter();
									break;
								}
							}

							if (BRAddressHash168(&hash, (*internalIt).GetChar())) {
								memcpy(hashData, hash.u8, sizeof(UInt168));
								if (!_bloomFilter->ContainsData(hashData)) {
									_bloomFilter.reset();
									updateBloomFilter();
									break;
								}
							}
							if (externalIt != externalAddrs.end()) externalIt++;
							if (internalIt != internalAddrs.end()) internalIt++;
						}
					}
				}

				// set timestamp when tx is verified
				if (tx && relayCount >= _maxConnectCount && tx->getBlockHeight() == TX_UNCONFIRMED &&
					tx->getTimestamp() == 0) {
					_wallet->updateTransactions({tx->getHash()}, TX_UNCONFIRMED, (uint32_t) time(NULL));
				}
			}

			if (pubTx.HasCallback()) pubTx.FireCallback(0, "success");
		}

		void PeerManager::OnHasTx(const PeerPtr &peer, const UInt256 &txHash) {
			int isWalletTx = 0, hasPendingCallbacks = 0;
			size_t relayCount = 0;
			PublishedTransaction pubTx;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				TransactionPtr tx = _wallet->transactionForHash(txHash);
				peer->info("has tx: {}", Utils::UInt256ToString(txHash, true));

				for (size_t i = _publishedTx.size(); i > 0; i--) { // see if tx is in list of published tx
					if (UInt256Eq(&(_publishedTxHashes[i - 1]), &txHash)) {
						if (!tx) tx = _publishedTx[i - 1].GetTransaction();
						pubTx = _publishedTx[i - 1];
						_publishedTx[i - 1].ResetCallback();
						relayCount = addPeerToList(peer, txHash, _txRelays);
					} else if (_publishedTx[i - 1].HasCallback()) hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (_syncStartHeight == 0 || peer != _downloadPeer)) {
					peer->scheduleDisconnect(-1);  // cancel publish tx timeout
				}

				if (tx) {
					isWalletTx = _wallet->registerTransaction(tx);
					if (isWalletTx) tx = _wallet->transactionForHash(tx->getHash());

					// reschedule sync timeout
					if (_syncStartHeight > 0 && peer == _downloadPeer && isWalletTx) {
						peer->scheduleDisconnect(PROTOCOL_TIMEOUT);
					}

					// keep track of how many peers have or relay a tx, this indicates how likely the tx is to confirm
					// (we only need to track this after syncing is complete)
					if (_syncStartHeight == 0)
						relayCount = addPeerToList(peer, txHash, _txRelays);

					// set timestamp when tx is verified
					if (relayCount >= _maxConnectCount && tx && tx->getBlockHeight() == TX_UNCONFIRMED &&
						tx->getTimestamp() == 0) {
						std::vector<UInt256> hashes = {txHash};
						_wallet->updateTransactions(hashes, TX_UNCONFIRMED, (uint32_t) time(NULL));
					}

					removePeerFromList(peer, txHash, _txRequests);
				}
			}

			if (pubTx.HasCallback()) pubTx.FireCallback(0, "has tx");
		}

		void PeerManager::OnRejectedTx(const PeerPtr &peer, const UInt256 &txHash, uint8_t code, const std::string &reason) {

			PublishedTransaction pubTx;
			{
				boost::mutex::scoped_lock scopedLock(lock);
				peer->info("rejected tx: {}", Utils::UInt256ToString(txHash, true));
				TransactionPtr tx = _wallet->transactionForHash(txHash);
				removePeerFromList(peer, txHash, _txRequests);

				for (size_t i = _publishedTx.size(); i > 0; --i) { // see if tx is in list of published tx
					if (UInt256Eq(&_publishedTxHashes[i - 1], &(tx->getHash()))) {
						pubTx = _publishedTx[i - 1];
						if (code != 0x12)
							_publishedTx[i - 1].ResetCallback();
					}
				}

				if (tx) {
					if (removePeerFromList(peer, txHash, _txRelays) && tx->getBlockHeight() == TX_UNCONFIRMED) {
						// set timestamp 0 to mark tx as unverified
						_wallet->updateTransactions({txHash}, TX_UNCONFIRMED, 0);
					}

					// if we get rejected for any reason other than double-spend, the peer is likely misconfigured
					if (code != REJECT_SPENT && _wallet->getTransactionAmountSent(tx) > 0) {
						for (size_t i = 0; i <
										   tx->getInputs().size(); i++) { // check that all inputs are confirmed before dropping peer
							const TransactionPtr &t = _wallet->transactionForHash(
									tx->getInputs()[i].getTransctionHash());
							if (!t || t->getBlockHeight() != TX_UNCONFIRMED) continue;
							tx = nullptr;
							break;
						}

						if (tx != nullptr) peerMisbehaving(peer);
					}
				}
			}

			fireTxStatusUpdate();
			if (pubTx.HasCallback()) pubTx.FireCallback(code, reason);
			if (code != 0x12) {
				_wallet->removeTransaction(pubTx.GetTransaction()->getHash());
			}
		}

		void PeerManager::OnRelayedBlock(const PeerPtr &peer, const MerkleBlockPtr &block) {
			size_t i, j, fpCount = 0, saveCount = 0;
			MerkleBlockPtr b, b2, prev, next;
			uint32_t txTime = 0;
			std::vector<MerkleBlockPtr> saveBlocks;
			std::vector<UInt256> txHashes = block->MerkleBlockTxHashes();

			{
				boost::mutex::scoped_lock scopedLock(lock);
				prev = _blocks.Get(block->getPrevBlockHash());

				if (prev) {
					txTime = block->getTimestamp();
					block->setHeight(prev->getHeight() + 1);
				}

				// track the observed bloom filter false positive rate using a low pass filter to smooth out variance
				if (peer == _downloadPeer && block->getTransactionCount() > 0) {
					for (i = 0; i < txHashes.size(); i++) { // wallet tx are not false-positives
						if (_wallet->transactionForHash(txHashes[i]) == nullptr) fpCount++;
					}

					// moving average number of tx-per-block
					_averageTxPerBlock = _averageTxPerBlock * 0.999 + block->getTransactionCount() * 0.001;

					// 1% low pass filter, also weights each block by total transactions, compared to the avarage
					_fpRate = _fpRate * (1.0 - 0.01 * block->getTransactionCount() / _averageTxPerBlock) +
							 0.01 * fpCount / _averageTxPerBlock;

					// false positive rate sanity check
					if (peer->GetConnectStatus() == Peer::Connected &&
						_fpRate > BLOOM_DEFAULT_FALSEPOSITIVE_RATE * 10.0) {
						peer->warn(
							"bloom filter false positive rate {} too high after {} blocks, disconnecting...",
							_fpRate, _lastBlock->getHeight() + 1 - _filterUpdateHeight);
//						peer->Disconnect();
					} else if (_lastBlock->getHeight() + 500 < peer->GetLastBlock() &&
							   _fpRate > BLOOM_REDUCED_FALSEPOSITIVE_RATE * 10.0) {
						updateBloomFilter(); // rebuild bloom filter when it starts to degrade
					}
				}

				// ignore block headers that are newer than one week before earliestKeyTime (it's a header if it has 0 totalTx)
				if (block->getTransactionCount() == 0 &&
					block->getTimestamp() + 7 * 24 * 60 * 60 > _earliestKeyTime + 2 * 60 * 60) {
				} else if (_bloomFilter ==
						   nullptr) { // ingore potentially incomplete blocks when a filter update is pending

					if (peer == _downloadPeer && _lastBlock->getHeight() < _estimatedHeight) {
						peer->scheduleDisconnect(PROTOCOL_TIMEOUT); // reschedule sync timeout
						_connectFailureCount = 0; // reset failure count once we know our initial request didn't timeout
					}
				} else if (!prev) { // block is an orphan
					peer->info("relayed orphan block {}, previous {}, last block is {}, height {}",
							   Utils::UInt256ToString(block->getHash(), true),
							   Utils::UInt256ToString(block->getPrevBlockHash(), true),
							   Utils::UInt256ToString(_lastBlock->getHash(), true),
							   _lastBlock->getHeight());

					if (block->getHeight() + 7 * 24 * 60 * 60 <
						time(nullptr)) { // ignore orphans older than one week ago
					} else {
						// call getblocks, unless we already did with the previous block, or we're still syncing
						if (_lastBlock->getHeight() >= peer->GetLastBlock() &&
							(!_lastOrphan || !_lastOrphan->isEqual(block.get()))) {
							peer->info("calling getblocks");
							GetBlocksParameter getBlocksParameter(getBlockLocators(), UINT256_ZERO);
							peer->SendMessage(MSG_GETBLOCKS, getBlocksParameter);
						}

						_orphans.insert(block); // BUG: limit total orphans to avoid memory exhaustion attack
						_lastOrphan = block;
						peer->scheduleDisconnect(PROTOCOL_TIMEOUT); // reschedule sync timeout
					}
				} else if (!verifyBlock(block, prev, peer)) { // block is invalid
					peer->warn("relayed invalid block");
					peerMisbehaving(peer);
				} else if (UInt256Eq(&block->getPrevBlockHash(),
									 &_lastBlock->getHash())) { // new block extends main chain
					if ((block->getHeight() % 500) == 0 || txHashes.size() > 0 ||
						block->getHeight() >= peer->GetLastBlock()) {
						peer->info("adding block #{}, false positive rate: {}", block->getHeight(), _fpRate);
						if (block->getHeight() <= _estimatedHeight)
							fireSyncProgress(block->getHeight(), _estimatedHeight);
					}

					_blocks.Insert(block);
					_lastBlock = block;
					fireBlockHeightIncreased(block->getHeight());

					if (txHashes.size() > 0)
						_wallet->updateTransactions(txHashes, block->getHeight(), txTime);
					if (_downloadPeer) _downloadPeer->SetCurrentBlockHeight(block->getHeight());

					if (block->getHeight() < _estimatedHeight && peer == _downloadPeer) {
						peer->scheduleDisconnect(PROTOCOL_TIMEOUT); // reschedule sync timeout
						_connectFailureCount = 0; // reset failure count once we know our initial request didn't timeout
					}

					if ((block->getHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0)
						saveCount = 1; // save transition block immediately

					if (block->getHeight() == _estimatedHeight) { // chain download is complete
						saveCount = (block->getHeight() % BLOCK_DIFFICULTY_INTERVAL) + BLOCK_DIFFICULTY_INTERVAL + 1;
						loadMempools();
					}
				} else if (_blocks.Contains(block)) { // we already have the block (or at least the header)
					if ((block->getHeight() % 500) == 0 || txHashes.size() > 0 ||
						block->getHeight() >= peer->GetLastBlock()) {
						peer->info("relayed existing block #{}", block->getHeight());
					}

					b = _lastBlock;
					while (b && b->getHeight() > block->getHeight())
						b = _blocks.Get(b->getPrevBlockHash()); // is block in main chain?

					if (b->isEqual(block.get())) { // if it's not on a fork, set block heights for its transactions
						if (txHashes.size() > 0)
							_wallet->updateTransactions(txHashes, block->getHeight(), txTime);
						if (block->getHeight() == _lastBlock->getHeight()) _lastBlock = block;
					}

					b = _blocks.Get(block->getHash());
					if (b != nullptr) _blocks.Remove(b);
					_blocks.Insert(block);

					if (b != nullptr && b != block) {
						for(std::set<MerkleBlockPtr>::iterator it = _orphans.begin(); it != _orphans.end(); ++it) {
							if ((*it)->isEqual(b.get()))	{
								_orphans.erase(it);
								break;
							}
						}
						if (_lastOrphan == b) _lastOrphan = nullptr;
					}
				} else if (_lastBlock->getHeight() < peer->GetLastBlock() &&
						   block->getHeight() >
						   _lastBlock->getHeight() + 1) { // special case, new block mined durring rescan
					peer->info("marking new block #{} as orphan until rescan completes", block->getHeight());
					_orphans.insert(block); // mark as orphan til we're caught up
					_lastOrphan = block;
				} else if (block->getHeight() <= _chainParams.GetLastCheckpoint().GetHeight()) { // old fork
					peer->info("ignoring block on fork older than most recent checkpoint, block #{}, hash: {}",
							   block->getHeight(), Utils::UInt256ToString(block->getHash(), true));
				} else { // new block is on a fork
					peer->warn("chain fork reached height {}", block->getHeight());
					_blocks.Insert(block);

					if (block->getHeight() > _lastBlock->getHeight()) { // check if fork is now longer than main chain
						b = block;
						b2 = _lastBlock;

						while (b && b2 && !b->isEqual(b2.get())) { // walk back to where the fork joins the main chain
							b = _blocks.Get(b->getPrevBlockHash());
							if (b && b->getHeight() < b2->getHeight()) b2 = _blocks.Get(b2->getPrevBlockHash());
						}

						peer->info("reorganizing chain from height {}, new height is {}", b->getHeight(),
								   block->getHeight());

						_wallet->SetTxUnconfirmedAfter(b->getHeight());  // mark tx after the join point as unconfirmed

						b = block;

						while (b && b2 &&
							   b->getHeight() > b2->getHeight()) { // set transaction heights for new main chain
							uint32_t height = b->getHeight(), timestamp = b->getTimestamp();

							txHashes = b->MerkleBlockTxHashes();
							b = _blocks.Get(b->getPrevBlockHash());
							if (b) timestamp = timestamp / 2 + b->getTimestamp() / 2;
							if (txHashes.size() > 0)
								_wallet->updateTransactions(txHashes, height, timestamp);
						}

						_lastBlock = block;
						for (int k = 1; k <= block->getHeight() - b2->getHeight(); ++k) {
							fireBlockHeightIncreased(b2->getHeight() + k);
						}

						if (block->getHeight() == _estimatedHeight) { // chain download is complete
							saveCount =
									(block->getHeight() % BLOCK_DIFFICULTY_INTERVAL) + BLOCK_DIFFICULTY_INTERVAL + 1;
							loadMempools();
						}
					}
				}

				if (block && block->getHeight() != BLOCK_UNKNOWN_HEIGHT) {
					if (block->getHeight() > _estimatedHeight) _estimatedHeight = block->getHeight();

					// check if the next block was received as an orphan
					UInt256 prevBlockHash = block->getHash();
					for (std::set<MerkleBlockPtr>::iterator it = _orphans.begin(); it != _orphans.end(); ++it) {
						if (UInt256Eq(&(*it)->getPrevBlockHash(), &prevBlockHash)) {
							next = *it;
							_orphans.erase(it);
							break;
						}
					}
				}

				saveBlocks.reserve(saveCount);

				for (i = 0, b = block; b && i < saveCount; i++) {
					assert(b->getHeight() != BLOCK_UNKNOWN_HEIGHT); // verify all blocks to be saved are in the chain
					if (_blocks.Get(b->getPrevBlockHash()))
						saveBlocks.push_back(b);
					b = _blocks.Get(b->getPrevBlockHash());
				}

				// make sure the set of blocks to be saved starts at a difficulty interval
				j = (i > 0) ? saveBlocks[i - 1]->getHeight() % BLOCK_DIFFICULTY_INTERVAL : 0;
				if (j > 0) i -= (i > BLOCK_DIFFICULTY_INTERVAL - j) ? BLOCK_DIFFICULTY_INTERVAL - j : i;
				assert(i == 0 || (saveBlocks[i - 1]->getHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0);
			}

			if (i > 0) fireSaveBlocks(i > 1, saveBlocks);

			if (block && block->getHeight() != BLOCK_UNKNOWN_HEIGHT && block->getHeight() >= peer->GetLastBlock()) {
				fireTxStatusUpdate(); // notify that transaction confirmations may have changed
			}

			if (next) OnRelayedBlock(peer, next);
		}

		void PeerManager::OnRelayedPingMsg(const PeerPtr &peer) {
			fireSyncIsInactive(_reconnectSeconds);
		}

		void PeerManager::OnNotfound(const PeerPtr &peer, const std::vector<UInt256> &txHashes,
									 const std::vector<UInt256> &blockHashes) {
			boost::mutex::scoped_lock scopedLock(lock);
			for (size_t i = 0; i < txHashes.size(); i++) {
				removePeerFromList(peer, txHashes[i], _txRelays);
				removePeerFromList(peer, txHashes[i], _txRequests);
			}
		}

		void PeerManager::OnSetFeePerKb(const PeerPtr &peer, uint64_t feePerKb) {
			uint64_t maxFeePerKb = 0, secondFeePerKb = 0;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = _connectedPeers.size(); i > 0; i--) { // find second highest fee rate
					const PeerPtr &p = _connectedPeers[i - 1];
					if (p->GetConnectStatus() != Peer::Connected) continue;
					if (p->getFeePerKb() > maxFeePerKb) secondFeePerKb = maxFeePerKb, maxFeePerKb = p->getFeePerKb();
				}

				if (secondFeePerKb * 3 / 2 > DEFAULT_FEE_PER_KB && secondFeePerKb * 3 / 2 <= MAX_FEE_PER_KB &&
					secondFeePerKb * 3 / 2 > _wallet->getFeePerKb(Asset::GetELAAssetID())) {
					peer->info("increasing feePerKb to {} based on feefilter messages from peers",
							   secondFeePerKb * 3 / 2);
					_wallet->setFeePerKb(Asset::GetELAAssetID(), secondFeePerKb * 3 / 2);
				}
			}
		}

		const TransactionPtr &PeerManager::OnRequestedTx(const PeerPtr &peer, const UInt256 &txHash) {
			int hasPendingCallbacks = 0, error = 0;
			PublishedTransaction pubTx;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = _publishedTx.size(); i > 0; i--) {
					if (UInt256Eq(&_publishedTxHashes[i - 1], &txHash)) {
						pubTx = _publishedTx[i - 1];
//						_publishedTx[i - 1].ResetCallback();
					} else if (_publishedTx[i - 1].HasCallback()) hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (_syncStartHeight == 0 || peer != _downloadPeer)) {
					peer->scheduleDisconnect(-1); // cancel publish tx timeout
				}

				addPeerToList(peer, txHash, _txRelays);
				if (pubTx.GetTransaction() != nullptr) _wallet->registerTransaction(pubTx.GetTransaction());
				if (pubTx.GetTransaction() != nullptr && !_wallet->transactionIsValid(pubTx.GetTransaction()))
					error = EINVAL;
			}

			if (error && pubTx.HasCallback()) pubTx.FireCallback(error, "tx not signed");
			return pubTx.GetTransaction();
		}

		bool PeerManager::OnNetworkIsReachable(const PeerPtr &peer) {
			return fireNetworkIsReachable();
		}

		void PeerManager::OnThreadCleanup(const PeerPtr &peer) {
		}

		void PeerManager::publishPendingTx(const PeerPtr &peer) {
			for (size_t i = _publishedTx.size(); i > 0; i--) {
				if (!_publishedTx[i - 1].HasCallback()) continue;
				peer->scheduleDisconnect(PROTOCOL_TIMEOUT);  // schedule publish timeout
				break;
			}

			InventoryParameter inventoryParameter;
			inventoryParameter.txHashes = _publishedTxHashes;
			peer->SendMessage(MSG_INV, inventoryParameter);
		}

		const std::vector<PublishedTransaction> PeerManager::getPublishedTransaction() const {
			std::vector<PublishedTransaction> publishedTx;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				publishedTx = _publishedTx;
			}
			return publishedTx;
		}

		const std::vector<UInt256> PeerManager::getPublishedTransactionHashes() const {
			std::vector<UInt256> publishedTxHashes;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				publishedTxHashes = _publishedTxHashes;
			}
			return publishedTxHashes;
		}

		int PeerManager::ReconnectTaskCount() const {
			return _reconnectTaskCount;
		}

		void PeerManager::SetReconnectTaskCount(int count) {
			boost::mutex::scoped_lock scopedLock(lock);
			_reconnectTaskCount = count;
		}

		size_t
		PeerManager::addPeerToList(const PeerPtr &peer, const UInt256 &txHash, std::vector<TransactionPeerList> &list) {
			for (size_t i = list.size(); i > 0; i--) {
				if (!UInt256Eq(&list[i - 1].GetTransactionHash(), &txHash)) continue;

				for (size_t j = list[i - 1].GetPeers().size(); j > 0; j--) {
					if (list[i - 1].GetPeers()[j - 1]->IsEqual(peer.get()))
						return list[i - 1].GetPeers().size();
				}

				list[i - 1].AddPeer(peer);
				return list[i - 1].GetPeers().size();
			}

			list.push_back(TransactionPeerList(txHash, {peer}));
			return 1;
		}

		bool PeerManager::removePeerFromList(const PeerPtr &peer, const UInt256 &txHash,
											 std::vector<TransactionPeerList> &list) {
			for (size_t i = list.size(); i > 0; i--) {
				if (!UInt256Eq(&list[i - 1].GetTransactionHash(), &txHash)) continue;

				for (size_t j = list[i - 1].GetPeers().size(); j > 0; j--) {
					if (!list[i - 1].GetPeers()[j - 1]->IsEqual(peer.get())) continue;
					list[i - 1].RemovePeerAt(j - 1);
					return true;
				}

				break;
			}

			return false;
		}

		void PeerManager::peerMisbehaving(const PeerPtr &peer) {
			for (size_t i = _peers.size(); i > 0; i--) {
				if (_peers[i - 1] == peer->GetPeerInfo())
					_peers.erase(_peers.begin() + i - 1);
			}

			if (++_misbehavinCount >= 10) { // clear out stored peers so we get a fresh list from DNS for next connect
				_misbehavinCount = 0;
				_peers.clear();
			}

			peer->Disconnect();
		}

		const PluginType &PeerManager::GetPluginType() const {
			return _pluginType;
		}

		const std::vector<PeerInfo> &PeerManager::GetPeers() const {
			return _peers;
		}

		void PeerManager::SetPeers(const std::vector<PeerInfo> &peers) {
			_peers = peers;
		}

		void PeerManager::updateBloomFilter() {

			if (_downloadPeer && (_downloadPeer->GetFlags() & PEER_FLAG_NEEDSUPDATE) == 0) {
				_downloadPeer->SetNeedsFilterUpdate(true);
				_downloadPeer->SetFlags(_downloadPeer->GetFlags() | PEER_FLAG_NEEDSUPDATE);
				_downloadPeer->info("filter update needed, waiting for pong");
				// wait for pong so we're sure to include any tx already sent by the peer in the updated filter
				PingParameter pingParameter;
				pingParameter.callback = boost::bind(&PeerManager::updateFilterPingDone, this, _downloadPeer, _1);
				_downloadPeer->SendMessage(MSG_PING, pingParameter);
			}
		}

		void PeerManager::updateFilterPingDone(const PeerPtr &peer, int success) {
			if (!success) return;

			boost::mutex::scoped_lock scopedLock(lock);
			peer->info("updating filter with newly created wallet addresses");
			_bloomFilter = nullptr;

			PingParameter pingParameter;
			if (_lastBlock->getHeight() < _estimatedHeight) { // if we're syncing, only update download peer
				if (_downloadPeer) {
					loadBloomFilter(_downloadPeer);
					pingParameter.callback = boost::bind(&PeerManager::updateFilterPingDone, this, _downloadPeer,
														 _1);
					_downloadPeer->SendMessage(MSG_PING, pingParameter);// wait for pong so filter is loaded
				}
			} else {
				for (size_t i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->GetConnectStatus() != Peer::Connected) continue;
					pingParameter.callback = boost::bind(&PeerManager::updateFilterPingDone, this,
														 _connectedPeers[i - 1],
														 _1);
					loadBloomFilter(peer);
					_downloadPeer->SendMessage(MSG_PING, pingParameter);// wait for pong so filter is loaded
				}
			}
		}

		void PeerManager::loadBloomFilterDone(const PeerPtr &peer, int success) {

			lock.lock();
			if (success) {
				MempoolParameter mempoolParameter;
				mempoolParameter.KnownTxHashes = _publishedTxHashes;
				mempoolParameter.CompletionCallback = boost::bind(&PeerManager::mempoolDone, this, peer, _1);
				peer->SendMessage(MSG_MEMPOOL, mempoolParameter);
				lock.unlock();
			} else {

				if (peer == _downloadPeer) {
					peer->info("sync succeeded");
					_keepAliveTimestamp = time(nullptr);
					_syncSucceeded = true;
					syncStopped();
					lock.unlock();
					fireSyncStopped(0);
				} else lock.unlock();
			}
		}

		std::vector<UInt128> PeerManager::addressLookup(const std::string &hostname) {
			struct addrinfo hints, *servinfo, *p;
			std::vector<UInt128> addrList;

			memset(&hints, 0, sizeof(hints));
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_family = PF_UNSPEC;
			if (getaddrinfo(hostname.c_str(), NULL, &hints, &servinfo) == 0) {
				for (p = servinfo; p != NULL; p = p->ai_next) {
					UInt128 addr;
					memset(&addr, 0, sizeof(addr));
					char host[INET6_ADDRSTRLEN];
					if (p->ai_family == AF_INET) {
						addr.u16[5] = 0xffff;
						addr.u32[3] = ((struct sockaddr_in *) p->ai_addr)->sin_addr.s_addr;
						inet_ntop(AF_INET, &addr.u32[3], host, sizeof(host));
					} else if (p->ai_family == AF_INET6) {
						addr = *(UInt128 *) &((struct sockaddr_in6 *) p->ai_addr)->sin6_addr;
						inet_ntop(AF_INET6, &addr, host, sizeof(host));
					}
					Log::debug("{} -> {}", hostname, host);
					addrList.push_back(addr);
				}

				freeaddrinfo(servinfo);
			}

			addrList.shrink_to_fit();
			return addrList;
		}

		bool PeerManager::verifyBlock(const MerkleBlockPtr &block, const MerkleBlockPtr &prev, const PeerPtr &peer) {
			bool r = true;

			if (!prev || !UInt256Eq(&block->getPrevBlockHash(), &prev->getHash()) ||
				block->getHeight() != prev->getHeight() + 1)
				r = false;

			// check if we hit a difficulty transition, and find previous transition time
			if (r && (block->getHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0) {
				MerkleBlockPtr b = block;
				UInt256 prevBlock;

				for (uint32_t i = 0; b && i < BLOCK_DIFFICULTY_INTERVAL; i++) {
					b = _blocks.Get(b->getPrevBlockHash());
				}

				if (!b) {
					peer->warn("missing previous difficulty tansition, can't verify block: {}",
							   Utils::UInt256ToString(block->getHash(), true));
					r = false;
				} else prevBlock = b->getPrevBlockHash();

				while (b) { // free up some memory
					b = _blocks.Get(prevBlock);
					if (b) prevBlock = b->getPrevBlockHash();

					if (b && (b->getHeight() % BLOCK_DIFFICULTY_INTERVAL) != 0) {
						_blocks.Remove(b);
					}
				}
			}

			//fixme figure out why difficult validation fails occasionally
			if (0 && verifyDifficulty(_chainParams, block, _blocks)) {
				peer->error("relayed block with invalid difficulty target {}, blockHash: {}", block->getTarget(),
							Utils::UInt256ToString(block->getHash()));
				r = false;
			}

			if (r) {
				const MerkleBlockPtr &checkpoint = _checkpoints.Get(block->getHash());

				// verify blockchain checkpoints
				if (checkpoint && !block->isEqual(checkpoint.get())) {
					peer->warn("relayed a block that differs from the checkpoint at height {}, blockHash: {}, "
							   "expected: {}", block->getHeight(), Utils::UInt256ToString(block->getHash(), true),
							   Utils::UInt256ToString(checkpoint->getHash(), true));
					r = false;
				}
			}

			return r;
		}

		std::vector<UInt256> PeerManager::getBlockLocators() {
			// append 10 most recent block hashes, decending, then continue appending, doubling the step back each time,
			// finishing with the genesis block (top, -1, -2, -3, -4, -5, -6, -7, -8, -9, -11, -15, -23, -39, -71, -135, ..., 0)
			MerkleBlockPtr block = _lastBlock;
			int32_t step = 1, i = 0, j;

			std::vector<UInt256> locators;
			while (block != nullptr && block->getHeight() > 0) {
				locators.push_back(block->getHash());
				if (++i >= 10) step *= 2;

				for (j = 0; block && j < step; j++) {
					block = _blocks.Get(block->getPrevBlockHash());
				}
			}

			locators.push_back(_chainParams.GetFirstCheckpoint().GetHash());
			return locators;
		}

		void PeerManager::loadMempools() {
			// after syncing, load filters and get mempools from other peers
			for (size_t i = _connectedPeers.size(); i > 0; i--) {
				const PeerPtr &peer = _connectedPeers[i - 1];

				if (peer->GetConnectStatus() != Peer::Connected) continue;

				if (peer != _downloadPeer || _fpRate > BLOOM_REDUCED_FALSEPOSITIVE_RATE * 5.0) {
					loadBloomFilter(peer);
					publishPendingTx(peer);
					PingParameter pingParameter;
					pingParameter.callback = boost::bind(&PeerManager::loadBloomFilterDone, this, peer, _1);
					peer->SendMessage(MSG_PING, pingParameter);
				} else {
					MempoolParameter mempoolParameter;
					mempoolParameter.KnownTxHashes = _publishedTxHashes;
					mempoolParameter.CompletionCallback = boost::bind(&PeerManager::loadBloomFilterDone, this, peer,
																	  _1);
					peer->SendMessage(MSG_MEMPOOL, mempoolParameter);
				}
			}
		}

		void PeerManager::mempoolDone(const PeerPtr &peer, int success) {
			bool syncFinished = false;

			if (success) {
				peer->info("mempool request finished");

				{
					boost::mutex::scoped_lock scopedLock(lock);
					if (_syncStartHeight > 0) {
						peer->info("sync succeeded");
						_keepAliveTimestamp = time(nullptr);
						_syncSucceeded = true;
						syncFinished = true;
						syncStopped();
					}

					ResendUnconfirmedTx(peer);
//					requestUnrelayedTx(peer);
					peer->SendMessage(MSG_GETADDR, Message::DefaultParam);
				}

				fireTxStatusUpdate();
				if (syncFinished) fireSyncStopped(0);
			} else peer->info("mempool request failed");
		}

		void PeerManager::ResendUnconfirmedTx(const PeerPtr &peer) {
			std::vector<TransactionPtr> tx = _wallet->TxUnconfirmedBefore(TX_UNCONFIRMED);
			std::vector<UInt256> txHashes;

			for (size_t i = 0; i < tx.size(); i++) {
				if (!peerListHasPeer(_txRelays, tx[i]->getHash(), peer) &&
					!peerListHasPeer(_txRequests, tx[i]->getHash(), peer)) {
					txHashes.push_back(tx[i]->getHash());
					addPeerToList(peer, tx[i]->getHash(), _txRequests);
				}
			}

			if (!txHashes.empty()) {
				lock.unlock();

				for (size_t i = 0; i < tx.size(); ++i) {
					publishTransaction(tx[i]);
				}

				lock.lock();

				if ((peer->GetPeerInfo().Flags & PEER_FLAG_SYNCED) == 0) {
					PingParameter pingParameter;
					pingParameter.callback = boost::bind(&PeerManager::requestUnrelayedTxGetDataDone, this, peer, _1);
					peer->SendMessage(MSG_PING, pingParameter);
				}
			} else peer->SetFlags(peer->GetFlags() | PEER_FLAG_SYNCED);
		}

		void PeerManager::requestUnrelayedTx(const PeerPtr &peer) {
			std::vector<TransactionPtr> tx = _wallet->TxUnconfirmedBefore(TX_UNCONFIRMED);
			std::vector<UInt256> txHashes;

			for (size_t i = 0; i < tx.size(); i++) {
				if (!peerListHasPeer(_txRelays, tx[i]->getHash(), peer) &&
					!peerListHasPeer(_txRequests, tx[i]->getHash(), peer)) {
					txHashes.push_back(tx[i]->getHash());
					addPeerToList(peer, tx[i]->getHash(), _txRequests);
				}
			}

			if (!txHashes.empty()) {
				GetDataParameter getDataParameter(txHashes, {});
				peer->SendMessage(MSG_GETDATA, getDataParameter);

				if ((peer->GetPeerInfo().Flags & PEER_FLAG_SYNCED) == 0) {
					PingParameter pingParameter;
					pingParameter.callback = boost::bind(&PeerManager::requestUnrelayedTxGetDataDone, this, peer, _1);
					peer->SendMessage(MSG_PING, pingParameter);
				}
			} else peer->SetFlags(peer->GetFlags() | PEER_FLAG_SYNCED);
		}

		bool PeerManager::peerListHasPeer(const std::vector<TransactionPeerList> &peerList, const UInt256 &txhash,
										  const PeerPtr &peer) {
			for (size_t i = peerList.size(); i > 0; i--) {
				if (!UInt256Eq(&peerList[i - 1].GetTransactionHash(), &txhash)) continue;

				for (size_t j = peerList[i - 1].GetPeers().size(); j > 0; j--) {
					if (peerList[i - 1].GetPeers()[j - 1]->IsEqual(peer.get())) return true;
				}

				break;
			}

			return false;
		}

		void PeerManager::requestUnrelayedTxGetDataDone(const PeerPtr &callbackPeer, int success) {
			bool isPublishing;
			size_t count = 0;
			PeerPtr peer = callbackPeer;

			boost::mutex::scoped_lock scopedLock(lock);
			if (success) peer->SetFlags(peer->GetFlags() | PEER_FLAG_SYNCED);

			for (size_t i = _connectedPeers.size(); i > 0; i--) {
				peer = _connectedPeers[i - 1];
				if (peer->GetConnectStatus() == Peer::Connected) count++;
				if ((peer->GetPeerInfo().Flags & PEER_FLAG_SYNCED) != 0) continue;
				count = 0;
				break;
			}

			// don't remove transactions until we're connected to maxConnectCount peers, and all peers have finished
			// relaying their mempools
			if (count >= _maxConnectCount) {
				UInt256 hash;
				std::vector<TransactionPtr> tx = _wallet->TxUnconfirmedBefore(TX_UNCONFIRMED);

				for (size_t i = tx.size(); i > 0; i--) {
					hash = tx[i - 1]->getHash();
					isPublishing = false;

					for (size_t j = _publishedTx.size(); !isPublishing && j > 0; j--) {
						if (_publishedTx[j - 1].GetTransaction()->IsEqual(tx[i - 1].get()) &&
							_publishedTx[j - 1].HasCallback())
							isPublishing = true;
					}

					if (!isPublishing && PeerListCount(_txRelays, hash) == 0 &&
						PeerListCount(_txRequests, hash) == 0) {
						peer->info("removing tx unconfirmed at: {}, txHash: {}", _lastBlock->getHeight(),
								   Utils::UInt256ToString(hash, true));
						assert(tx[i - 1]->getBlockHeight() == TX_UNCONFIRMED);
						_wallet->removeTransaction(hash);
					} else if (!isPublishing && PeerListCount(_txRelays, hash) < _maxConnectCount) {
						// set timestamp 0 to mark as unverified
						_wallet->updateTransactions({hash}, TX_UNCONFIRMED, 0);
					}
				}
			}
		}

		size_t PeerManager::PeerListCount(const std::vector<TransactionPeerList> &list, const UInt256 &txhash) {
			for (size_t i = list.size(); i > 0; i--) {
				if (UInt256Eq(&list[i - 1].GetTransactionHash(), &txhash)) return list[i - 1].GetPeers().size();
			}

			return 0;
		}

		void PeerManager::publishTxInivDone(const PeerPtr &peer, int success) {
			boost::mutex::scoped_lock scopedLock(lock);
			requestUnrelayedTx(peer);
		}

	}
}