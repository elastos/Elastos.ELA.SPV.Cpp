// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdlib.h>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <SDK/Common/Log.h>
#include <Core/BRAddress.h>
#include <Core/BRBIP32Sequence.h>
#include <Core/BRWallet.h>
#include <SDK/Common/ParamChecker.h>

#include "BRAddress.h"
#include "BRBIP39Mnemonic.h"
#include "BRArray.h"
#include "BRTransaction.h"

#include "Wallet.h"
#include "Utils.h"
#include "ErrorCode.h"
#include "Account/MultiSignSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		namespace {
			uint64_t _txFee(uint64_t feePerKb, size_t size) {
				uint64_t standardFee =
						((size + 999) / 1000) * TX_FEE_PER_KB, // standard fee based on tx size rounded up to nearest kb
						fee = (((size * feePerKb / 1000) + 99) / 100) *
							  100; // fee using feePerKb, rounded up to nearest 100 satoshi

				return (fee > standardFee) ? fee : standardFee;
			}
		}

		Wallet::Wallet() {

		}

		Wallet::Wallet(const std::vector<TransactionPtr> &txArray,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<Listener> &listener) :
				feePerKb(DEFAULT_FEE_PER_KB),
				_subAccount(subAccount) {

			for (size_t i = 0; txArray.size(); i++) {
				if (!txArray[i]->isSigned() || allTx.Contains(txArray[i])) continue;
				allTx.Insert(txArray[i]);
				transactions.push_back(txArray[i]);
			}
			sortTransations();

			_subAccount->InitAccount(transactions, this);
			WalletUpdateBalance();

			if (!transactions.empty() && !WalletContainsTx(transactions[0])) { // verify transactions match master pubKey
				std::stringstream ess;
				ess << "txCount = " << transactions.size()
					<< ", wallet do not contain tx[0] = "
					<< Utils::UInt256ToString(transactions[0]->getHash());
				Log::getLogger()->error(ess.str());
				throw std::logic_error(ess.str());
			}

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			for (std::vector<TransactionPtr>::const_iterator it = transactions.cbegin();
				 it != transactions.cend(); ++it) {
				(*it)->isRegistered() = true;
			}

			for (int i = 0; i < txArray.size(); ++i) {
				TxRemarkMap[Utils::UInt256ToString(txArray[i]->getHash())] = txArray[i]->getRemark();
			}
		}

		Wallet::~Wallet() {
		}

		void Wallet::initListeningAddresses(const std::vector<std::string> &addrs) {
			ListeningAddrs = addrs;
		}

		void Wallet::RegisterRemark(const TransactionPtr &transaction) {
			TxRemarkMap[Utils::UInt256ToString(transaction->getHash())] = transaction->getRemark();
		}

		std::string Wallet::GetRemark(const std::string &txHash) {
			if (TxRemarkMap.find(txHash) != TxRemarkMap.end())
				return "";
			return TxRemarkMap[txHash];
		}

		std::vector<UTXO> Wallet::getUTXOSafe() {
			std::vector<UTXO> result(utxos.size());

			{
				for (size_t i = 0; utxos.size(); i++) {
					result[i] = utxos[i];
				}
			}

			return result;
		}

		nlohmann::json Wallet::GetBalanceInfo() {
			std::vector<UTXO> utxos = getUTXOSafe();
			nlohmann::json j;
			std::map<std::string, uint64_t> addressesBalanceMap;

			{
				boost::mutex::scoped_lock scopedLock(lock);

				for (size_t i = 0; i < utxos.size(); ++i) {
					if (!allTx.Contains(utxos[i].hash)) continue;

					const TransactionPtr &t = allTx.GetTransaction(utxos[i].hash);
					if (addressesBalanceMap.find(t->getOutputs()[utxos[i].n].getAddress()) !=
						addressesBalanceMap.end()) {
						addressesBalanceMap[t->getOutputs()[utxos[i].n].getAddress()] += t->getOutputs()[utxos[i].n].getAmount();
					} else {
						addressesBalanceMap[t->getOutputs()[utxos[i].n].getAddress()] = t->getOutputs()[utxos[i].n].getAmount();
					}
				}
			}

			std::vector<nlohmann::json> balances;
			std::for_each(addressesBalanceMap.begin(), addressesBalanceMap.end(),
						  [&addressesBalanceMap, &balances](const std::map<std::string, uint64_t>::value_type &item) {
							  nlohmann::json balanceKeyValue;
							  balanceKeyValue[item.first] = item.second;
							  balances.push_back(balanceKeyValue);
						  });

			j["Balances"] = balances;
			return j;
		}

		uint64_t Wallet::GetBalanceWithAddress(const std::string &address) {
			std::vector<UTXO> utxos = getUTXOSafe();
//fixme [refactor] comlete me
//			ELATransaction *t;
//			uint64_t balance = 0;
//			pthread_mutex_lock(&_wallet->Raw.lock);
//			for (size_t i = 0; i < utxosCount; ++i) {
//				void *tempPtr = BRSetGet(_wallet->Raw.allTx, &utxos[i].hash);
//				if (tempPtr == nullptr) continue;
//				t = static_cast<ELATransaction *>(tempPtr);
//				if (t->outputs[utxos[i].n]->getAddress() == address) {
//					balance += t->outputs[utxos[i].n]->getAmount();
//				}
//			}
//			pthread_mutex_unlock(&_wallet->Raw.lock);
//
//			return balance;
			return 0;
		}

		std::vector<TransactionPtr> Wallet::getTransactions() const {
//fixme [refactor] comlete me
//			size_t transactionCount = BRWalletTransactions((BRWallet *) _wallet, NULL, 0);
//
//			BRTransaction **transactions = (BRTransaction **) calloc(transactionCount, sizeof(BRTransaction *));
//			transactionCount = BRWalletTransactions((BRWallet *) _wallet, transactions, transactionCount);
//
//			SharedWrapperList<Transaction, BRTransaction *> results(transactionCount);
//			// TODO: Decide if copy is okay; if not, be sure to mark 'isRegistered = true'
//			//   We should not copy; but we need to deal with wallet-initiated 'free'
//			for (int index = 0; index < transactionCount; index++) {
//				results.push_back(TransactionPtr(new Transaction(*(ELATransaction *) transactions[index])));
//			}
//
//			if (NULL != transactions) free(transactions);
//			return results;
			return std::vector<TransactionPtr>();
		}

		std::vector<TransactionPtr> Wallet::getTransactionsConfirmedBefore(uint32_t blockHeight) const {
//fixme [refactor] comlete me
//			size_t transactionCount = BRWalletTxUnconfirmedBefore((BRWallet *) _wallet, NULL, 0, blockHeight);
//
//			BRTransaction **transactions = (BRTransaction **) calloc(transactionCount, sizeof(BRTransaction *));
//			transactionCount = BRWalletTxUnconfirmedBefore((BRWallet *) _wallet, transactions, transactionCount,
//														   blockHeight);
//
//			SharedWrapperList<Transaction, BRTransaction *> results(transactionCount);
//			for (int index = 0; index < transactionCount; index++) {
//				results.push_back(TransactionPtr(new Transaction(*(ELATransaction *) transactions[index])));
//			}
//
//			if (NULL != transactions) free(transactions);
//			return results;
			return std::vector<TransactionPtr>();
		}

		uint64_t Wallet::getBalance() const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = balance;
			}
			return result;
		}

		uint64_t Wallet::getTotalSent() {
			uint64_t resutl;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				resutl = totalSent;
			}
			return resutl;
		}

		uint64_t Wallet::getTotalReceived() {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = totalReceived;
			}
			return result;
		}

		uint64_t Wallet::getFeePerKb() {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = feePerKb;
			}
			return result;
		}

		void Wallet::setFeePerKb(uint64_t fee) {
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				feePerKb = fee;
			}
		}

		uint64_t Wallet::getMaxFeePerKb() {
			return MAX_FEE_PER_KB;
		}

		uint64_t Wallet::getDefaultFeePerKb() {
			return DEFAULT_FEE_PER_KB;
		}

		bool Wallet::AddressFilter(const std::string &fromAddress, const std::string &filterAddress) {
			return filterAddress == fromAddress;
		}

		TransactionPtr Wallet::CreateTxForOutputs(const std::vector<TransactionOutput> &outputs, size_t outCount,
												  uint64_t fee, const std::string &fromAddress,
												  bool(*filter)(const std::string &fromAddress,
																const std::string &addr)) {
			//fixme [refactor] comlete me
			return nullptr;
//			ELATransaction *tx, *transaction = ELATransactionNew();
//			uint64_t feeAmount, amount = 0, balance = 0, minAmount;
//			size_t i, j, cpfpSize = 0;
//			BRUTXO *o;
//			BRAddress addr = BR_ADDRESS_NONE;
//
//			assert(wallet != NULL);
//			assert(outputs != NULL && outCount > 0);
//
//			for (i = 0; outputs && i < outCount; i++) {
//				assert(outputs[i].script != NULL && outputs[i].scriptLen > 0);
//				CMBlock script;
//				script.SetMemFixed(outputs[i].script, outputs[i].scriptLen);
//
//				Address address(outputs[i].address);
//
//				TransactionOutput *output = new TransactionOutput(outputs[i].amount, script, address.getSignType());
//				transaction->outputs.push_back(output);
//				amount += outputs[i].amount;
//			}
//
//			minAmount = BRWalletMinOutputAmount(wallet);
//			pthread_mutex_lock(&lock);
//			feeAmount = fee > 0 ? fee : _txFee(feePerKb,
//											   ELATransactionSize(transaction) + TX_OUTPUT_SIZE);
//			transaction->fee = feeAmount;
//
//			// TODO: use up all UTXOs for all used addresses to avoid leaving funds in addresses whose public key is revealed
//			// TODO: avoid combining addresses in a single transaction when possible to reduce information leakage
//			// TODO: use up UTXOs received from any of the output scripts that this transaction sends funds to, to mitigate an
//			//       attacker double spending and requesting a refund
//			for (i = 0; i < array_count(utxos); i++) {
//				o = &utxos[i];
//				tx = (ELATransaction *) BRSetGet(allTx, o);
//				if (!tx || o->n >= tx->outputs.size()) continue;
//				if (filter && !fromAddress.empty() && !filter(fromAddress, tx->outputs[o->n]->getAddress())) {
//					continue;
//				}
//
//				BRTransactionAddInput(&transaction->raw, tx->raw.txHash, o->n, tx->outputs[o->n]->getAmount(),
//									  tx->outputs[o->n]->getScript(), tx->outputs[o->n]->getScript().GetSize(),
//									  nullptr, 0, TXIN_SEQUENCE);
//				std::string addr = Utils::UInt168ToAddress(tx->outputs[o->n]->getProgramHash());
//				size_t inCount = transaction->raw.inCount;
//				BRTxInput *input = &transaction->raw.inputs[inCount - 1];
//				memset(input->address, 0, sizeof(input->address));
//				strncpy(input->address, addr.c_str(), sizeof(input->address) - 1);
//
//				if (ELATransactionSize(transaction) + TX_OUTPUT_SIZE >
//					TX_MAX_SIZE) { // transaction size-in-bytes too large
//					delete transaction;
//					transaction = nullptr;
//
//					// check for sufficient total funds before building a smaller transaction
//					if (balance < amount + fee > 0 ? fee : _txFee(feePerKb, 10 +
//																							array_count(utxos) *
//																							TX_INPUT_SIZE +
//																							(outCount + 1) *
//																							TX_OUTPUT_SIZE + cpfpSize))
//						break;
//					pthread_mutex_unlock(&lock);
//
//					if (outputs[outCount - 1].amount > amount + feeAmount + minAmount - balance) {
//						BRTxOutput newOutputs[outCount];
//
//						for (j = 0; j < outCount; j++) {
//							newOutputs[j] = outputs[j];
//						}
//
//						newOutputs[outCount - 1].amount -= amount + feeAmount - balance; // reduce last output amount
//						transaction = (ELATransaction *) CreateTxForOutputs(wallet, (BRTxOutput *) newOutputs, outCount,
//																			fee, fromAddress, filter);
//					} else {
//						transaction = (ELATransaction *) CreateTxForOutputs(wallet, outputs, outCount - 1, fee,
//																			fromAddress, filter); // remove last output
//					}
//
//					balance = amount = feeAmount = 0;
//					pthread_mutex_lock(&lock);
//					break;
//				}
//
//				balance += tx->outputs[o->n]->getAmount();
//
////        // size of unconfirmed, non-change inputs for child-pays-for-parent fee
////        // don't include parent tx with more than 10 inputs or 10 outputs
////        if (tx->blockHeight == TX_UNCONFIRMED && tx->inCount <= 10 && tx->outCount <= 10 &&
////            ! _BRWalletTxIsSend(wallet, tx)) cpfpSize += BRTransactionSize(tx);
//
//				// fee amount after adding a change output
//				feeAmount = fee > 0 ? fee : _txFee(feePerKb,
//												   ELATransactionSize(transaction) + TX_OUTPUT_SIZE + cpfpSize);
//
//				// increase fee to round off remaining wallet balance to nearest 100 satoshi
//				if (balance > amount + feeAmount) feeAmount += (balance - (amount + feeAmount)) % 100;
//
//				if (balance == amount + feeAmount || balance >= amount + feeAmount + minAmount) break;
//			}
//
//			pthread_mutex_unlock(&lock);
//
//			if (transaction && (outCount < 1 || balance < amount + feeAmount)) { // no outputs/insufficient funds
//				delete transaction;
//				transaction = nullptr;
//				if (balance < amount + feeAmount)
//					throw std::logic_error("Available token is not enough");
//				else if (outCount < 1)
//					throw std::logic_error("Output count is not enough");
//			} else if (transaction && balance - (amount + feeAmount) > minAmount) { // add change output
//				WalletUnusedAddrs(wallet, &addr, 1, 1);
//				CMBlock script(BRAddressScriptPubKey(nullptr, 0, addr.s));
//				BRAddressScriptPubKey(script, script.GetSize(), addr.s);
//				Address address(addr.s);
//
//				TransactionOutput *output = new TransactionOutput(balance - (amount + feeAmount), script,
//																  address.getSignType());
//
//				transaction->outputs.push_back(output);
//			}
//
//			return (BRTransaction *) transaction;
		}

		TransactionPtr
		Wallet::WalletCreateTxForOutputs(const std::vector<TransactionOutput> &outputs, size_t outCount) {
			return CreateTxForOutputs(outputs, outCount, 0, "", nullptr);
		}

		TransactionPtr
		Wallet::createTransaction(const std::string &fromAddress, uint64_t fee, uint64_t amount,
								  const std::string &toAddress, const std::string &remark,
								  const std::string &memo) {
			UInt168 u168Address = UINT168_ZERO;
			if (!fromAddress.empty() && !Utils::UInt168FromAddress(u168Address, fromAddress)) {
				std::ostringstream oss;
				oss << "Invalid spender address: " << fromAddress;
				throw std::logic_error(oss.str());
			}

			if (!Utils::UInt168FromAddress(u168Address, toAddress)) {
				std::ostringstream oss;
				oss << "Invalid receiver address: " << toAddress;
				throw std::logic_error(oss.str());
			}

			TransactionOutputPtr output = TransactionOutputPtr(new TransactionOutput());
			output->setProgramHash(u168Address);
			output->setAmount(amount);
			output->setAddress(toAddress);
			output->setAssetId(Key::getSystemAssetId());
			output->setOutputLock(0);

			//fixme [refactor] replace raw output with output
//			BRTxOutput outputs[1];
//			outputs[0] = *output->getRaw();
//
//			ELATransaction *tx = (ELATransaction *) CreateTxForOutputs((BRWallet *) _wallet, outputs, 1, fee,
//																	   fromAddress, AddressFilter);
//
//			TransactionPtr result = nullptr;
//			if (tx != nullptr) {
//				result = TransactionPtr(new Transaction(tx));
//				result->setRemark(remark);
//
//				result->addAttribute(
//						new Attribute(Attribute::Nonce, Utils::convertToMemBlock(std::to_string(std::rand()))));
//				if (!memo.empty())
//					result->addAttribute(new Attribute(Attribute::Memo, Utils::convertToMemBlock(memo)));
//				if (tx->type == ELATransaction::TransferCrossChainAsset)
//					result->addAttribute(
//							new Attribute(Attribute::Confirmations, Utils::convertToMemBlock(std::to_string(1))));
//			}
//
//			return result;
			return nullptr;
		}

		bool Wallet::containsTransaction(const TransactionPtr &transaction) {
			bool result = false;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = WalletContainsTx(transaction);
			}
			return result;
		}

//		bool Wallet::inputFromWallet(const BRTxInput *in) {
//			for (size_t i = 0; i < transactions.size(); i++) {
//				if (UInt256Eq(&in->txHash, &transactions[i]->getHash())) {
//					if (containsAddress(tx->outputs[in->index]->getAddress())) {
//						return true;
//					}
//				}
//			}
//
//			return false;
//		}

		bool Wallet::registerTransaction(const TransactionPtr &transaction) {
			//fixme [refactor] comlete me
			return false;
//			return BRWalletRegisterTransaction((BRWallet *) _wallet, transaction->getRaw()) != 0;
		}

		void Wallet::removeTransaction(const UInt256 &transactionHash) {
			UInt256 *hashes = NULL;
			int notifyUser = 0, recommendRescan = 0;

			assert(!UInt256IsZero(&transactionHash));
			lock.lock();
			const TransactionPtr &tx = allTx.GetTransaction(transactionHash);

			TransactionPtr t;
			if (tx) {
				array_new(hashes, 0);

				for (size_t i = transactions.size(); i > 0; i--) { // find depedent transactions
					t = transactions[i - 1];
					if (t->getBlockHeight() < tx->getBlockHeight()) break;
					if (tx->IsEqual(t.get())) continue;

					for (size_t j = 0; j < t->getInputs().size(); j++) {
						if (!UInt256Eq(&t->getInputs()[j].getTransctionHash(), &transactionHash)) continue;
						array_add(hashes, t->getHash());
						break;
					}
				}

				if (array_count(hashes) > 0) {
					lock.unlock();

					for (size_t i = array_count(hashes); i > 0; i--) {
						removeTransaction(hashes[i - 1]);
					}

					removeTransaction(transactionHash);
				} else {
					for (size_t i = transactions.size(); i > 0; i--) {
						if (!transactions[i - 1]->IsEqual(tx.get())) continue;
						transactions.erase(transactions.begin() + i - 1);
						break;
					}

					WalletUpdateBalance();
					lock.unlock();

					// if this is for a transaction we sent, and it wasn't already known to be invalid, notify user
					if (AmountSentByTx(tx) > 0 && transactionIsValid(tx)) {
						recommendRescan = notifyUser = 1;

						for (size_t i = 0;
							 i < tx->getInputs().size(); i++) { // only recommend a rescan if all inputs are confirmed
							t = transactionForHash(tx->getInputs()[i].getTransctionHash());
							if (t && t->getBlockHeight() != TX_UNCONFIRMED) continue;
							recommendRescan = 0;
							break;
						}
					}

					balanceChanged(balance);
					txDeleted(transactionHash, notifyUser, recommendRescan);
				}

				array_free(hashes);
			} else lock.unlock();
		}

		void Wallet::updateTransactions(const std::vector<UInt256> &transactionsHashes, uint32_t height,
										uint32_t timestamp) {
			std::vector<UInt256> hashes(transactionsHashes.size());
			int needsUpdate = 0;
			size_t i, j, k;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				if (height > blockHeight) blockHeight = height;

				for (i = 0, j = 0; i < transactionsHashes.size(); i++) {
					const TransactionPtr &tx = allTx.GetTransaction(transactionsHashes[i]);
					if (!tx || (tx->getBlockHeight() == height && tx->getTimestamp() == timestamp)) continue;

					if (tx->getBlockHeight() == TX_UNCONFIRMED) needsUpdate = 1;

					tx->setTimestamp(timestamp);
					tx->setBlockHeight(height);

					if (WalletContainsTx(tx)) {
//					for (k = transactions.size(); k > 0; k--) { // remove and re-insert tx to keep wallet sorted
//						if (!transactions[k - 1]->IsEqual(tx.get())) continue;
//						array_rm(transactions, k - 1);
//						_BRWalletInsertTx(wallet, tx);
//						break;
//					}
						sortTransations();

						hashes[j++] = transactionsHashes[i];
						if (pendingTx.Contains(tx) || invalidTx.Contains(tx)) needsUpdate = 1;
					} else if (blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
						allTx.Remove(tx);
					}
				}

				if (needsUpdate) WalletUpdateBalance();
			}

			if (j > 0) txUpdated(hashes, blockHeight, timestamp);
		}

		const TransactionPtr &Wallet::transactionForHash(const UInt256 &transactionHash) {
			TransactionPtr tx;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				tx = allTx.GetTransaction(transactionHash);
			}
			return tx;
		}

		bool Wallet::transactionIsValid(const TransactionPtr &transaction) {
			bool r = false;
			if (transaction == nullptr || !transaction->isSigned()) return r;

			// TODO: XXX attempted double spends should cause conflicted tx to remain unverified until they're confirmed
			// TODO: XXX conflicted tx with the same wallet outputs should be presented as the same tx to the user

			if (transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed transactions can be invalid

				{
					boost::mutex::scoped_lock scoped_lock(lock);
					if (!allTx.Contains(transaction)) {
						for (size_t i = 0; r && i < transaction->getInputs().size(); i++) {
							if (spentOutputs.Constains(transaction->getInputs()[i].getTransctionHash())) r = 0;
						}
					} else if (invalidTx.Contains(transaction)) r = 0;
				}

				for (size_t i = 0; r && i < transaction->getInputs().size(); i++) {
					const TransactionPtr &t = transactionForHash(transaction->getInputs()[i].getTransctionHash());
					if (t && !transactionIsValid(t)) r = 0;
				}
			}

			return r;
		}

		bool Wallet::transactionIsPending(const TransactionPtr &transaction) {
			time_t now = time(NULL);
			uint32_t height;
			int r = 0;

			assert(transaction->isSigned());
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = blockHeight;
			}

			if (transaction != nullptr &&
				transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed transactions can be postdated
				if (transaction->getSize() > TX_MAX_SIZE) r = 1; // check transaction size is under TX_MAX_SIZE

				for (size_t i = 0; !r && i < transaction->getInputs().size(); i++) {
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX - 1) r = 1; // check for replace-by-fee
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX &&
						transaction->getLockTime() < TX_MAX_LOCK_HEIGHT && transaction->getLockTime() > blockHeight + 1)
						r = 1; // future lockTime
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX && transaction->getLockTime() > now)
						r = 1; // future lockTime
				}

				for (size_t i = 0; !r && i < transaction->getOutputs().size(); i++) { // check that no outputs are dust
					if (transaction->getOutputs()[i].getAmount() < TX_MIN_OUTPUT_AMOUNT) r = 1;
				}

				for (size_t i = 0;
					 !r && i < transaction->getInputs().size(); i++) { // check if any inputs are known to be pending
					const TransactionPtr &t = transactionForHash(transaction->getInputs()[i].getTransctionHash());
					if (t && transactionIsPending(t)) r = 1;
				}
			}

			return r;
		}

		bool Wallet::transactionIsVerified(const TransactionPtr &transaction) {
			bool r = true;
			assert(transaction != NULL && transaction->isSigned());

			if (transaction &&
				transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed transactions can be unverified
				if (transaction->getTimestamp() == 0 || !transactionIsValid(transaction) ||
					transactionIsPending(transaction))
					r = false;

				for (size_t i = 0;
					 r && i < transaction->getInputs().size(); i++) { // check if any inputs are known to be unverified
					const TransactionPtr &t = transactionForHash(transaction->getInputs()[i].getTransctionHash());
					if (t && !transactionIsVerified(t)) r = false;
				}
			}

			return r;
		}

		uint64_t Wallet::getTransactionAmount(const TransactionPtr &tx) {
			uint64_t amountSent = getTransactionAmountSent(tx);
			uint64_t amountReceived = getTransactionAmountReceived(tx);

			return amountSent == 0
				   ? amountReceived
				   : -1 * (amountSent - amountReceived + getTransactionFee(tx));
		}

		uint64_t Wallet::getTransactionFee(const TransactionPtr &tx) {
			return WalletFeeForTx(tx);
		}

		uint64_t Wallet::getTransactionAmountSent(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != NULL);
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; tx && i < tx->getInputs().size(); i++) {
					const TransactionPtr &t = allTx.GetTransaction(tx->getInputs()[i].getTransctionHash());
					uint32_t n = tx->getInputs()[i].getIndex();

					//fixme [refactor]
//					if (t && n < t->getOutputs().size() &&
//						allAddrs.find(t->getOutputs()[n].getAddress()) != allAddrs.end()) {
//						amount += t->getOutputs()[n].getAmount();
//					}
				}
			}

			return amount;
		}

		uint64_t Wallet::getTransactionAmountReceived(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != NULL);

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				// TODO: don't include outputs below TX_MIN_OUTPUT_AMOUNT
				for (size_t i = 0; tx && i < tx->getOutputs().size(); i++) {
					//fixme [refactor]
//					if (allAddrs.find(tx->getOutputs()[i].getAddress()) != allAddrs.end())
//						amount += tx->getOutputs()[i].getAmount();
				}
			}

			return amount;
		}

		uint64_t Wallet::getBalanceAfterTransaction(const TransactionPtr &transaction) {
			return BalanceAfterTx(transaction);
		}

		std::string Wallet::getTransactionAddress(const TransactionPtr &transaction) {

			return getTransactionAmount(transaction) > 0
				   ? getTransactionAddressInputs(transaction)   // we received -> from inputs
				   : getTransactionAddressOutputs(transaction); // we sent     -> to outputs
		}

		std::string Wallet::getTransactionAddressInputs(const TransactionPtr &transaction) {

			for (size_t i = 0; i < transaction->getInputs().size(); i++) {
				//fixme [refactor] get address from tx hash
//				std::string address = transaction->getInputs()[i].get;
//				if (!containsAddress(address))
//					return address;
			}
			return "";
		}

		std::string Wallet::getTransactionAddressOutputs(const TransactionPtr &transaction) {

			for (size_t i = 0; i < transaction->getOutputs().size(); i++) {
				std::string address = transaction->getOutputs()[i].getAddress();
				if (!containsAddress(address))
					return address;
			}
			return "";
		}

		uint64_t Wallet::getFeeForTransactionSize(size_t size) {
			uint64_t fee;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				fee = _txFee(feePerKb, size);
			}
			return fee;
		}

		uint64_t Wallet::getMinOutputAmount() {
			uint64_t amount;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				amount = (TX_MIN_OUTPUT_AMOUNT * feePerKb + MIN_FEE_PER_KB - 1) / MIN_FEE_PER_KB;
			}
			return (amount > TX_MIN_OUTPUT_AMOUNT) ? amount : TX_MIN_OUTPUT_AMOUNT;
		}

		uint64_t Wallet::getMaxOutputAmount() {

			return WalletMaxOutputAmount();
		}

		std::string Wallet::getReceiveAddress() const {
			std::vector<Address> addr = _subAccount->UnusedAddresses(1, 0);
			return addr[0].stringify();
		}

		std::vector<std::string> Wallet::getAllAddresses() {

			std::vector<Address> addrs = _subAccount->GetAllAddresses(INT64_MAX);

			std::vector<std::string> results;
			for (int i = 0; i < addrs.size(); i++) {
				results.push_back(addrs[i].stringify());
			}
			return results;
		}

		bool Wallet::containsAddress(const std::string &address) {
			if (_subAccount->IsSingleAddress()) {
				return _subAccount->GetParent()->GetAddress() == address;
			}

			bool result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				//fixme [refactor]
//				result = allAddrs.find(address) != allAddrs.end();
			}
			return result;
		}

		bool Wallet::addressIsUsed(const std::string &address) {
			bool result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				//fixme [refactor]
//				result = usedAddrs.find(address) != usedAddrs.end();
			}
			return result;
		}

		// maximum amount that can be sent from the wallet to a single address after fees
		uint64_t Wallet::WalletMaxOutputAmount() {
			uint64_t fee, amount = 0;
			size_t i, txSize, cpfpSize = 0, inCount = 0;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (i = utxos.size(); i > 0; i--) {
					UTXO &o = utxos[i - 1];
					const TransactionPtr &tx = allTx.GetTransaction(o.hash);
					if (!tx || o.n >= tx->getOutputs().size()) continue;
					inCount++;
					amount += tx->getOutputs()[o.n].getAmount();

//        // size of unconfirmed, non-change inputs for child-pays-for-parent fee
//        // don't include parent tx with more than 10 inputs or 10 outputs
//        if (tx->blockHeight == TX_UNCONFIRMED && tx->inCount <= 10 && tx->outCount <= 10 &&
//            ! _BRWalletTxIsSend(wallet, tx)) cpfpSize += BRTransactionSize(tx);
				}

				txSize = 8 + BRVarIntSize(inCount) + TX_INPUT_SIZE * inCount + BRVarIntSize(2) + TX_OUTPUT_SIZE * 2;
				fee = _txFee(feePerKb, txSize + cpfpSize);
			}

			return (amount > fee) ? amount - fee : 0;
		}

		uint64_t Wallet::WalletFeeForTx(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != nullptr);
			if (tx == nullptr) {
				return amount;
			}

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; i < tx->getInputs().size() && amount != UINT64_MAX; i++) {
					const TransactionPtr &t = allTx.GetTransaction(tx->getInputs()[i].getTransctionHash());
					uint32_t n = tx->getInputs()[i].getIndex();

					if (t && n < t->getOutputs().size()) {
						amount += t->getOutputs()[n].getAmount();
					} else amount = UINT64_MAX;
				}
			}

			for (size_t i = 0; tx->getOutputs().size() && amount != UINT64_MAX; i++) {
				amount -= tx->getOutputs()[i].getAmount();
			}

			return amount;
		}

		void Wallet::WalletUpdateBalance() {
			//fixme [refactor]
//			int isInvalid, isPending;
//			uint64_t balance = 0, prevBalance = 0;
//			time_t now = time(NULL);
//			size_t i, j;
//			ELATransaction *tx, *t;
//
//			array_clear(utxos);
//			array_clear(balanceHist);
//			BRSetClear(spentOutputs);
//			BRSetClear(invalidTx);
//			BRSetClear(pendingTx);
//			BRSetClear(usedAddrs);
//			totalSent = 0;
//			totalReceived = 0;
//
//			for (i = 0; i < array_count(transactions); i++) {
//				tx = (ELATransaction *) transactions[i];
//
//				// check if any inputs are invalid or already spent
//				if (tx->raw.blockHeight == TX_UNCONFIRMED) {
//					for (j = 0, isInvalid = 0; !isInvalid && j < tx->raw.inCount; j++) {
//						if (BRSetContains(spentOutputs, &tx->raw.inputs[j]) ||
//							BRSetContains(invalidTx, &tx->raw.inputs[j].txHash))
//							isInvalid = 1;
//					}
//
//					if (isInvalid) {
//						BRSetAdd(invalidTx, tx);
//						array_add(balanceHist, balance);
//						continue;
//					}
//				}
//
//				// add inputs to spent output set
//				for (j = 0; j < tx->raw.inCount; j++) {
//					BRSetAdd(spentOutputs, &tx->raw.inputs[j]);
//				}
//
//				// check if tx is pending
//				if (tx->raw.blockHeight == TX_UNCONFIRMED) {
//					isPending = (ELATransactionSize(tx) > TX_MAX_SIZE) ? 1 : 0; // check tx size is under TX_MAX_SIZE
//
//					for (j = 0; !isPending && j < tx->outputs.size(); j++) {
//						if (tx->outputs[j]->getAmount() < TX_MIN_OUTPUT_AMOUNT)
//							isPending = 1; // check that no outputs are dust
//					}
//
//					for (j = 0; !isPending && j < tx->raw.inCount; j++) {
//						if (tx->raw.inputs[j].sequence < UINT32_MAX - 1) isPending = 1; // check for replace-by-fee
//						if (tx->raw.inputs[j].sequence < UINT32_MAX && tx->raw.lockTime < TX_MAX_LOCK_HEIGHT &&
//							tx->raw.lockTime > blockHeight + 1)
//							isPending = 1; // future lockTime
//						if (tx->raw.inputs[j].sequence < UINT32_MAX && tx->raw.lockTime > now)
//							isPending = 1; // future lockTime
//						if (BRSetContains(pendingTx, &tx->raw.inputs[j].txHash))
//							isPending = 1; // check for pending inputs
//						// TODO: XXX handle BIP68 check lock time verify rules
//					}
//
//					if (isPending) {
//						BRSetAdd(pendingTx, tx);
//						array_add(balanceHist, balance);
//						continue;
//					}
//				}
//
//				// add outputs to UTXO set
//				// TODO: don't add outputs below TX_MIN_OUTPUT_AMOUNT
//				// TODO: don't add coin generation outputs < 100 blocks deep
//				// NOTE: balance/UTXOs will then need to be recalculated when last block changes
//				for (j = 0; tx->raw.blockHeight != TX_UNCONFIRMED && j < tx->outputs.size(); j++) {
//					if (!tx->outputs[j]->getAddress().empty()) {
//						if (((ELAWallet *) wallet)->IsSingleAddress) {
//							ELAWallet *elaWallet = (ELAWallet *) wallet;
//							if (elaWallet->SingleAddress == tx->outputs[j]->getAddress()) {
//								array_add(utxos, ((BRUTXO) {tx->raw.txHash, (uint32_t) j}));
//								balance += tx->outputs[j]->getAmount();
//							}
//						} else {
//							BRSetAdd(usedAddrs, (void *) tx->outputs[j]->getAddress().c_str());
//
//							if (BRSetContains(allAddrs, tx->outputs[j]->getAddress().c_str())) {
//								array_add(utxos, ((BRUTXO) {tx->raw.txHash, (uint32_t) j}));
//								balance += tx->outputs[j]->getAmount();
//							}
//						}
//					}
//				}
//
//				// transaction ordering is not guaranteed, so check the entire UTXO set against the entire spent output set
//				for (j = array_count(utxos); j > 0; j--) {
//					if (!BRSetContains(spentOutputs, &utxos[j - 1])) continue;
//					t = (ELATransaction *) BRSetGet(allTx, &utxos[j - 1].hash);
//					balance -= t->outputs[utxos[j - 1].n]->getAmount();
//					array_rm(utxos, j - 1);
//				}
//
//				if (prevBalance < balance) totalReceived += balance - prevBalance;
//				if (balance < prevBalance) totalSent += prevBalance - balance;
//				array_add(balanceHist, balance);
//				prevBalance = balance;
//			}
//
//			assert(array_count(balanceHist) == array_count(transactions));
//			balance = balance;
		}

		bool Wallet::WalletContainsTx(const TransactionPtr &tx) {
			bool r = false;

			if (tx == nullptr)
				return r;

			size_t outCount = tx->getOutputs().size();

			for (size_t i = 0; !r && i < outCount; i++) {
				if (_subAccount->IsSingleAddress()) {
					if (_subAccount->GetParent()->GetAddress() == tx->getOutputs()[i].getAddress()) {
						r = 1;
					}
				} else {
					//fixme [refactor]
//					if (allAddrs.find(tx->getOutputs()[i].getAddress()) != allAddrs.end()) {
//						r = 1;
//					}
				}
			}

			//fixme [refactor] complete me
//			for (size_t i = 0; !r && i < tx->getInputs().size(); i++) {
//				ELATransaction *t = (ELATransaction *) BRSetGet(allTx, &txn->raw.inputs[i].txHash);
//				uint32_t n = txn->raw.inputs[i].index;
//
//				if (t == nullptr || n >= t->outputs.size()) {
//					continue;
//				}
//
//				if (elaWallet->IsSingleAddress) {
//					if (elaWallet->SingleAddress == std::string()) {
//						r = 1;
//					}
//				} else {
//					if (BRSetContains(allAddrs, t->outputs[n]->getAddress().c_str())) {
//						r = 1;
//					}
//				}
//			}
//
//			//for listening addresses
//			for (size_t i = 0; i < outCount; ++i) {
//				if (std::find(elaWallet->ListeningAddrs.begin(), elaWallet->ListeningAddrs.end(),
//							  txn->outputs[i]->getAddress()) != elaWallet->ListeningAddrs.end())
//					r = 1;
//			}

			return r;
		}

		void Wallet::setApplyFreeTx(void *info, void *tx) {
		}

		void Wallet::balanceChanged(uint64_t balance) {
			if (!_listener.expired()) {
				_listener.lock()->balanceChanged(balance);
			}
		}

		void Wallet::txAdded(const TransactionPtr &tx) {
			if (!_listener.expired()) {
				_listener.lock()->onTxAdded(tx);
			}
		}

		void Wallet::txUpdated(const std::vector<UInt256> &txHashes, uint32_t blockHeight, uint32_t timestamp) {
			if (!_listener.expired()) {
				// Invoke the callback for each of txHashes.
				for (size_t i = 0; i < txHashes.size(); i++) {
					_listener.lock()->onTxUpdated(Utils::UInt256ToString(txHashes[i]), blockHeight, timestamp);
				}
			}
		}

		void Wallet::txDeleted(const UInt256 &txHash, int notifyUser, int recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onTxDeleted(Utils::UInt256ToString(txHash), static_cast<bool>(notifyUser),
											  static_cast<bool>(recommendRescan));
			}
		}

		uint32_t Wallet::getBlockHeight() const {
			return blockHeight;
		}

		uint64_t Wallet::BalanceAfterTx(const TransactionPtr &tx) {
			uint64_t result;

			assert(tx != NULL && tx->isSigned());
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = balance;

				for (size_t i = transactions.size(); tx && i > 0; i--) {
					if (!tx->IsEqual(transactions[i - 1].get())) continue;

					result = balanceHist[i - 1];
					break;
				}
			}

			return result;
		}

		void Wallet::signTransaction(const TransactionPtr &transaction, int forkId, const std::string &payPassword) {
			ParamChecker::checkNullPointer(transaction.get());
			_subAccount->SignTransaction(transaction, payPassword);
		}

		void Wallet::sortTransations() {
			//fixme [refator] sort keeping transactions sorted by date, oldest first (insertion sort), see _BRWalletTxCompare
//			std::sort(transactions.begin(), transactions.end(), )
		}

		uint64_t Wallet::AmountSentByTx(const TransactionPtr &tx) {
			uint64_t amount = 0;
			assert(tx != NULL);

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; tx && i < tx->getInputs().size(); i++) {
					const TransactionPtr &t = allTx.GetTransaction(tx->getInputs()[i].getTransctionHash());
					uint32_t n = tx->getInputs()[i].getIndex();

					//fixme [refactor]
//					if (t && n < t->getOutputs().size() &&
//						allAddrs.find(t->getOutputs()[n].getAddress()) != allAddrs.end()) {
//						amount += t->getOutputs()[n].getAmount();
//					}
				}
			}

			return amount;
		}

	}
}