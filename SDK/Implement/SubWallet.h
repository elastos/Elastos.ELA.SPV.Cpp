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

			virtual const std::string &GetInfoChainID() const;

		public: //implement ISubWallet
			virtual std::string GetChainID() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual nlohmann::json GetBalanceInfo() const;

			virtual uint64_t GetBalance(BalanceType type = Default) const;

			virtual uint64_t GetBalanceWithAddress(const std::string &address, BalanceType type = Default) const;

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(uint32_t start,
												 uint32_t count) const;

			virtual void AddCallback(ISubWalletCallback *subCallback);

			virtual void RemoveCallback(ISubWalletCallback *subCallback);

			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					const std::string &memo,
					bool useVotedUTXO = false);

			virtual nlohmann::json CreateCombineUTXOTransaction(
					const std::string &memo,
					bool useVotedUTXO = false);

			virtual nlohmann::json SignTransaction(
					const nlohmann::json &rawTransaction,
					const std::string &payPassword);

			virtual nlohmann::json GetTransactionSignedSigners(
					const nlohmann::json &rawTransaction) const;

			virtual nlohmann::json PublishTransaction(
					const nlohmann::json &rawTransaction);

			virtual nlohmann::json GetAllTransaction(
					uint32_t start,
					uint32_t count,
					const std::string &addressOrTxid) const;

			virtual nlohmann::json GetAllCoinBaseTransaction(
				uint32_t start,
				uint32_t count,
				const std::string &txID) const;

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword);

			virtual bool CheckSign(
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature);

			virtual nlohmann::json GetAssetInfo(
					const std::string &assetID) const;

			virtual std::string GetPublicKey() const;

			virtual nlohmann::json EncodeTransaction(const nlohmann::json &tx) const;

			virtual nlohmann::json DecodeTransaction(const nlohmann::json &encodedTx) const;

		protected: //implement Wallet::Listener
			virtual void balanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onCoinBaseTxAdded(const CoinBaseUTXOPtr &cb);

			virtual void onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			virtual void onCoinBaseSpent(const std::vector<uint256> &spentHashes);

			virtual void onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

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

		protected:
			friend class MasterWallet;

			SubWallet(const CoinInfoPtr &info,
					  const ChainConfigPtr &config,
					  MasterWallet *parent);

			virtual TransactionPtr CreateTx(
				const std::string &fromAddress,
				const std::vector<TransactionOutput> &outputs,
				const std::string &memo,
				bool useVotedUTXO = false) const;

			virtual void publishTransaction(const TransactionPtr &transaction);

			bool filterByAddressOrTxId(const TransactionPtr &transaction, const std::string &addressOrTxid) const;

			virtual void fireTransactionStatusChanged(const uint256 &txid, const std::string &status,
													  const nlohmann::json &desc, uint32_t confirms);

			const CoinInfoPtr &GetCoinInfo() const;

		protected:
			WalletManagerPtr _walletManager;
			std::vector<ISubWalletCallback *> _callbacks;
			MasterWallet *_parent;
			CoinInfoPtr _info;
			ChainConfigPtr _config;
			SubAccountPtr _subAccount;

			uint32_t _syncStartHeight;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
