// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "GroupedAsset.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/WalletCore/BIPs/Address.h>
#include <SDK/WalletCore/BIPs/HDKeychain.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <SDK/Plugin/Transaction/Payload/RegisterAsset.h>
#include <SDK/Wallet/UTXO.h>

#include <Interface/ISubWallet.h>

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace Elastos {
	namespace ElaWallet {

		Wallet::Wallet(uint32_t lastBlockHeight,
					   const std::string &walletID,
					   const std::vector<AssetPtr> &assetArray,
					   const std::vector<TransactionPtr> &txns,
					   const UTXOArray &cbUTXOs,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<Wallet::Listener> &listener) :
				_walletID(walletID),
				_blockHeight(lastBlockHeight),
				_feePerKb(DEFAULT_FEE_PER_KB),
				_subAccount(subAccount) {

			_listener = boost::weak_ptr<Listener>(listener);

			_subAccount->Init(txns, this);

			// TODO: change to better way later
			if (walletID.find("IDChain") != std::string::npos)
				_subAccount->InitDID();

			if (assetArray.empty()) {
				InstallDefaultAsset();
			} else {
				InstallAssets(assetArray);
			}

			if (!txns.empty() && !ContainsTx(txns[0])) { // verify _transactions match master pubKey
				std::string hash = txns[0]->GetHash().GetHex();
				ErrorChecker::ThrowLogicException(Error::WalletNotContainTx, "Wallet do not contain tx = " + hash);
			}

			bool needUpdate = false, movedToCoinbase = false;
			InputArray spentInputs;
			for (size_t i = 0; i < txns.size(); ++i) {
				if (txns[i]->IsCoinBase()) {
					movedToCoinbase = true;
					const OutputArray &outputs = txns[i]->GetOutputs();
					for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
						if (_subAccount->ContainsAddress((*o)->Addr())) {
							UTXOPtr cb(new UTXO(txns[i]->GetHash(), (*o)->FixedIndex(), txns[i]->GetTimestamp(),
												txns[i]->GetBlockHeight(), *o));
							_coinBaseUTXOs.push_back(cb);
							break;
						}
					}
				} else if (ContainsTx(txns[i])) {
					txns[i]->IsRegistered() = true;
					if (StripTransaction(txns[i])) {
						SPVLOG_DEBUG("{} strip tx: {}, h: {}, t: {}",
									 _walletID,
									 txns[i]->GetHash().GetHex(),
									 txns[i]->GetBlockHeight(),
									 txns[i]->GetTimestamp());
						needUpdate = true;
					}

					if (/*!txns[i]->IsSigned() || */_allTx.Contains(txns[i]))
						continue;

					_allTx.Insert(txns[i]);
					InsertTx(txns[i]);

					if (txns[i]->GetBlockHeight() != TX_UNCONFIRMED) {
						for (InputArray::iterator in = txns[i]->GetInputs().begin(); in != txns[i]->GetInputs().end(); ++in)
							spentInputs.push_back(*in);

						BalanceAfterUpdatedTx(txns[i]);
					} else {
						for (InputArray::iterator in = txns[i]->GetInputs().begin(); in != txns[i]->GetInputs().end(); ++in)
							_spendingOutputs.push_back(UTXOPtr(new UTXO(*in)));
						SPVLOG_DEBUG("{} tx[{}]: {}, h: {}", _walletID, i,
									 txns[i]->GetHash().GetHex(), txns[i]->GetBlockHeight());
					}
				} else {
					needUpdate = true;
				}
			}

			_coinBaseUTXOs.insert(_coinBaseUTXOs.end(), cbUTXOs.begin(), cbUTXOs.end());

			std::vector<uint256> updatedSpent;
			for (InputArray::iterator in = spentInputs.begin(); in != spentInputs.end(); ++in) {
				for (UTXOArray::iterator cb = _coinBaseUTXOs.begin(); cb != _coinBaseUTXOs.end(); ++cb) {
					if ((*cb)->Equal((*in))) {
						(*cb)->SetSpent(true);
						updatedSpent.push_back((*cb)->Hash());
						break;
					}
				}

				for (GroupedAssetMap::iterator asset = _groupedAssets.begin(); asset != _groupedAssets.end(); ++asset) {
					if (asset->second->RemoveSpentUTXO(*in))
						break;
				}
			}

			for (UTXOArray::iterator o = _coinBaseUTXOs.begin(); o != _coinBaseUTXOs.end(); ++o) {
				if (!(*o)->Spent())
					_groupedAssets[(*o)->Output()->AssetID()]->AddCoinBaseUTXO((*o));
			}

			if (!updatedSpent.empty()) {
				SPVLOG_DEBUG("{} update spent hash count {}", _walletID, updatedSpent.size());
				coinBaseSpent(updatedSpent);
			}

			if (movedToCoinbase) {
				SPVLOG_DEBUG("{} mv coinbase tx to single table", _walletID);
				coinBaseUpdatedAll(_coinBaseUTXOs);
			}

			if (needUpdate) {
				SPVLOG_DEBUG("{} contain not striped tx, update all tx", _walletID);
				txUpdatedAll(_transactions);
			}
			SPVLOG_DEBUG("{} balance info {}", _walletID, GetBalanceInfo().dump());
		}

		Wallet::~Wallet() {
		}

		std::vector<UTXOPtr> Wallet::GetAllUTXO(const std::string &address) const {
			boost::mutex::scoped_lock scopedLock(lock);
			UTXOArray result;

			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				UTXOArray utxos = it->second->GetUTXOs(address);
				result.insert(result.end(), utxos.begin(), utxos.end());
			}

			return result;
		}

		UTXOArray Wallet::GetVoteUTXO() const {
			boost::mutex::scoped_lock scopedLock(lock);
			UTXOArray result;

			std::for_each(_groupedAssets.begin(), _groupedAssets.end(),
						  [&result](GroupedAssetMap::reference &asset) {
							  const UTXOArray &utxos = asset.second->GetVoteUTXO();
							  result.insert(result.end(), utxos.begin(), utxos.end());
			});

			return result;
		}

		nlohmann::json Wallet::GetBalanceInfo() {
			boost::mutex::scoped_lock scopedLock(lock);
			nlohmann::json info;

			GroupedAssetMap::iterator it;
			for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				nlohmann::json assetInfo;
				assetInfo["AssetID"] = it->first.GetHex();
				assetInfo["Summary"] = it->second->GetBalanceInfo();
				info.push_back(assetInfo);
			}

			return info;
		}

		BigInt Wallet::GetBalanceWithAddress(const uint256 &assetID, const std::string &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);

			BigInt balance = 0;
			std::vector<UTXOPtr> utxos = GetUTXO(assetID, addr);

			for (size_t i = 0; i < utxos.size(); ++i) {
				const TransactionPtr tx = _allTx.Get(utxos[i]->Hash());
				if (tx) {
					OutputPtr o = tx->OutputOfIndex(utxos[i]->Index());
					if (o && o->Addr().String() == addr) {
						balance += o->Amount();
					}
				}
			}

			return balance;
		}

		BigInt Wallet::GetBalance(const uint256 &assetID) const {
			ErrorChecker::CheckParam(!ContainsAsset(assetID), Error::InvalidAsset, "asset not found");

			boost::mutex::scoped_lock scoped_lock(lock);

			return _groupedAssets[assetID]->GetBalance();
		}

		uint64_t Wallet::GetFeePerKb() const {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _feePerKb;
		}

		void Wallet::SetFeePerKb(uint64_t fee) {
			boost::mutex::scoped_lock scoped_lock(lock);
			_feePerKb = fee;
		}

		uint64_t Wallet::GetDefaultFeePerKb() {
			return DEFAULT_FEE_PER_KB;
		}

		// only support asset of ELA
		TransactionPtr Wallet::Vote(const VoteContent &voteContent, const std::string &memo, bool max) {
			return _groupedAssets[Asset::GetELAAssetID()]->Vote(voteContent, memo, max);
		}

		TransactionPtr Wallet::Consolidate(const std::string &memo, const uint256 &assetID) {
			Lock();
			bool containAsset = ContainsAsset(assetID);
			Unlock();

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx = _groupedAssets[assetID]->Consolidate(memo);

			if (assetID != Asset::GetELAAssetID())
				_groupedAssets[Asset::GetELAAssetID()]->AddFeeForTx(tx);

			return tx;
		}

		TransactionPtr Wallet::CreateTransaction(uint8_t type,
												 const PayloadPtr &payload,
												 const Address &fromAddress,
												 const std::vector<OutputPtr> &outputs,
												 const std::string &memo,
												 bool max) {

			ErrorChecker::CheckParam(!IsAssetUnique(outputs), Error::InvalidAsset, "asset is not unique in outputs");

			uint256 assetID = outputs.front()->AssetID();

			Lock();
			bool containAsset = ContainsAsset(assetID);
			Unlock();

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx;
			if (fromAddress.Valid() && (_subAccount->IsProducerDepositAddress(fromAddress) || _subAccount->IsCRDepositAddress(fromAddress)))
				tx = _groupedAssets[assetID]->CreateRetrieveDepositTx(type, payload, outputs, fromAddress, memo);
			else
				tx = _groupedAssets[assetID]->CreateTxForOutputs(type, payload, outputs, fromAddress, memo, max);

			if (assetID != Asset::GetELAAssetID())
				_groupedAssets[Asset::GetELAAssetID()]->AddFeeForTx(tx);

			return tx;
		}

		bool Wallet::ContainsTransaction(const TransactionPtr &tx) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return ContainsTx(tx);
		}

		bool Wallet::RegisterTransaction(const TransactionPtr &tx) {
			bool r = true, wasAdded = false;
			UTXOPtr cb = nullptr;
			std::map<uint256, BigInt> changedBalance;

			bool IsReceiveTx = IsReceiveTransaction(tx);
			if (tx != nullptr && (IsReceiveTx || (!IsReceiveTx && tx->IsSigned()))) {
				Lock();
				if (!tx->IsCoinBase() && !_allTx.Contains(tx)) {
					if (ContainsTx(tx)) {
						// TODO: verify signatures when possible
						// TODO: handle tx replacement with input sequence numbers
						//       (for now, replacements appear invalid until confirmation)
						_allTx.Insert(tx);
						InsertTx(tx);
						if (tx->GetBlockHeight() != TX_UNCONFIRMED)
							changedBalance = BalanceAfterUpdatedTx(tx);
						wasAdded = true;
					} else { // keep track of unconfirmed non-wallet tx for invalid tx checks and child-pays-for-parent fees
						// BUG: limit total non-wallet unconfirmed tx to avoid memory exhaustion attack
						// if (tx->GetBlockHeight() == TX_UNCONFIRMED) _allTx.Insert(tx);
						r = false;
						// BUG: XXX memory leak if tx is not added to wallet->_allTx, and we can't just free it
					}
				} else if (tx->IsCoinBase() && nullptr == CoinBaseForHashInternal(tx->GetHash())) {
					cb = RegisterCoinBaseTx(tx);
				}
				Unlock();
			} else {
				r = false;
			}

			if (wasAdded) {
				UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
				UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
				txAdded(tx);
			} else if (cb) {
				coinBaseTxAdded(cb);
			}

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);

			return r;
		}

		void Wallet::RemoveTransaction(const uint256 &txHash) {
			bool notifyUser = false, recommendRescan = false;
			std::vector<uint256> hashes;

			assert(txHash != 0);

			Lock();
			const TransactionPtr tx = _allTx.Get(txHash);

			if (tx) {
				for (size_t i = _transactions.size(); i > 0; i--) { // find depedent _transactions
					const TransactionPtr &t = _transactions[i - 1];
					if (t->GetBlockHeight() < tx->GetBlockHeight()) break;
					if (tx->IsEqual(t.get())) continue;

					for (size_t j = 0; j < t->GetInputs().size(); j++) {
						if (t->GetInputs()[j]->TxHash() != txHash) continue;
						hashes.push_back(t->GetHash());
						break;
					}
				}

				if (!hashes.empty()) {
					Unlock();
					for (size_t i = hashes.size(); i > 0; i--) {
						RemoveTransaction(hashes[i - 1]);
					}

					RemoveTransaction(txHash);
				} else {
					for (size_t i = _transactions.size(); i > 0; i--) {
						if (_transactions[i - 1]->IsEqual(tx.get())) {
							_transactions.erase(_transactions.begin() + i - 1);
							break;
						}
					}

					BalanceAfterRemoveTx(tx);
					Unlock();

					// if this is for a transaction we sent, and it wasn't already known to be invalid, notify user
					if (AmountSentByTx(tx) > 0 && TransactionIsValid(tx)) {
						recommendRescan = notifyUser = true;

						for (size_t i = 0;
							 i < tx->GetInputs().size(); i++) { // only recommend a rescan if all inputs are confirmed
							TransactionPtr t = TransactionForHash(tx->GetInputs()[i]->TxHash());
							if (t && t->GetBlockHeight() != TX_UNCONFIRMED) continue;
							recommendRescan = false;
							break;
						}
					}

					txDeleted(tx->GetHash(), notifyUser, recommendRescan);
				}
			} else {
				Unlock();
			}
		}

		void Wallet::UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			std::vector<uint256> hashes, cbHashes, spentCoinBase;
			std::map<uint256, BigInt> changedBalance;
			std::vector<RegisterAsset *> payloads;
			UTXOPtr cb;
			size_t i;

#ifdef SPDLOG_DEBUG_ON
			for (i = 0; i < txHashes.size(); ++i) {
				SPVLOG_DEBUG("{} update tx[{}]: {}, h: {}, t: {}", _walletID, i, txHashes[i].GetHex(), blockHeight, timestamp);
			}
#endif
			Lock();
			if (blockHeight != TX_UNCONFIRMED && blockHeight > _blockHeight)
				_blockHeight = blockHeight;

			for (i = 0; i < txHashes.size(); i++) {
				TransactionPtr tx = _allTx.Get(txHashes[i]);
				if (tx) {
					bool needUpdate = false;
					if (tx->GetBlockHeight() == blockHeight && tx->GetTimestamp() == timestamp)
						continue;

					if (tx->GetBlockHeight() == TX_UNCONFIRMED && blockHeight != TX_UNCONFIRMED) {
						needUpdate = true;
						if (tx->GetTransactionType() == Transaction::registerAsset) {
							RegisterAsset *p = dynamic_cast<RegisterAsset *>(tx->GetPayload());
							if (p) payloads.push_back(p);
						}
					}

					tx->SetTimestamp(timestamp);
					tx->SetBlockHeight(blockHeight);

					if (ContainsTx(tx)) {
						for (std::vector<TransactionPtr>::reverse_iterator it = _transactions.rbegin(); it != _transactions.rend();) {
							if ((*it)->IsEqual(tx.get())) {
								it = std::vector<TransactionPtr>::reverse_iterator(_transactions.erase((++it).base()));
								InsertTx(tx);
								break;
							} else {
								++it;
							}
						}
						hashes.push_back(txHashes[i]);
						RemoveSpendingUTXO(tx->GetInputs());
						GetSpentCoinbase(tx->GetInputs(), spentCoinBase);
						if (needUpdate)
							changedBalance = BalanceAfterUpdatedTx(tx);
					} else if (blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
						Log::warn("{} remove non-wallet tx: {}", _walletID, tx->GetHash().GetHex());
						_allTx.Remove(tx);
					}
				} else if ((cb = CoinBaseForHashInternal(txHashes[i])) != nullptr) {
					if (cb->BlockHeight() == blockHeight && cb->Timestamp() == timestamp)
						continue;

					cb->SetTimestamp(timestamp);
					cb->SetBlockHeight(blockHeight);
					_groupedAssets[cb->Output()->AssetID()]->AddCoinBaseUTXO(cb);
					cbHashes.push_back(txHashes[i]);
				}
			}

			if (!payloads.empty()) {
				for (i = 0; i < payloads.size(); ++i)
					InstallAssets({payloads[i]->GetAsset()});
			}

			Unlock();

			if (!hashes.empty())
				txUpdated(hashes, blockHeight, timestamp);

			if (!cbHashes.empty())
				coinBaseTxUpdated(cbHashes, blockHeight, timestamp);

			if (!spentCoinBase.empty()) {
				coinBaseSpent(spentCoinBase);
			}

			if (!payloads.empty()) {
				for (i = 0; i < payloads.size(); ++i)
					assetRegistered(payloads[i]->GetAsset(), payloads[i]->GetAmount(), payloads[i]->GetController());
			}

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);
		}

		TransactionPtr Wallet::TransactionForHash(const uint256 &txHash) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _allTx.Get(txHash);
		}

		UTXOPtr Wallet::CoinBaseTxForHash(const uint256 &txHash) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return CoinBaseForHashInternal(txHash);
		}

		bool Wallet::TransactionIsValid(const TransactionPtr &tx) {
			bool r = true;
			if (tx == nullptr || !tx->IsSigned())
				return false;

			// TODO: XXX attempted double spends should cause conflicted tx to remain unverified until they're confirmed
			// TODO: XXX conflicted tx with the same wallet outputs should be presented as the same tx to the user

#if 0
			if (tx->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be invalid
//				Lock();
//				if (!_allTx.Contains(tx)) {
//					for (size_t i = 0; r && i < tx->GetInputs().size(); ++i) {
//						if (_spentOutputs.Contains(tx->GetInputs()[i]))
//							r = false;
//					}
//				} else if (_invalidTx.Contains(tx)) {
//					r = false;
//				}
//				Unlock();

				for (size_t i = 0; r && i < tx->GetInputs().size(); ++i) {
					TransactionPtr t = TransactionForHash(tx->GetInputs()[i]->TxHash());
					if (t && !TransactionIsValid(t))
						r = false;
				}
			}
#endif

			return r;
		}

#if 0
		bool Wallet::TransactionIsPending(const TransactionPtr &transaction) {
			time_t now = time(NULL);
			uint32_t height;
			bool r = false;

			assert(transaction->IsSigned());

			Lock();
			height = _blockHeight;
			Unlock();


			if (transaction != nullptr &&
				transaction->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be postdated
				if (transaction->GetSize() > TX_MAX_SIZE) r = true; // check transaction size is under TX_MAX_SIZE

				for (size_t i = 0; !r && i < transaction->GetInputs().size(); i++) {
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX - 1) r = true; // check for replace-by-fee
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX &&
						transaction->GetLockTime() < TX_MAX_LOCK_HEIGHT &&
						transaction->GetLockTime() > height + 1)
						r = true; // future lockTime
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX && transaction->GetLockTime() > now)
						r = true; // future lockTime
				}

				for (size_t i = 0; !r && i < transaction->GetOutputs().size(); i++) { // check that no outputs are dust
					if (transaction->GetOutputs()[i].Amount() < TX_MIN_OUTPUT_AMOUNT) r = true;
				}

				for (size_t i = 0;
					 !r && i < transaction->GetInputs().size(); i++) { // check if any inputs are known to be pending
					const TransactionPtr &t = TransactionForHash(transaction->GetInputs()[i].GetTransctionHash());
					if (t && TransactionIsPending(t)) r = true;
				}
			}

			return r;
		}

		bool Wallet::TransactionIsVerified(const TransactionPtr &transaction) {
			bool r = true;
			assert(transaction != NULL && transaction->IsSigned());

			if (transaction &&
				transaction->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be unverified
				if (transaction->GetTimestamp() == 0 || !TransactionIsValid(transaction) ||
					TransactionIsPending(transaction))
					r = false;

				for (size_t i = 0;
					 r && i < transaction->GetInputs().size(); i++) { // check if any inputs are known to be unverified
					const TransactionPtr &t = TransactionForHash(transaction->GetInputs()[i].GetTransctionHash());
					if (t && !TransactionIsVerified(t)) r = false;
				}
			}

			return r;
		}
#endif

		BigInt Wallet::AmountSentByTx(const TransactionPtr &tx) {
			BigInt amount(0);

			boost::mutex::scoped_lock scopedLock(lock);
			if (!tx)
				return amount;

			for (InputArray::iterator in = tx->GetInputs().begin(); in != tx->GetInputs().end(); ++in) {
				TransactionPtr t = _allTx.Get((*in)->TxHash());
				UTXOPtr cb = nullptr;
				if (t) {
					OutputPtr o = t->OutputOfIndex((*in)->Index());
					if (o && _subAccount->ContainsAddress(o->Addr())) {
						amount += o->Amount();
					}
				} else if ((cb = CoinBaseForHashInternal((*in)->TxHash())) != nullptr && cb->Index() == (*in)->Index()) {
					amount += cb->Output()->Amount();
				}
			}

			return amount;
		}

		bool Wallet::IsReceiveTransaction(const TransactionPtr &tx) const {
			boost::mutex::scoped_lock scopedLock(lock);
			bool status = true;
			for (InputArray::iterator in = tx->GetInputs().begin(); in != tx->GetInputs().end(); ++in) {
				if (ContainsInput(*in)) {
					status = false;
					break;
				}
			}

			return status;
		}

		bool Wallet::StripTransaction(const TransactionPtr &tx) const {
			if (IsReceiveTransaction(tx) && tx->GetOutputs().size() > 2 &&
				tx->GetOutputs().size() - 1 == tx->GetOutputs().back()->FixedIndex()) {
				size_t sizeBeforeStrip = tx->GetOutputs().size();
				boost::mutex::scoped_lock scopedLock(lock);
				std::vector<OutputPtr> newOutputs;
				const std::vector<OutputPtr> &outputs = tx->GetOutputs();
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if (_subAccount->ContainsAddress((*o)->Addr()))
						newOutputs.push_back(*o);
				}

				if (newOutputs.size() != sizeBeforeStrip) {
					tx->SetOutputs(newOutputs);
					SPVLOG_DEBUG("{} strip tx {}, h: {}", _walletID, tx->GetHash().GetHex(), tx->GetBlockHeight());
					return true;
				}
			}
			return false;
		}

		Address Wallet::GetReceiveAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<Address> addr = _subAccount->UnusedAddresses(1, 0);
			return addr[0];
		}

		size_t Wallet::GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count,
									   bool containInternal) const {
			boost::mutex::scoped_lock scopedLock(lock);

			return _subAccount->GetAllAddresses(addr, start, count, containInternal);
		}

		size_t Wallet::GetAllDID(std::vector<Address> &did, uint32_t start, size_t count) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetAllDID(did, start, count);
		}

		size_t Wallet::GetAllPublickeys(std::vector<std::string> &pubkeys, uint32_t start, size_t count,
		                                bool containInternal) {
			boost::mutex::scoped_lock scopedLock(lock);

			return _subAccount->GetAllPublickeys(pubkeys, start, count, containInternal);
		}

		Address Wallet::GetOwnerDepositAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return Address(PrefixDeposit, _subAccount->OwnerPubKey());
		}

		Address Wallet::GetCROwnerDepositAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return Address(PrefixDeposit, _subAccount->DIDPubKey());
		}

		Address Wallet::GetOwnerAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return Address(PrefixStandard, _subAccount->OwnerPubKey());
		}

		std::vector<Address> Wallet::GetAllSpecialAddresses() const {
			std::vector<Address> result;
			boost::mutex::scoped_lock scopedLock(lock);
			if (_subAccount->Parent()->GetSignType() != Account::MultiSign) {
				// Owner address
				result.push_back(Address(PrefixStandard, _subAccount->OwnerPubKey()));
				// Owner deposit address
				result.push_back(Address(PrefixDeposit, _subAccount->OwnerPubKey()));
				// CR Owner deposit address
				result.push_back(Address(PrefixDeposit, _subAccount->DIDPubKey()));
			}

			return result;
		}

		bytes_t Wallet::GetOwnerPublilcKey() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->OwnerPubKey();
		}

		bool Wallet::IsDepositAddress(const Address &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);

			if (_subAccount->IsProducerDepositAddress(addr))
				return true;
			if (_subAccount->IsCRDepositAddress(addr))
				return true;

			return false;
		}

		bool Wallet::ContainsAddress(const Address &address) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _subAccount->ContainsAddress(address);
		}

		const std::string &Wallet::GetWalletID() const {
			return _walletID;
		}

		void Wallet::SetBlockHeight(uint32_t height) {
			boost::mutex::scoped_lock scopedLock(lock);
			_blockHeight = height;
		}

		uint32_t Wallet::LastBlockHeight() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _blockHeight;
		}

		void Wallet::SignTransaction(const TransactionPtr &tx, const std::string &payPassword) {
			boost::mutex::scoped_lock scopedLock(lock);
			_subAccount->SignTransaction(tx, payPassword);
		}

		std::string Wallet::SignWithDID(const Address &did, const std::string &msg, const std::string &payPasswd) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->SignWithDID(did, msg, payPasswd);
		}

		std::vector<TransactionPtr> Wallet::TxUnconfirmedBefore(uint32_t blockHeight) {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<TransactionPtr> result;

			for (size_t i = _transactions.size(); i > 0; --i) {
				if (_transactions[i - 1]->GetBlockHeight() >= blockHeight) {
					result.push_back(_transactions[i - 1]);
				} else {
					break;
				}
			}
			std::reverse(result.begin(), result.end());

			return result;
		}

		void Wallet::SetTxUnconfirmedAfter(uint32_t blockHeight) {
			size_t i, j, count;

			Lock();
			_blockHeight = blockHeight;
			count = i = _transactions.size();
			while (i > 0 && _transactions[i - 1]->GetBlockHeight() > blockHeight) i--;
			count -= i;

			std::vector<uint256> hashes;

			for (j = count; j > 0; --j) {
				if (_transactions[i + j - 1]->GetBlockHeight() != TX_UNCONFIRMED) {
					_transactions[i + j - 1]->SetBlockHeight(TX_UNCONFIRMED);
					hashes.push_back(_transactions[i + j - 1]->GetHash());
					BalanceAfterUpdatedTx(_transactions[i + j - 1]);
				}
			}

			Unlock();

			if (count > 0) txUpdated(hashes, TX_UNCONFIRMED, 0);
		}

		std::vector<Address> Wallet::UnusedAddresses(uint32_t gapLimit, bool internal) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		std::vector<TransactionPtr> Wallet::GetAllTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _transactions;
		}

		std::vector<UTXOPtr> Wallet::GetAllCoinBaseTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _coinBaseUTXOs;
		}

		AssetPtr Wallet::GetAsset(const uint256 &assetID) const {
			boost::mutex::scoped_lock scopedLock(lock);
			if (!ContainsAsset(assetID)) {
				Log::warn("asset not found: {}", assetID.GetHex());
				return nullptr;
			}

			return _groupedAssets[assetID]->GetAsset();
		}

		nlohmann::json Wallet::GetAllAssets() const {
			boost::mutex::scoped_lock scopedLock(lock);
			nlohmann::json j;
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				j.push_back(it->first.GetHex());
			}
			return j;
		}

		bool Wallet::AssetNameExist(const std::string &name) const {
			boost::mutex::scoped_lock scopedLock(lock);
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it)
				if (it->second->GetAsset()->GetName() == name)
					return true;
			return false;
		}

		bool Wallet::ContainsAsset(const uint256 &assetID) const {
			return _groupedAssets.find(assetID) != _groupedAssets.end();
		}

		bool Wallet::ContainsTx(const TransactionPtr &tx) const {
			bool r = false;

			if (tx == nullptr)
				return r;

			// support register asset tx
			if (tx->GetTransactionType() == Transaction::registerAsset) {
				return true;
			}

			const OutputArray &outputs = tx->GetOutputs();
			for (OutputArray::const_iterator it = outputs.cbegin(); !r && it != outputs.cend(); ++it) {
				if (_subAccount->ContainsAddress((*it)->Addr()))
					r = true;
			}

			for (const InputPtr &input : tx->GetInputs()) {
				if (ContainsInput(input)) {
					r = true;
					break;
				}
			}

			return r;
		}

		bool Wallet::ContainsInput(const InputPtr &in) const {
			bool r = false;
			UTXOPtr cb = nullptr;
			OutputPtr output = nullptr;

			const TransactionPtr tx = _allTx.Get(in->TxHash());
			if (tx) {
				output = tx->OutputOfIndex(in->Index());
				if (output && _subAccount->ContainsAddress(output->Addr())) {
					r = true;
				}
			} else if ((cb = CoinBaseForHashInternal(in->TxHash()))) {
				if (cb->Index() == in->Index()) {
					r = true;
				}
			}

			return r;
		}

		UTXOPtr Wallet::CoinBaseForHashInternal(const uint256 &txHash) const {
			for (size_t i = 0; i < _coinBaseUTXOs.size(); ++i) {
				if (_coinBaseUTXOs[i]->Hash() == txHash) {
					return _coinBaseUTXOs[i];
				}
			}

			return nullptr;
		}

		UTXOPtr Wallet::RegisterCoinBaseTx(const TransactionPtr &tx) {
			const OutputArray &outputs = tx->GetOutputs();
			for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
				if (_subAccount->ContainsAddress((*o)->Addr())) {
					UTXOPtr cb(new UTXO(tx->GetHash(), (*o)->FixedIndex(), tx->GetTimestamp(), tx->GetBlockHeight(), (*o)));
					_coinBaseUTXOs.push_back(cb);
					return cb;
				}
			}

			return nullptr;
		}

		void Wallet::InsertTx(const TransactionPtr &tx) {
			size_t i = _transactions.size();

			while (i > 0 && TxCompare(_transactions[i - 1], tx) > 0) {
				i--;
			}

			_transactions.insert(_transactions.begin() + i, tx);
		}

		int Wallet::TxCompare(const TransactionPtr &tx1, const TransactionPtr &tx2) const {
			size_t i = -1, j = -1;

			if (TxIsAscending(tx1, tx2))
				return 1;
			if (TxIsAscending(tx2, tx1))
				return -1;
			if ((i = _subAccount->InternalChainIndex(tx1)) != -1)
				j = _subAccount->InternalChainIndex(tx2);
			if (j == -1 && (i = _subAccount->ExternalChainIndex(tx1)) != -1)
				j = _subAccount->ExternalChainIndex(tx2);
			if (i != -1 && j != -1 && i != j)
				return (i > j) ? 1 : -1;
			return 0;
		}

		bool Wallet::TxIsAscending(const TransactionPtr &tx1, const TransactionPtr &tx2) const {
			if (! tx1 || ! tx2)
				return false;

			if (tx1->GetBlockHeight() > tx2->GetBlockHeight()) return 1;
			if (tx1->GetBlockHeight() < tx2->GetBlockHeight()) return 0;

			for (size_t i = 0; i < tx1->GetInputs().size(); i++) {
				if (tx1->GetInputs()[i]->TxHash() == tx2->GetHash()) return 1;
			}

			for (size_t i = 0; i < tx2->GetInputs().size(); i++) {
				if (tx2->GetInputs()[i]->TxHash() == tx1->GetHash()) return 0;
			}

			for (size_t i = 0; i < tx1->GetInputs().size(); i++) {
				if (TxIsAscending(_allTx.Get(tx1->GetInputs()[i]->TxHash()), tx2)) return 1;
			}

			return 0;
		}

		std::vector<UTXOPtr> Wallet::GetUTXO(const uint256 &assetID, const std::string &addr) const {
			if (!ContainsAsset(assetID)) {
				Log::error("asset not found: {}", assetID.GetHex());
				return {};
			}

			return _groupedAssets[assetID]->GetUTXOs(addr);
		}

		bool Wallet::IsAssetUnique(const OutputArray &outputs) const {
			for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
				if (outputs.front()->AssetID() != (*o)->AssetID())
					return false;
			}

			return true;
		}

		std::map<uint256, BigInt> Wallet::BalanceAfterUpdatedTx(const TransactionPtr &tx) {
			GroupedAssetMap::iterator it;
			std::map<uint256, BigInt> changedBalance;
			if (tx->GetBlockHeight() != TX_UNCONFIRMED) {
				for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
					if (it->second->RemoveSpentUTXO(tx->GetInputs())) {
						changedBalance[it->first] = it->second->GetBalance();
					}
					RemoveSpendingUTXO(tx->GetInputs());
				}

				const OutputArray &outputs = tx->GetOutputs();
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if (_subAccount->ContainsAddress((*o)->Addr())) {
						_subAccount->AddUsedAddrs((*o)->Addr());
						const uint256 &asset = (*o)->AssetID();
						if (ContainsAsset(asset)) {
							uint16_t n = (*o)->FixedIndex();
							UTXOPtr utxo(new UTXO(tx->GetHash(), n, tx->GetTimestamp(), tx->GetBlockHeight(), (*o)));
							if (_groupedAssets[asset]->AddUTXO(utxo))
								changedBalance[asset] = _groupedAssets[asset]->GetBalance();
						}
					}
				}
			} else {
				const OutputArray &outputs = tx->GetOutputs();
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if (_subAccount->ContainsAddress((*o)->Addr())) {
						_subAccount->AddUsedAddrs((*o)->Addr());
						if (ContainsAsset((*o)->AssetID()) &&
							_groupedAssets[(*o)->AssetID()]->RemoveSpentUTXO(tx->GetHash(), (*o)->FixedIndex())) {
							changedBalance[(*o)->AssetID()] = _groupedAssets[(*o)->AssetID()]->GetBalance();
						}
					}
				}

				const InputArray &inputs = tx->GetInputs();
				for (InputArray::const_iterator in = inputs.cbegin(); in != inputs.cend(); ++in) {
					const TransactionPtr txInput = _allTx.Get((*in)->TxHash());
					UTXOPtr cb;
					if (txInput && txInput->GetBlockHeight() != TX_UNCONFIRMED) {
						OutputPtr o = txInput->OutputOfIndex((*in)->Index());
						if (o) {
							_spendingOutputs.push_back(UTXOPtr(new UTXO(*in)));
							if (_subAccount->ContainsAddress(o->Addr()) && ContainsAsset(o->AssetID())) {
								UTXOPtr utxo(new UTXO(txInput->GetHash(), o->FixedIndex(), txInput->GetTimestamp(), txInput->GetBlockHeight(), o));
								if (_groupedAssets[o->AssetID()]->AddUTXO(utxo))
									changedBalance[o->AssetID()] = _groupedAssets[o->AssetID()]->GetBalance();
							}
						}
					} else if ((cb = CoinBaseForHashInternal((*in)->TxHash())) != nullptr && cb->Index() == (*in)->Index()) {
						_spendingOutputs.push_back(UTXOPtr(new UTXO(*in)));
						// TODO BUG update spent status to database
						cb->SetSpent(false);
						if (ContainsAsset(cb->Output()->AssetID())) {
							if (_groupedAssets[cb->Output()->AssetID()]->AddCoinBaseUTXO(cb))
								changedBalance[cb->Output()->AssetID()] = _groupedAssets[cb->Output()->AssetID()]->GetBalance();
						}
					}
				}
			}

			return changedBalance;
		}

		void Wallet::BalanceAfterRemoveTx(const TransactionPtr &tx) {
			if (tx->GetBlockHeight() == TX_UNCONFIRMED) {
				RemoveSpendingUTXO(tx->GetInputs());
			} else {
				// TODO: consider rollback later
			}
		}

		void Wallet::RemoveSpendingUTXO(const InputArray &inputs) {
			for (InputArray::const_iterator input = inputs.cbegin(); input != inputs.cend(); ++input) {
				for (UTXOArray::iterator o = _spendingOutputs.begin(); o != _spendingOutputs.end(); ++o) {
					if ((*o)->Equal(*input)) {
						_spendingOutputs.erase(o);
						break;
					}
				}
			}
		}

		void Wallet::UpdateLockedBalance() {
			std::map<uint256, BigInt> changedBalance;

			lock.lock();
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				if (it->second->UpdateLockedBalance()) {
					changedBalance[it->first] = it->second->GetBalance();
				}
			}
			lock.unlock();

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);
		}

		void Wallet::InstallAssets(const std::vector<AssetPtr> &assets) {
			for (size_t i = 0; i < assets.size(); ++i) {
				if (!ContainsAsset(assets[i]->GetHash())) {
					_groupedAssets[assets[i]->GetHash()] = GroupedAssetPtr(new GroupedAsset(this, assets[i]));
				} else {
					Log::debug("asset {} already exist", assets[i]->GetHash().GetHex());
				}
			}
		}

		void Wallet::InstallDefaultAsset() {
			AssetPtr asset(new Asset());
			_groupedAssets[asset->GetHash()] = GroupedAssetPtr(new GroupedAsset(this, asset));
			assetRegistered(asset, 0, uint168());
		}

		bool Wallet::IsUTXOSpending(const UTXOPtr &utxo) const {
			for (size_t i = 0; i < _spendingOutputs.size(); ++i) {
				if (*utxo == *_spendingOutputs[i]) {
					return true;
				}
			}

			return false;
		}

		void Wallet::GetSpentCoinbase(const InputArray &inputs, std::vector<uint256> &coinbase) const {
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				it->second->GetSpentCoinbase(inputs, coinbase);
			}
		}

		void Wallet::balanceChanged(const uint256 &asset, const BigInt &balance) {
			if (!_listener.expired()) {
				_listener.lock()->balanceChanged(asset, balance);
			}
		}

		void Wallet::coinBaseTxAdded(const UTXOPtr &cb) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseTxAdded(cb);
			}
		}

		void Wallet::coinBaseUpdatedAll(const UTXOArray &cbs) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseUpdatedAll(cbs);
			}
		}

		void Wallet::coinBaseTxUpdated(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseTxUpdated(txHashes, blockHeight, timestamp);
			}
		}

		void Wallet::coinBaseSpent(const std::vector<uint256> &spentHashes) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseSpent(spentHashes);
			}
		}

		void Wallet::coinBaseDeleted(const uint256 &txHash, bool notifyUser, bool recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseTxDeleted(txHash, notifyUser, recommendRescan);
			}
		}

		void Wallet::txAdded(const TransactionPtr &tx) {
			if (!_listener.expired()) {
				_listener.lock()->onTxAdded(tx);
			}
		}

		void Wallet::txUpdated(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			if (!_listener.expired()) {
				_listener.lock()->onTxUpdated(txHashes, blockHeight, timestamp);
			}
		}

		void Wallet::txDeleted(const uint256 &txHash, bool notifyUser, bool recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onTxDeleted(txHash, notifyUser, recommendRescan);
			}
		}

		void Wallet::txUpdatedAll(const std::vector<TransactionPtr> &txns) {
			if (!_listener.expired()) {
				_listener.lock()->onTxUpdatedAll(txns);
			}
		}

		void Wallet::assetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			if (!_listener.expired()) {
				_listener.lock()->onAssetRegistered(asset, amount, controller);
			}
		}

	}
}