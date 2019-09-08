// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterWallet.h"
#include "SubWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <SDK/Account/SubAccount.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/SpvService/Config.h>
#include <SDK/Wallet/UTXO.h>

#include <algorithm>
#include <boost/scoped_ptr.hpp>

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent) :
				PeerManager::Listener(),
				_parent(parent),
				_info(info),
				_config(config) {

			fs::path subWalletDBPath = _parent->GetDataPath();
			subWalletDBPath /= _info->GetChainID() + DB_FILE_EXTENSION;

			_subAccount = SubAccountPtr(new SubAccount(_parent->_account, _config->Index()));
			std::string walletID = _parent->GetID() + ":" + _info->GetChainID();
			_walletManager = WalletManagerPtr(
					new SpvService(walletID, _subAccount, subWalletDBPath,
								   _info->GetEarliestPeerTime(), _config->DisconnectionTime(),
								   _config->PluginType(), config->ChainParameters()));

			_walletManager->RegisterWalletListener(this);
			_walletManager->RegisterPeerManagerListener(this);

			WalletPtr wallet = _walletManager->getWallet();

			wallet->SetFeePerKb(_config->FeePerKB());
		}

		SubWallet::~SubWallet() {

		}

		std::string SubWallet::GetChainID() const {
			return _info->GetChainID();
		}

		const std::string &SubWallet::GetInfoChainID() const {
			return _info->GetChainID();
		}

		const SubWallet::WalletManagerPtr &SubWallet::GetWalletManager() const {
			return _walletManager;
		}

		nlohmann::json SubWallet::GetBalanceInfo() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			nlohmann::json info = _walletManager->getWallet()->GetBalanceInfo();

			ArgInfo("r => {}", info.dump());
			return info;
		}

		std::string SubWallet::GetBalance() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			std::string balance = _walletManager->getWallet()->GetBalance(Asset::GetELAAssetID()).getDec();

			ArgInfo("r => {}", balance);

			return balance;
		}

		std::string SubWallet::CreateAddress() {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			std::string address = _walletManager->getWallet()->GetReceiveAddress().String();

			ArgInfo("r => {}", address);

			return address;
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			std::vector<Address> addresses;
			size_t maxCount = _walletManager->getWallet()->GetAllAddresses(addresses, start, count, false);

			std::vector<std::string> addrString;
			for (size_t i = 0; i < addresses.size(); ++i) {
				addrString.push_back(addresses[i].String());
			}

			j["Addresses"] = addrString;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string SubWallet::GetBalanceWithAddress(const std::string &address) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("addr: {}", address);

			std::string balance = _walletManager->getWallet()->GetBalanceWithAddress(Asset::GetELAAssetID(), address).getDec();

			ArgInfo("r => {}", balance);
			return balance;
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("callback: 0x{:x}", (long)subCallback);

			boost::mutex::scoped_lock scoped_lock(lock);

			if (std::find(_callbacks.begin(), _callbacks.end(), subCallback) != _callbacks.end())
				return;
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("callback: 0x{:x}", (long)subCallback);
			boost::mutex::scoped_lock scoped_lock(lock);

			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		TransactionPtr SubWallet::CreateTx(const std::string &fromAddress, const std::vector<OutputPtr> &outputs,
		                                   const std::string &memo) const {
			for (const OutputPtr &output : outputs) {
				ErrorChecker::CheckParam(!output->Addr().Valid(), Error::CreateTransaction,
				                         "invalid receiver address " + output->Addr().String());

				ErrorChecker::CheckParam(output->Amount() < 0, Error::CreateTransaction,
				                         "output amount should big than zero");
			}

			std::string m;

			if (!memo.empty())
				m = "type:text,msg:" + memo;

			TransactionPtr tx = _walletManager->getWallet()->CreateTransaction(fromAddress, outputs, m);

			if (_info->GetChainID() == "ELA")
				tx->SetVersion(Transaction::TxVersion::V09);

			tx->FixIndex();

			return tx;
		}

		TransactionPtr SubWallet::CreateConsolidateTx(const std::string &memo, const uint256 &asset) const {
			std::string m;

			if (!memo.empty())
				m = "type:text,msg:" + memo;

			TransactionPtr tx = _walletManager->getWallet()->Consolidate(m, asset);

			if (_info->GetChainID() == "ELA")
				tx->SetVersion(Transaction::TxVersion::V09);

			tx->FixIndex();

			return tx;
		}

		void SubWallet::EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const {
			SPVLOG_DEBUG("raw tx: {}", tx->ToJson().dump());

			ByteStream stream;
			tx->Serialize(stream, true);
			const bytes_t &hex = stream.GetBytes();

			result["Algorithm"] = "base64";
			result["ID"] = tx->GetHash().GetHex().substr(0, 8);
			result["Data"] = hex.getBase64();
			result["ChainID"] = GetChainID();
			result["Fee"] = tx->GetFee();
		}

		TransactionPtr SubWallet::DecodeTx(const nlohmann::json &encodedTx) const {
			TransactionPtr tx(new Transaction());

			if (encodedTx.find("Algorithm") == encodedTx.end() ||
				encodedTx.find("Data") == encodedTx.end() ||
				encodedTx.find("ChainID") == encodedTx.end()) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Invalid input");
			}

			std::string algorithm, data, chainID;

			try {
				algorithm = encodedTx["Algorithm"].get<std::string>();
				data = encodedTx["Data"].get<std::string>();
				chainID = encodedTx["ChainID"].get<std::string>();
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Invalid input: " + std::string(e.what()));
			}

			if (chainID != GetChainID()) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument,
												  "Invalid input: tx is not belongs to current subwallet");
			}

			bytes_t rawHex;
			if (algorithm == "base64") {
				rawHex.setBase64(data);
			} else {
				ErrorChecker::CheckCondition(true, Error::InvalidArgument, "Decode tx with unknown algorithm");
			}

			ByteStream stream(rawHex);
			ErrorChecker::CheckParam(!tx->Deserialize(stream, true), Error::InvalidArgument,
									 "Invalid input: deserialize fail");

			return tx;
		}

		nlohmann::json SubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
		                                            const std::string &amount, const std::string &memo) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("toAddr: {}", toAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			BigInt bnAmount;
			bnAmount.setDec(amount);

			std::vector<OutputPtr> outputs;
			Address receiveAddr(toAddress);
			outputs.push_back(OutputPtr(new TransactionOutput(bnAmount, receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::SignTransaction(const nlohmann::json &createdTx,
												  const std::string &payPassword) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", createdTx.dump());
			ArgInfo("passwd: *");

			TransactionPtr tx = DecodeTx(createdTx);

			_walletManager->getWallet()->SignTransaction(tx, payPassword);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::PublishTransaction(const nlohmann::json &signedTx) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", signedTx.dump());

			TransactionPtr tx = DecodeTx(signedTx);

			SPVLOG_DEBUG("publishing tx: {}", tx->ToJson().dump());
			publishTransaction(tx);

			nlohmann::json result;
			result["TxHash"] = tx->GetHash().GetHex();
			result["Fee"] = tx->GetFee();

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::GetAllUTXOs(uint32_t start, uint32_t count, const std::string &address) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("addr: {}", address);
			size_t maxCount = 0, pageCount = 0;

			const WalletPtr &wallet = _walletManager->getWallet();

			std::vector<UTXOPtr> UTXOs = wallet->GetAllUTXO(address);

			maxCount = UTXOs.size();
			nlohmann::json j, jutxos;

			for (size_t i = start; i < UTXOs.size() && pageCount < count; ++i) {
				nlohmann::json item;
				item["Hash"] = UTXOs[i]->Hash().GetHex();
				item["Index"] = UTXOs[i]->Index();
				item["Amount"] = UTXOs[i]->Output()->Amount().getDec();
				jutxos.push_back(item);
				pageCount++;
			}

			j["MaxCount"] = maxCount;
			j["UTXOs"] = jutxos;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::CreateConsolidateTransaction(const std::string &memo) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("memo: {}", memo);

			TransactionPtr tx = CreateConsolidateTx(memo, Asset::GetELAAssetID());

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("addrOrTxID: {}", addressOrTxid);

			const WalletPtr &wallet = _walletManager->getWallet();

			std::vector<TransactionPtr> allTxs = wallet->GetAllTransactions();
			size_t fullTxCount = allTxs.size();
			size_t pageCount = count;
			nlohmann::json j;

			if (start >= fullTxCount) {
				j["Transactions"] = {};
				j["MaxCount"] = fullTxCount;

				ArgInfo("r => {}", j.dump());
				return j;
			}

			if (fullTxCount < start + count)
				pageCount = fullTxCount - start;

			std::vector<TransactionPtr> transactions(pageCount);
			uint32_t realCount = 0;
			for (long i = fullTxCount - 1 - start; i >= 0 && realCount < pageCount; --i) {
				if (!filterByAddressOrTxId(allTxs[i], addressOrTxid))
					continue;
				transactions[realCount++] = allTxs[i];
			}

			std::vector<nlohmann::json> jsonList(realCount);
			for (size_t i = 0; i < realCount; ++i) {
				uint32_t confirms = 0;
				uint32_t lastBlockHeight = _walletManager->getWallet()->LastBlockHeight();
				std::string hash = transactions[i]->GetHash().GetHex();

				confirms = transactions[i]->GetConfirms(lastBlockHeight);

				jsonList[i] = transactions[i]->GetSummary(_walletManager->getWallet(), confirms, !addressOrTxid.empty());
			}
			j["Transactions"] = jsonList;
			j["MaxCount"] = fullTxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::GetAllCoinBaseTransaction(uint32_t start, uint32_t count,
															const std::string &txID) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("txID: {}", txID);

			nlohmann::json j;
			const WalletPtr wallet = _walletManager->getWallet();
			std::vector<UTXOPtr> cbs = wallet->GetAllCoinBaseTransactions();
			size_t maxCount = cbs.size();
			size_t pageCount = count, realCount = 0;

			if (start >= maxCount) {
				j["Transactions"] = {};
				j["MaxCount"] = maxCount;
				ArgInfo("r => {}", j.dump());
				return j;
			}

			if (maxCount < start + count)
				pageCount = maxCount - start;

			if (!txID.empty())
				pageCount = 1;

			std::vector<nlohmann::json> jcbs;
			jcbs.reserve(pageCount);
			for (size_t i = maxCount - start; i > 0 && realCount < pageCount; --i) {
				const UTXOPtr &cbptr = cbs[i - 1];
				nlohmann::json cb;

				if (!txID.empty()) {
					if (cbptr->Hash().GetHex() == txID) {
						cb["TxHash"] = txID;
						uint32_t confirms = cbptr->GetConfirms(_walletManager->getWallet()->LastBlockHeight());
						cb["Timestamp"] = cbptr->Timestamp();
						cb["Amount"] = cbptr->Output()->Amount().getDec();
						cb["Status"] = confirms <= 100 ? "Pending" : "Confirmed";
						cb["Direction"] = "Received";

						cb["ConfirmStatus"] = confirms <= 100 ? std::to_string(confirms) : "100+";
						cb["Height"] = cbptr->BlockHeight();
						cb["Spent"] = cbptr->Spent();
						cb["Address"] = Address(cbptr->Output()->ProgramHash()).String();
						cb["Type"] = Transaction::coinBase;
						jcbs.push_back(cb);
						realCount++;
						break;
					}
				} else {
					nlohmann::json cb;

					cb["TxHash"] = cbptr->Hash().GetHex();
					uint32_t confirms = cbptr->GetConfirms(_walletManager->getWallet()->LastBlockHeight());
					cb["Timestamp"] = cbptr->Timestamp();
					cb["Amount"] = cbptr->Output()->Amount().getDec();
					cb["Status"] = confirms <= 100 ? "Pending" : "Confirmed";
					cb["Direction"] = "Received";

					jcbs.push_back(cb);
					realCount++;
				}
			}
			j["Transactions"] = jcbs;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		void SubWallet::publishTransaction(const TransactionPtr &tx) {
			if (!_walletManager->getWallet()->ContainsTransaction(tx)) {
				ErrorChecker::ThrowLogicException(Error::WalletNotContainTx, "tx do not belong to the current wallet");
			}

			_walletManager->PublishTransaction(tx);
		}

		void SubWallet::balanceChanged(const uint256 &assetID, const BigInt &balance) {
			ArgInfo("{} {} Balance: {}", _walletManager->getWallet()->GetWalletID(), GetFunName(), balance.getDec());
			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&assetID, &balance](ISubWalletCallback *callback) {
							  callback->OnBalanceChanged(assetID.GetHex(), balance.getDec());
						  });
		}

		void SubWallet::onCoinBaseTxAdded(const UTXOPtr &cb) {
			ArgInfo("{} {} Hash:{}", _walletManager->getWallet()->GetWalletID(), GetFunName(), cb->Hash().GetHex());
		}

		void SubWallet::onCoinBaseUpdatedAll(const UTXOArray &cbs) {
			ArgInfo("{} {} cnt: {}", _walletManager->getWallet()->GetWalletID(), GetFunName(), cbs.size());
		}

		void SubWallet::onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {
			ArgInfo("{} {} size: {}, height: {}, timestamp: {}",
					   _walletManager->getWallet()->GetWalletID(), GetFunName(),
					   hashes.size(), blockHeight, timestamp);
		}

		void SubWallet::onCoinBaseSpent(const std::vector<uint256> &spentHashes) {
			ArgInfo("{} {} size: {}: [{},{} {}]",
					   _walletManager->getWallet()->GetWalletID(), GetFunName(),
					   spentHashes.size(), spentHashes.front().GetHex(),
					   (spentHashes.size() > 2 ? " ...," : ""),
					   (spentHashes.size() > 1 ? spentHashes.back().GetHex() : ""));
		}

		void SubWallet::onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			ArgInfo("{} {} Hash: {}, notify: {}, rescan: {}",
					_walletManager->getWallet()->GetWalletID(), GetFunName(),
					hash.GetHex(), notifyUser, recommendRescan);
		}

		void SubWallet::onTxAdded(const TransactionPtr &tx) {
			const uint256 &txHash = tx->GetHash();
			ArgInfo("{} {} Hash: {}, h: {}", _walletManager->getWallet()->GetWalletID(), GetFunName(), txHash.GetHex(), tx->GetBlockHeight());

			fireTransactionStatusChanged(txHash, "Added", nlohmann::json(), 0);
		}

		void SubWallet::onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {
			ArgInfo("{} {} size: {}, height: {}, timestamp: {}", _walletManager->getWallet()->GetWalletID(),
					GetFunName(),
					hashes.size(), blockHeight, timestamp);

			if (_walletManager->GetAllTransactionsCount() == 1) {
				_info->SetEaliestPeerTime(timestamp);
				_parent->_account->Save();
			}

			for (size_t i = 0; i < hashes.size(); ++i) {
				TransactionPtr tx = _walletManager->getWallet()->TransactionForHash(hashes[i]);
				uint32_t confirm = tx->GetConfirms(blockHeight);

				fireTransactionStatusChanged(hashes[i], "Updated", nlohmann::json(), confirm);
			}
		}

		void SubWallet::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			ArgInfo("{} {} hash: {}, notify: {}, rescan: {}", _walletManager->getWallet()->GetWalletID(), GetFunName(),
					hash.GetHex(), notifyUser, recommendRescan);

			fireTransactionStatusChanged(hash, "Deleted", nlohmann::json(), 0);
		}

		void SubWallet::onTxUpdatedAll(const std::vector<TransactionPtr> &txns) {
			ArgInfo("{} {} tx cnt: {}", _walletManager->getWallet()->GetWalletID(), GetFunName(), txns.size());
		}

		void SubWallet::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			ArgInfo("{} {} asset: {}, amount: {}",
					_walletManager->getWallet()->GetWalletID(), GetFunName(),
					asset->GetName(), amount, controller.GetHex());

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&asset, &amount, &controller](ISubWalletCallback *callback) {
							  callback->OnAssetRegistered(asset->GetHash().GetHex(), asset->ToJson());
			});
		}

		bool SubWallet::filterByAddressOrTxId(const TransactionPtr &tx, const std::string &addressOrTxid) const {

			if (addressOrTxid.empty()) {
				return true;
			}

			const WalletPtr &wallet = _walletManager->getWallet();
			for (InputArray::const_iterator in = tx->GetInputs().cbegin(); in != tx->GetInputs().cend(); ++in) {
				TransactionPtr t = wallet->TransactionForHash((*in)->TxHash());
				if (t) {
					OutputPtr o = t->OutputOfIndex((*in)->Index());
					if (o && o->Addr().String() == addressOrTxid)
						return true;
				}
			}
			for (OutputArray::const_iterator o = tx->GetOutputs().cbegin(); o != tx->GetOutputs().cend(); ++o) {
				if ((*o)->Addr().String() == addressOrTxid) {
					return true;
				}
			}

			if (addressOrTxid.length() == 64) {
				if (uint256(addressOrTxid) == tx->GetHash()) {
					return true;
				}
			}

			return false;
		}

		void SubWallet::syncStarted() {
		}

		void SubWallet::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime) {
			struct tm tm;

			localtime_r(&lastBlockTime, &tm);
			ArgInfo("{} {} [ {} / {} ] {}", _walletManager->getWallet()->GetWalletID(), GetFunName(),
					currentHeight, estimatedHeight, asctime(&tm));

			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&currentHeight, &estimatedHeight, &lastBlockTime](ISubWalletCallback *callback) {
							  callback->OnBlockSyncProgress(currentHeight, estimatedHeight, lastBlockTime);
						  });
		}

		void SubWallet::syncStopped(const std::string &error) {
		}

		void SubWallet::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
		}

		void SubWallet::txPublished(const std::string &hash, const nlohmann::json &result) {
			ArgInfo("{} {} hash: {} result: {}", _walletManager->getWallet()->GetWalletID(), GetFunName(), hash, result.dump());

			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(), [&hash, &result](ISubWalletCallback *callback) {
				callback->OnTxPublished(hash, result);
			});
		}

		void SubWallet::connectStatusChanged(const std::string &status) {
			ArgInfo("{} {} status: {}", _walletManager->getWallet()->GetWalletID(), GetFunName(), status);

			boost::mutex::scoped_lock scopedLock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(), [&status](ISubWalletCallback *callback) {
				callback->OnConnectStatusChanged(status);
			});
		}

		void SubWallet::fireTransactionStatusChanged(const uint256 &txid, const std::string &status,
													 const nlohmann::json &desc, uint32_t confirms) {
			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&txid, &status, &desc, confirms](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(txid.GetHex(), status, desc, confirms);
						  });
		}

		const CoinInfoPtr &SubWallet::GetCoinInfo() const {
			return _info;
		}

		void SubWallet::StartP2P() {
			_walletManager->SyncStart();
		}

		void SubWallet::StopP2P() {
			_walletManager->SyncStop();
			_walletManager->ExecutorStop();
		}

		void SubWallet::FlushData() {
			_walletManager->DatabaseFlush();
		}

		void SubWallet::SyncStart() {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			_walletManager->SyncStart();
		}

		void SubWallet::SyncStop() {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			_walletManager->SyncStop();
		}

		nlohmann::json SubWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			nlohmann::json j;
			j["Info"] = _subAccount->GetBasicInfo();
			j["ChainID"] = _info->GetChainID();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::GetTransactionSignedInfo(const nlohmann::json &encodedTx) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", encodedTx.dump());

			TransactionPtr tx = DecodeTx(encodedTx);

			nlohmann::json info = tx->GetSignedInfo();

			ArgInfo("r => {}", info.dump());

			return info;
		}

		nlohmann::json SubWallet::GetAssetInfo(const std::string &assetID) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);

			nlohmann::json info;

			AssetPtr asset = _walletManager->getWallet()->GetAsset(uint256(assetID));
			info["Registered"] = (asset != nullptr);
			if (asset != nullptr)
				info["Info"] = asset->ToJson();
			else
				info["Info"] = {};

			ArgInfo("r => {}", info.dump());
			return info;
		}

	}
}
