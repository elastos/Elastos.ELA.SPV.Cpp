// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONHUB_H__
#define __ELASTOS_SDK_TRANSACTIONHUB_H__

#include <SDK/TransactionHub/GroupedAssetTransactions.h>
#include <SDK/TransactionHub/UTXOList.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Common/Lockable.h>
#include <SDK/Common/ElementSet.h>
#include <SDK/BIPs/Address.h>
#include <SDK/Account/ISubAccount.h>

#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <map>

#define TX_FEE_PER_KB        1000ULL     // standard tx fee per kb of tx size, rounded up to nearest kb
#define TX_OUTPUT_SIZE       34          // estimated size for a typical transaction output
#define TX_INPUT_SIZE        148         // estimated size for a typical compact pubkey transaction input
#define TX_MAX_SIZE          100000      // no tx can be larger than this size in bytes
#define TX_UNCONFIRMED INT32_MAX
#define DEFAULT_FEE_PER_KB (10000)                  // 10 satoshis-per-byte
#define MIN_FEE_PER_KB     TX_FEE_PER_KB                       // bitcoind 0.12 default min-relay fee
#define MAX_FEE_PER_KB     ((TX_FEE_PER_KB*1000100 + 190)/191) // slightly higher than a 10,000bit fee on a 191byte tx
#define TX_MAX_LOCK_HEIGHT   500000000   // a lockTime below this value is a block height, otherwise a timestamp
#define TX_MIN_OUTPUT_AMOUNT (TX_FEE_PER_KB*3*(TX_OUTPUT_SIZE + TX_INPUT_SIZE)/1000) //no txout can be below this amount

namespace Elastos {
	namespace ElaWallet {

		class TransactionHub : public Lockable {

		public:

			TransactionHub(const std::vector<Asset> &assetArray,
				const std::vector<TransactionPtr> &transactions,
				   const SubAccountPtr &subAccount,
				   const boost::shared_ptr<AssetTransactions::Listener> &listener);

			virtual ~TransactionHub();

			void InitListeningAddresses(const std::vector<std::string> &addrs);

			const std::string &GetWalletID() const;

			void SetWalletID(const std::string &walletID);

			void SetBlockHeight(uint32_t height);

			uint32_t GetBlockHeight() const;

			nlohmann::json GetBalanceInfo();

			void RegisterRemark(const TransactionPtr &transaction);

			std::string GetRemark(const std::string &txHash);

			uint64_t GetBalanceWithAddress(const uint256 &assetID, const Address &address, AssetTransactions::BalanceType type) const;

			// returns the first unused external address
			Address GetReceiveAddress() const;

			size_t GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal);

			Address GetVoteDepositAddress() const;

			bool IsVoteDepositAddress(const Address &addr) const;

			// true if the address was previously generated by BRWalletUnusedAddrs() (even if it's now used)
			bool ContainsAddress(const Address &address);

			// true if the address was previously used as an input or output in any wallet transaction
			bool AddressIsUsed(const Address &address);

			uint64_t GetBalance(const uint256 &assetID, AssetTransactions::BalanceType type) const;

			uint64_t GetFeePerKb(const uint256 &assetID) const;

			void SetFeePerKb(const uint256 &assetID, uint64_t fee);

			uint64_t GetDefaultFeePerKb();

			void UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress);

			TransactionPtr
			CreateTransaction(const Address &fromAddress, uint64_t amount,
							  const Address &toAddress, uint64_t fee,
							  const uint256 &assetID, bool useVotedUTXO);

			bool ContainsTransaction(const TransactionPtr &transaction);

			bool RegisterTransaction(const TransactionPtr &tx);

			void RemoveTransaction(const uint256 &transactionHash);

			void UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight, uint32_t timestamp);

			TransactionPtr TransactionForHash(const uint256 &transactionHash);

			std::vector<TransactionPtr> GetAllTransactions() const;

			bool TransactionIsValid(const TransactionPtr &transaction);

			bool TransactionIsPending(const TransactionPtr &transaction);

			bool TransactionIsVerified(const TransactionPtr &transaction);

			uint64_t GetTransactionAmount(const TransactionPtr &tx);

			uint64_t GetTransactionFee(const TransactionPtr &tx);

			uint64_t GetTransactionAmountSent(const TransactionPtr &tx);

			uint64_t GetTransactionAmountReceived(const TransactionPtr &tx);

			void SignTransaction(const TransactionPtr &tx, const std::string &payPassword);

			void UpdateBalance();

			std::vector<UTXO> GetUTXOsSafe(const uint256 &assetID) const;

			std::vector<UTXO> GetAllUTXOsSafe();

			std::vector<TransactionPtr> TxUnconfirmedBefore(uint32_t blockHeight);

			void SetTxUnconfirmedAfter(uint32_t blockHeight);

			const std::vector<std::string> &GetListeningAddrs() const;

			std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal);

			void UpdateAssets(const std::vector<Asset> &assetArray);

			nlohmann::json GetAllSupportedAssets() const;

			bool ContainsAsset(const std::string &assetID);

		protected:

			uint64_t WalletFeeForTx(const TransactionPtr &tx);

		protected:
			std::string _walletID;
			GroupedAssetTransactions _transactions;

			typedef std::map<std::string, std::string> TransactionRemarkMap;
			TransactionRemarkMap _txRemarkMap;
			std::vector<std::string> _listeningAddrs;

			SubAccountPtr _subAccount;
		};

		typedef boost::shared_ptr<TransactionHub> WalletPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONHUB_H__
