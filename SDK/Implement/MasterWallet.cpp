// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDChainSubWallet.h"
#include "SidechainSubWallet.h"
#include "MainchainSubWallet.h"
#include "SubWallet.h"
#include "MasterWallet.h"
#include "TokenchainSubWallet.h"

#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Payload/RegisterIdentification.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/SpvService/Config.h>
#include <CMakeConfig.h>

#include <vector>
#include <boost/filesystem.hpp>

#define COIN_COINFIG_FILE "CoinConfig.json"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(0) {

			_config = ConfigPtr(new Config(rootPath));
			_account = AccountPtr(new Account(dataPath + "/" + _id));
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &mnemonic,
								   const std::string &passphrase,
								   const std::string &payPasswd,
								   bool singleAddress,
								   bool p2pEnable,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_earliestPeerTime(earliestPeerTime),
				_initFrom(from) {

			Mnemonic m(rootPath);
			ErrorChecker::CheckLogic(!m.Validate(mnemonic), Error::Mnemonic, "Invalid mnemonic");

			_config = ConfigPtr(new Config(rootPath));
			_account = AccountPtr(new Account(dataPath + "/" + _id, mnemonic, passphrase, payPasswd, singleAddress));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &keystoreContent,
								   const std::string &backupPassword,
								   const std::string &payPasswd,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_earliestPeerTime(0),
				_initFrom(from) {

			KeyStore keystore;
			keystore.Import(keystoreContent, backupPassword);

			_config = ConfigPtr(new Config(rootPath));

			_account = AccountPtr(new Account(dataPath + "/" + _id, keystore, payPasswd));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &readonlyWalletJson,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
			_id(id),
			_p2pEnable(p2pEnable),
			_initFrom(from),
			_earliestPeerTime(0) {

			_config = ConfigPtr(new Config(rootPath));
			_account = AccountPtr(new Account(dataPath + "/" + _id, readonlyWalletJson));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::vector<PublicKeyRing> &pubKeyRings,
								   uint32_t m,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   bool singleAddress,
								   bool compatible,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime) {
			ErrorChecker::CheckParam(pubKeyRings.size() < m, Error::InvalidArgument, "Invalid M");

			_config = ConfigPtr(new Config(rootPath));
			_account = AccountPtr(new Account(dataPath + "/" + _id, pubKeyRings, m, singleAddress, compatible));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &xprv,
								   const std::string &payPassword,
								   const std::vector<PublicKeyRing> &cosigners,
								   uint32_t m,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   bool singleAddress,
								   bool compatible,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime) {

			ErrorChecker::CheckParam(cosigners.size() + 1 < m, Error::InvalidArgument, "Invalid M");

			_config = ConfigPtr(new Config(rootPath));
			_account = AccountPtr(new Account(dataPath + "/" + _id, xprv, payPassword, cosigners, m, singleAddress, compatible));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &mnemonic,
								   const std::string &passphrase,
								   const std::string &payPasswd,
								   const std::vector<PublicKeyRing> &cosigners,
								   uint32_t m,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   bool singleAddress,
								   bool compatible,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime) {

			ErrorChecker::CheckParam(cosigners.size() + 1 < m, Error::InvalidArgument, "Invalid M");

			_config = ConfigPtr(new Config(rootPath));
			_account = AccountPtr(new Account(dataPath + "/" + _id, mnemonic, passphrase, payPasswd, cosigners, m, singleAddress, compatible));
			_account->Save();
		}

		MasterWallet::~MasterWallet() {

		}

		std::string MasterWallet::GenerateMnemonic(const std::string &language, const std::string &rootPath,
		                                           Mnemonic::WordCount wordCount) {
			return Mnemonic(boost::filesystem::path(rootPath)).Create(language, wordCount);
		}

		void MasterWallet::RemoveLocalStore() {
			_account->Remove();
		}

		std::string MasterWallet::GetID() const {
			return _id;
		}

		std::string MasterWallet::GetWalletID() const {
			return _id;
		}

		std::vector<ISubWallet *> MasterWallet::GetAllSubWallets() const {
			ArgInfo("{} {}", _id, GetFunName());

			std::vector<ISubWallet *> subwallets;
			for (WalletMap::const_iterator it = _createdWallets.cbegin(); it != _createdWallets.cend(); ++it) {
				subwallets.push_back(it->second);
			}

			std::string result;
			for (size_t i = 0; i < subwallets.size(); ++i)
				result += subwallets[i]->GetChainID() + ",";

			ArgInfo("r => {}", result);
			return subwallets;
		}

		ISubWallet * MasterWallet::CreateSubWallet(const std::string &chainID) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("chainID: {}", chainID);

			ErrorChecker::CheckParamNotEmpty(chainID, "Chain ID");
			ErrorChecker::CheckParam(chainID.size() > 128, Error::InvalidArgument, "Chain ID sould less than 128");


			if (_createdWallets.find(chainID) != _createdWallets.end()) {
				ISubWallet *subWallet = _createdWallets[chainID];
				ArgInfo("r => already created");
				return subWallet;
			}

			ChainConfigPtr chainConfig = _config->GetChainConfig(chainID);
			ErrorChecker::CheckLogic(chainConfig == nullptr, Error::InvalidArgument, "Unsupport chain ID: " + chainID);

			CoinInfoPtr info(new CoinInfo());

			info->SetChainID(chainID);
			info->SetVisibleAsset(Asset::GetELAAssetID());

			_account->AddSubWalletInfoList(info);
			SubWallet *subWallet = SubWalletFactoryMethod(info, chainConfig, this);
			_createdWallets[chainID] = subWallet;
			_account->Save();
			startPeerManager(subWallet);

			ArgInfo("r => create subwallet");
			subWallet->GetBasicInfo();

			return subWallet;
		}

		bool MasterWallet::VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("mnemonic: *");
			ArgInfo("passphrase: *");
			bool r = _account->VerifyPrivateKey(mnemonic, passphrase);
			ArgInfo("r => {}", r);
			return r;
		}

		bool MasterWallet::VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("passphrase: *");
			ArgInfo("payPasswd: *");
			bool r = _account->VerifyPassPhrase(passphrase, payPasswd);
			ArgInfo("r => {}", r);
			return r;
		}

		bool MasterWallet::VerifyPayPassword(const std::string &payPasswd) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("payPasswd: *");
			bool r = _account->VerifyPayPassword(payPasswd);
			ArgInfo("r => {}", r);
			return r;
		}

		void MasterWallet::CloseAllSubWallets() {
			for (WalletMap::iterator it = _createdWallets.begin(); it != _createdWallets.end(); ) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				std::string id = _id + ":" + subWallet->GetChainID();
				Log::info("{} closing...", id);
				stopPeerManager(subWallet);

				it = _createdWallets.erase(it);

				delete subWallet;
				Log::info("{} closed", id);
			}
		}

		void MasterWallet::DestroyWallet(ISubWallet *wallet) {
			ErrorChecker::CheckParam(wallet == nullptr, Error::Wallet, "Destroy wallet can't be null");
			ErrorChecker::CheckParam(_createdWallets.empty(), Error::Wallet, "There is no sub wallet in this wallet.");

			SubWallet *subWallet = dynamic_cast<SubWallet *>(wallet);
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("subWallet: {}", subWallet->GetInfoChainID());

			if (_createdWallets.find(subWallet->GetInfoChainID()) == _createdWallets.end())
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Sub wallet did not created");

			if (_createdWallets[subWallet->GetInfoChainID()] != wallet)
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Sub wallet does not belong to this master wallet");

			_account->RemoveSubWalletInfo(subWallet->_info);
			_account->Save();

			stopPeerManager(subWallet);
			WalletMap::iterator it = _createdWallets.find(subWallet->GetInfoChainID());
			_createdWallets.erase(it);

			delete subWallet;

			ArgInfo("r => {} {} done", _id, GetFunName());
		}

		nlohmann::json MasterWallet::GetPubKeyInfo() const {
			ArgInfo("{} {}", _id, GetFunName());

			nlohmann::json j = _account->GetPubKeyInfo();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json MasterWallet::ExportReadonlyKeyStore() {
			return _account->ExportReadonlyWallet();
		}

		nlohmann::json MasterWallet::ExportKeyStore(const std::string &backupPassword, const std::string &payPassword) {
			KeyStore keyStore = _account->ExportKeyStore(payPassword);
			return keyStore.Export(backupPassword, true);
		}

		std::string MasterWallet::ExportMnemonic(const std::string &payPassword) {
			return _account->GetDecryptedMnemonic(payPassword);
		}

		std::string MasterWallet::ExportxPrivateKey(const std::string &payPasswd) const {
			ErrorChecker::CheckLogic(_account->Readonly(), Error::UnsupportOperation,
									 "Unsupport operation: read-only wallet do not contain xprv");

			return _account->GetxPrvKeyString(payPasswd);
		}

		std::string MasterWallet::ExportMasterPublicKey() const {
			return _account->MasterPubKeyString();
		}

		void MasterWallet::InitSubWallets() {
			const std::vector<CoinInfoPtr> &info = _account->SubWalletInfoList();

			if (info.size() == 0) {
				ChainConfigPtr mainchainConfig = _config->GetChainConfig("ELA");
				if (mainchainConfig) {
					CoinInfoPtr defaultInfo(new CoinInfo());
					defaultInfo->SetChainID(mainchainConfig->ID());
					defaultInfo->SetVisibleAsset(Asset::GetELAAssetID());

					ISubWallet *subWallet = SubWalletFactoryMethod(defaultInfo, mainchainConfig, this);
					SubWallet *subWalletImpl = dynamic_cast<SubWallet *>(subWallet);
					ErrorChecker::CheckCondition(subWalletImpl == nullptr, Error::CreateSubWalletError,
												 "Recover sub wallet error");
					startPeerManager(subWalletImpl);
					_createdWallets[subWalletImpl->GetInfoChainID()] = subWallet;

					_account->AddSubWalletInfoList(defaultInfo);
					_account->Save();
				}
			} else {
				for (int i = 0; i < info.size(); ++i) {
					ChainConfigPtr chainConfig = _config->GetChainConfig(info[i]->GetChainID());
					if (chainConfig == nullptr) {
						Log::error("Can not find config of chain ID: " + info[i]->GetChainID());
						continue;
					}

					ISubWallet *subWallet = SubWalletFactoryMethod(info[i], chainConfig, this);
					SubWallet *subWalletImpl = dynamic_cast<SubWallet *>(subWallet);
					ErrorChecker::CheckCondition(subWalletImpl == nullptr, Error::CreateSubWalletError,
												 "Recover sub wallet error");
					startPeerManager(subWalletImpl);
					_createdWallets[subWalletImpl->GetInfoChainID()] = subWallet;
				}
			}
		}

		SubWallet *MasterWallet::SubWalletFactoryMethod(const CoinInfoPtr &info, const ChainConfigPtr &config,
														MasterWallet *parent) {

			if (_initFrom == CreateNormal) {
				Log::info("Create new master wallet");
				info->SetEaliestPeerTime(config->ChainParameters()->LastCheckpoint().Timestamp());
			} else if (_initFrom == CreateMultiSign) {
				if (_earliestPeerTime != 0) {
					info->SetEaliestPeerTime(_earliestPeerTime);
				} else {
					info->SetEaliestPeerTime(config->ChainParameters()->FirstCheckpoint().Timestamp());
				}
				Log::info("Create new multi-sign master wallet");
			} else if (_initFrom == ImportFromMnemonic) {
				if (_earliestPeerTime != 0) {
					info->SetEaliestPeerTime(_earliestPeerTime);
				} else {
					info->SetEaliestPeerTime(config->ChainParameters()->FirstCheckpoint().Timestamp());
				}
				Log::info("Import master wallet with mnemonic");
			} else if (_initFrom == ImportFromKeyStore) {
				Log::info("Master wallet import with keystore");
			} else if (_initFrom == ImportFromLocalStore) {
				Log::info("Master wallet init from local store");
			} else {
				Log::error("Should not be here");
				info->SetEaliestPeerTime(config->ChainParameters()->FirstCheckpoint().Timestamp());
			}
			Log::info("{}:{} Ealiest peer time: {}", _id, info->GetChainID(), info->GetEarliestPeerTime());

			if (info->GetChainID() == "ELA") {
				return new MainchainSubWallet(info, config, parent);
			} else if (info->GetChainID() == "IDChain") {
				return new IDChainSubWallet(info, config, parent);
			} else if (info->GetChainID() == "TokenChain") {
				return new TokenchainSubWallet(info, config, parent);
			} else {
				ErrorChecker::ThrowLogicException(Error::InvalidChainID, "Invalid chain ID: " + info->GetChainID());
			}

			return nullptr;
		}

		std::string MasterWallet::GetDataPath() const {
			return _account->GetDataPath();
		}

		void MasterWallet::startPeerManager(SubWallet *wallet) {
			if (_p2pEnable)
				wallet->StartP2P();
		}

		void MasterWallet::stopPeerManager(SubWallet *wallet) {
			if (_p2pEnable)
				wallet->StopP2P();
		}

		bool MasterWallet::IsAddressValid(const std::string &address) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("addr: {}", address);

			bool result = Address(address).Valid();

			ArgInfo("r => {}", result);
			return result;
		}

		std::vector<std::string> MasterWallet::GetSupportedChains() const {
			ArgInfo("{} {}", _id, GetFunName());

			std::vector<std::string> chainIDs;

			const std::vector<ChainConfigPtr> &chainConfigs = _config->GetChainConfigs();
			for (size_t i = 0; i < chainConfigs.size(); ++i)
				chainIDs.push_back(chainConfigs[i]->ID());

			std::string chainID = "";
			for (size_t i = 0; i < chainIDs.size(); ++i) {
				chainID += chainIDs[i] + ", ";
			}

			ArgInfo("r => size: {} list: {}", chainIDs.size(), chainID);
			return chainIDs;
		}

		void MasterWallet::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("old: *");
			ArgInfo("new: *");

			_account->ChangePassword(oldPassword, newPassword);
		}

		nlohmann::json MasterWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _id, GetFunName());

			nlohmann::json info = _account->GetBasicInfo();

			ArgInfo("r => {}", info.dump());
			return info;
		}

		bool MasterWallet::IsEqual(const MasterWallet &wallet) const {
			return _account->Equal(wallet._account);
		}

		void MasterWallet::FlushData() {
			for (WalletMap::const_iterator it = _createdWallets.cbegin(); it != _createdWallets.cend(); ++it) {
				SubWallet *subWallet = dynamic_cast<SubWallet*>(it->second);
				subWallet->FlushData();
			}
		}

	}
}
