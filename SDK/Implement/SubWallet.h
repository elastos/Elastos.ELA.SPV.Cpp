// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SUBWALLET_H__
#define __ELASTOS_SDK_SUBWALLET_H__

#include <SDK/P2P/ChainParams.h>
#include <SDK/SpvService/SpvService.h>
#include <SDK/Account/SubAccount.h>

#include <Interface/ISubWallet.h>
#include <Interface/ISubWalletCallback.h>

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

namespace Elastos {
	namespace ElaWallet {

#define SELA_PER_ELA 100000000

		class MasterWallet;
		class Transaction;
		class ChainConfig;
		class CoinInfo;

		typedef boost::shared_ptr<Transaction> TransactionPtr;
		typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;
		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;

		class SubWallet : public virtual ISubWallet,
						  public Wallet::Listener,
						  public PeerManager::Listener,
						  public Lockable {
		public:
			typedef boost::shared_ptr<SpvService> WalletManagerPtr;

			virtual ~SubWallet();

			const WalletManagerPtr &GetWalletManager() const;

			void StartP2P();

			void StopP2P();

			void FlushData();

			virtual const std::string &GetInfoChainID() const;

		public: //implement ISubWallet
			virtual std::string GetChainID() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual nlohmann::json GetBalanceInfo() const;

			virtual std::string GetBalance() const;

			virtual std::string GetBalanceWithAddress(const std::string &address) const;

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(uint32_t start,
												 uint32_t count) const;

			virtual nlohmann::json GetAllPublicKeys(uint32_t start, uint32_t count) const;

			virtual void AddCallback(ISubWalletCallback *subCallback);

			virtual void RemoveCallback(ISubWalletCallback *subCallback);

			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const std::string &amount,
					const std::string &memo);

			virtual nlohmann::json GetAllUTXOs(uint32_t start, uint32_t count, const std::string &address) const;

			virtual nlohmann::json CreateConsolidateTransaction(
					const std::string &memo);

			virtual nlohmann::json SignTransaction(
					const nlohmann::json &createdTx,
					const std::string &payPassword);

			virtual nlohmann::json GetTransactionSignedInfo(
					const nlohmann::json &rawTransaction) const;

			virtual nlohmann::json PublishTransaction(
					const nlohmann::json &signedTx);

			virtual nlohmann::json GetAllTransaction(
					uint32_t start,
					uint32_t count,
					const std::string &addressOrTxid) const;

			virtual nlohmann::json GetAllCoinBaseTransaction(
				uint32_t start,
				uint32_t count,
				const std::string &txID) const;

			virtual nlohmann::json GetAssetInfo(
					const std::string &assetID) const;

			virtual void SyncStart();

			virtual void SyncStop();

		protected: //implement Wallet::Listener
			virtual void balanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onCoinBaseTxAdded(const UTXOPtr &cb);

			virtual void onCoinBaseUpdatedAll(const UTXOArray &cbs);

			virtual void onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			virtual void onCoinBaseSpent(const std::vector<uint256> &spentHashes);

			virtual void onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxUpdatedAll(const std::vector<TransactionPtr> &txns);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

		protected: //implement PeerManager::Listener
			virtual void syncStarted();

			virtual void syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime);

			// func syncStopped(_ error: BRPeerManagerError?)
			virtual void syncStopped(const std::string &error);

			// func txStatusUpdate()
			virtual void txStatusUpdate() {}

			// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			// func savePeers(_ replace: Bool, _ peers: [BRPeer])
			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers) {}

			// func networkIsReachable() -> Bool}
			virtual bool networkIsReachable() { return true; }

			// Called on publishTransaction
			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void syncIsInactive(uint32_t time) {}

			virtual void connectStatusChanged(const std::string &status);

		protected:
			friend class MasterWallet;

			SubWallet(const CoinInfoPtr &info,
					  const ChainConfigPtr &config,
					  MasterWallet *parent);

			TransactionPtr CreateTx(
				uint8_t type,
				const PayloadPtr &payload,
				const std::string &fromAddress,
				const std::vector<OutputPtr> &outputs,
				const std::string &memo,
				bool max = false) const;

			TransactionPtr CreateConsolidateTx(
				const std::string &memo,
				const uint256 &asset) const;

			virtual void publishTransaction(const TransactionPtr &tx);

			bool filterByAddressOrTxId(const TransactionPtr &transaction, const std::string &addressOrTxid) const;

			virtual void fireTransactionStatusChanged(const uint256 &txid, const std::string &status,
													  const nlohmann::json &desc, uint32_t confirms);

			const CoinInfoPtr &GetCoinInfo() const;

			void EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const;

			TransactionPtr DecodeTx(const nlohmann::json &encodedTx) const;

		protected:
			WalletManagerPtr _walletManager;
			std::vector<ISubWalletCallback *> _callbacks;
			MasterWallet *_parent;
			CoinInfoPtr _info;
			ChainConfigPtr _config;
			SubAccountPtr _subAccount;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
