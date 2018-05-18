// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVCLIENT_WALLET_H__
#define __ELASTOS_SDK_SPVCLIENT_WALLET_H__

#include <string>
#include <BRWallet.h>
#include <boost/weak_ptr.hpp>

#include "BRInt.h"

#include "Wrapper.h"
#include "Transaction.h"
#include "Address.h"
#include "SharedWrapperList.h"
#include "MasterPubKey.h"
#include "TransactionOutput.h"
#include "WrapperList.h"

namespace Elastos {
	namespace SDK {

		class Wallet :
				public Wrapper<BRWallet> {

		public:
			class Listener {
			public:
				// func balanceChanged(_ balance: UInt64)
				virtual void balanceChanged(uint64_t balance) = 0;

				// func txAdded(_ tx: BRTxRef)
				virtual void onTxAdded(Transaction *transaction) = 0;

				// func txUpdated(_ txHashes: [UInt256], blockHeight: UInt32, timestamp: UInt32)
				virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) = 0;

				// func txDeleted(_ txHash: UInt256, notifyUser: Bool, recommendRescan: Bool)
				virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) = 0;
			};

		public:
			Wallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
				   const MasterPubKeyPtr &masterPubKey,
				   const boost::shared_ptr<Listener> &listener);

			~Wallet();

			virtual std::string toString() const;

			virtual BRWallet *getRaw() const;

			// returns the first unused external address
			std::string getReceiveAddress() const;

			// writes all addresses previously genereated with BRWalletUnusedAddrs() to addrs
			// returns the number addresses written, or total number available if addrs is NULL
			std::vector<std::string> getAllAddresses();

			// true if the address was previously generated by BRWalletUnusedAddrs() (even if it's now used)
			// int BRWalletContainsAddress(BRWallet *wallet, const char *addr);
			bool containsAddress(const std::string &address);

			// true if the address was previously used as an input or output in any wallet transaction
			// int BRWalletAddressIsUsed(BRWallet *wallet, const char *addr);
			bool addressIsUsed(const std::string &address);

			SharedWrapperList<Transaction, BRTransaction *> getTransactions() const;

			SharedWrapperList<Transaction, BRTransaction *> getTransactionsConfirmedBefore(uint32_t blockHeight) const;

			uint64_t getBalance() const;

			uint64_t getTotalSent();

			uint64_t getTotalReceived();

			uint64_t getFeePerKb();

			void setFeePerKb(uint64_t feePerKb);

			uint64_t getMaxFeePerKb();

			uint64_t getDefaultFeePerKb();

			/**
			 * Creates a Transaction for sending `amount` to `address`.  Will create a
			 * TransactionOutput for `address` with a script of:
			 *      DUP HASH160 <pub key hash for address> EQUALVERIFY CHECKSIG
			 * (provided `address` is a 'pub key address'; otherwise produces a script for a scriptAddress)
			 *
			 * Will iterate over the wallet's UTXOs adding in their transaction output (amount, script) as
			 * a BRCoreTransactionInput.  If the UTXOs can't cover `amount` then `null` is returned;
			 * otherwise a 'change' output is added for an unused wallet address.
			 *
			 * @param amount the amount to send
			 * @param address the address to send to
			 * @return a consistently constructed transaction.
			 */
			TransactionPtr createTransaction(uint64_t amount, const Address &address);

			/**
			 * Create a BRCoreTransaction with the provided outputs
			 *
			 * @param outputs the outputs to include
			 * @return a consistently constructed transaction (input selected, fees handled, etc)
			 */
			TransactionPtr createTransactionForOutputs(const WrapperList<TransactionOutput, BRTxOutput> &outputs);

			/**
			 * Sign `transaction` for the provided `forkId` (BTC or BCH) using `phrase`.  The `phrase` must
			 * be the 'paper key' used when the wallet's MasterPubKey was originally created.
			 *
			 * @param transaction
			 * @param forkId
			 * @param phrase
			 * @return
			 */
			bool signTransaction(const TransactionPtr &transaction, int forkId, const CMBlock &phrase);

			bool containsTransaction(const TransactionPtr &transaction);

			bool registerTransaction(const TransactionPtr &transaction);

			void removeTransaction(const UInt256 &transactionHash);

			void updateTransactions(const std::vector<UInt256> &transactionsHashes, uint32_t blockHeight,
									uint32_t timestamp);

			/**
			 * Returns the BRCoreTransaction with the provided `transactionHash` if it exists; otherwise
			 * NULL is returned.
			 *
			 * If BRCoreTransaction.JNI_COPIES_TRANSACTIONS is true, then the returned transaction is
			 * a copy of the Core BRTransaction - TBD caution when passed back into Core
			 *
			 * @param transactionHash
			 * @return
			 */
			TransactionPtr transactionForHash(const UInt256 &transactionHash);

			/**
			 * Check if a transaction is valid - THIS METHOD WILL FATAL if the transaction is not signed.
			 * You must call transaction.isSigned to avoid the FATAL.
		 	 *
			 * @param transaction
			 * @return
			 */
			bool transactionIsValid(const TransactionPtr &transaction);

			bool transactionIsPending(const TransactionPtr &transaction);

			bool transactionIsVerified(const TransactionPtr &transaction);

			/**
			 * Return the net amount received by this wallet.  If the amount is positive, then the balance
			 * of this wallet increased; if the amount is negative, then the balance decreased.
			 *
			 * @param tx
			 * @return
			 */
			uint64_t getTransactionAmount(const TransactionPtr &tx);

			uint64_t getTransactionFee(const TransactionPtr &tx);

			uint64_t getTransactionAmountSent(const TransactionPtr &tx);

			uint64_t getTransactionAmountReceived(const TransactionPtr &tx);

			uint64_t getBalanceAfterTransaction(const TransactionPtr &transaction);

			/**
			 * Return a BRCoreAddress for a) the receiver (if we sent an amount) or b) the sender (if
			 * we received an amount).  The returned address will be the first address that is not in
			 * this wallet from the outputs or the inputs, respectively.
			 *
			 * @param transaction
			 * @return
			 */
			std::string getTransactionAddress(const TransactionPtr &transaction);

			/**
			 * Return the first BRCoreAddress from the `transaction` inputs that is not an address
			 * in this wallet.
			 *
			 * @param transaction
			 * @return The/A BRCoreAddress that received an amount from us (that we sent to)
			 */
			std::string getTransactionAddressInputs(const TransactionPtr &transaction);

			/**
			 * Return the first BRCoreAddress from the `transaction` outputs this is not an address
			 * in this wallet.
			 *
			 * @param transaction
			 * @return The/A BRCoreAddress that sent to us.
			 */
			std::string getTransactionAddressOutputs(const TransactionPtr &transaction);

			uint64_t getFeeForTransactionSize(size_t size);

			uint64_t getFeeForTransactionAmount(uint64_t amount);

			uint64_t getMinOutputAmount();

			uint64_t getMaxOutputAmount();

		private:
			BRWallet *_wallet;

			boost::weak_ptr<Listener> _listener;
		};

		typedef boost::shared_ptr<Wallet> WalletPtr;

	}
}

#endif //__ELASTOS_SDK_SPVCLIENT_WALLET_H__
