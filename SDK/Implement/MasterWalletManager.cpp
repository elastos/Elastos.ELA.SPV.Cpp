// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ByteStream.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Base64.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/SidechainMerkleBlock.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Plugin/ELAPlugin.h>
#include <SDK/Plugin/IDPlugin.h>
#include <Interface/MasterWalletManager.h>
#include <CMakeConfig.h>


#include <boost/filesystem.hpp>

using namespace boost::filesystem;

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define LOCAL_STORE_FILE "LocalStore.json"

namespace Elastos {
	namespace ElaWallet {

		MasterWalletManager::MasterWalletManager(const std::string &rootPath, const std::string &dataPath) :
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(true) {
			ErrorChecker::CheckParamNotEmpty(rootPath, "rootPath");
			initMasterWallets();
		}

		MasterWalletManager::MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath, const std::string &dataPath) :
				_masterWalletMap(walletMap),
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(true) {
		}

		MasterWalletManager::~MasterWalletManager() {
			for (MasterWalletMap::iterator it = _masterWalletMap.begin(); it != _masterWalletMap.end();) {
				MasterWallet *masterWallet = static_cast<MasterWallet *>(it->second);
				masterWallet->CloseAllSubWallets();
				it = _masterWalletMap.erase(it);

				delete masterWallet;
			}
		}

		std::string MasterWalletManager::GenerateMnemonic(const std::string &language, int wordCount) const {
			ArgInfo("{}", GetFunName());
			ArgInfo("language: {}", language);
			ArgInfo("wordCount: {}", wordCount);

			Mnemonic::WordCount count = Mnemonic::WordCount(wordCount);
			std::string mnemonic = MasterWallet::GenerateMnemonic(language, _rootPath, count);

			ArgInfo("r => {}", "*");
			return mnemonic;
		}

		std::string MasterWalletManager::GetMultiSignPubKey(const std::string &phrase, const std::string &phrasePassword) const {

			ArgInfo("{}", GetFunName());
			ArgInfo("phrase: {}", "*");
			ArgInfo("passphrase: *, empty: {}", phrasePassword.empty());

			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");

			Mnemonic mnemonic = Mnemonic(boost::filesystem::path(_rootPath));

			uint512 seed = mnemonic.DeriveSeed(phrase, phrasePassword);

			HDSeed hdseed(seed.bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));

			bytes_t pubkey = rootKey.getChild("1'/0").pubkey();
			std::string pubString = pubkey.getHex();

			seed = 0;

			ArgInfo("r => {}", pubString);
			return pubString;
		}

		std::string MasterWalletManager::GetMultiSignPubKey(const std::string &privKey) const {
			ArgInfo("{}", GetFunName());
			ArgInfo("prvkey: {}", "*");

			bytes_t prvkey(privKey);

			ErrorChecker::CheckCondition(prvkey.size() != 32, Error::PubKeyFormat,
										 "Private key length do not as expected");
			Key key(prvkey);

			prvkey.clean();

			std::string pubString = key.PubKey().getHex();

			ArgInfo("r => {}", pubString);

			return pubString;
		}

		IMasterWallet *MasterWalletManager::CreateMasterWallet(
				const std::string &masterWalletId,
				const std::string &mnemonic,
				const std::string &phrasePassword,
				const std::string &payPassword,
				bool singleAddress) {

			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletId);
			ArgInfo("mnemonic: {}", "*");
			ArgInfo("passphrase: *, empty: {}", phrasePassword.empty());
			ArgInfo("payPasswd: {}", "*");

			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "mnemonic");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end()) {
				ArgInfo("r => already created: 0x{:x}", (long)_masterWalletMap[masterWalletId]);
				return _masterWalletMap[masterWalletId];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  singleAddress, _p2pEnable, _rootPath, _dataPath,
														  0, CreateNormal);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			ArgInfo("r => 0x{:x}", (long)masterWallet);
			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletID,
																		const nlohmann::json &publicKeys,
																		uint32_t m, time_t timestamp) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("signers: {}", publicKeys.dump());
			ArgInfo("m: {}", m);
			ArgInfo("timestamp: {}", timestamp);

			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckPubKeyJsonArray(publicKeys, m, "Signers");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => 0x{:x}", (long)_masterWalletMap[masterWalletID]);
				return _masterWalletMap[masterWalletID];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, publicKeys, m, _rootPath, _dataPath,
														  _p2pEnable, timestamp, CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;

			ArgInfo("r => 0x{:x}", (long)masterWallet);
			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletID,
																		const std::string &xprv,
																		const std::string &payPassword,
																		const nlohmann::json &publicKeys,
																		uint32_t m, time_t timestamp) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("xprv: {}", "*");
			ArgInfo("payPasswd: {}", "*");
			ArgInfo("publicKeys: {}", publicKeys.dump());
			ArgInfo("m: {}", m);
			ArgInfo("timestamp: {}", timestamp);

			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckPubKeyJsonArray(publicKeys, m - 1, "Signers");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => 0x{:x}", (long)_masterWalletMap[masterWalletID]);
				return _masterWalletMap[masterWalletID];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, xprv, payPassword, publicKeys,
														  m, _rootPath, _dataPath, _p2pEnable, timestamp,
														  CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;

			ArgInfo("r => 0x{:x}", (long)masterWallet);
			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletId,
																		const std::string &mnemonic,
																		const std::string &phrasePassword,
																		const std::string &payPassword,
																		const nlohmann::json &publicKeys,
																		uint32_t m, time_t timestamp) {

			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletId);
			ArgInfo("mnemonic: {}", "*");
			ArgInfo("passphrase: *, empty: {}", phrasePassword.empty());
			ArgInfo("payPasswd: {}", "*");
			ArgInfo("signers: {}", publicKeys.dump());
			ArgInfo("m: {}", m);
			ArgInfo("timestamp: {}", timestamp);

			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "Mnemonic");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");
			ErrorChecker::CheckPubKeyJsonArray(publicKeys, m - 1, "Signers");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end()) {
				ArgInfo("r => 0x{:x}", (long)_masterWalletMap[masterWalletId]);
				return _masterWalletMap[masterWalletId];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  publicKeys, m, _p2pEnable, _rootPath, _dataPath, timestamp,
														  CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			ArgInfo("r => 0x{:x}", (long)masterWallet);
			return masterWallet;
		}

		std::vector<IMasterWallet *> MasterWalletManager::GetAllMasterWallets() const {
			ArgInfo("{}", GetFunName());

			std::vector<IMasterWallet *> result;
			for (MasterWalletMap::const_iterator it = _masterWalletMap.cbegin(); it != _masterWalletMap.cend(); ++it) {
				if (GetMasterWallet(it->first))
					result.push_back(GetMasterWallet(it->first));
			}

			std::string walletID = "";
			for (int i = 0; i < result.size(); ++i) {
				walletID += static_cast<MasterWallet *>(result[i])->GetWalletID() + ", ";
			}

			ArgInfo("r => size: {} list: {}", result.size(), walletID);

			return result;
		};

		void MasterWalletManager::DestroyWallet(const std::string &masterWalletID) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				MasterWallet *masterWallet = static_cast<MasterWallet *>(_masterWalletMap[masterWalletID]);
				masterWallet->RemoveLocalStore();

				masterWallet->CloseAllSubWallets();
				_masterWalletMap.erase(masterWallet->GetWalletID());
				delete masterWallet;
			} else {
				Log::warn("Master wallet is not exist");
			}

			ArgInfo("{} done", GetFunName());
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithKeystore(const std::string &masterWalletID,
													  const nlohmann::json &keystoreContent,
													  const std::string &backupPassword,
													  const std::string &payPassword) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("keystore: {}", "*");
			ArgInfo("backupPasswd: {}", "*");
			ArgInfo("payPasswd: {}", "*");

			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckParam(!keystoreContent.is_object(), Error::KeyStore, "key store should be json object");
			ErrorChecker::CheckPassword(backupPassword, "Backup");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist 0x{:x}", (long)_masterWalletMap[masterWalletID]);
				return _masterWalletMap[masterWalletID];
			}


			MasterWallet *masterWallet = new MasterWallet(masterWalletID, keystoreContent, backupPassword,
														  payPassword, _rootPath, _dataPath, _p2pEnable,
														  ImportFromKeyStore);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			masterWallet->InitSubWallets();

			ArgInfo("r => 0x{:x}", (long)masterWallet);

			return masterWallet;
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithMnemonic(const std::string &masterWalletId, const std::string &mnemonic,
													  const std::string &phrasePassword, const std::string &payPassword,
													  bool singleAddress, time_t timestamp) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletId);
			ArgInfo("mnemonic: {}", "*");
			ArgInfo("passphrase: *, empty: {}", phrasePassword.empty());
			ArgInfo("payPasswd: {}", "*");
			ArgInfo("singleAddr: {}", singleAddress);
			ArgInfo("timestamp: {}", timestamp);

			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "Mnemonic");
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end()) {
				ArgInfo("r => already exist 0x{:x}", (long)_masterWalletMap[masterWalletId]);
				return _masterWalletMap[masterWalletId];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  singleAddress, _p2pEnable, _rootPath, _dataPath, timestamp,
														  ImportFromMnemonic);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			ArgInfo("r => 0x{:x}", (long)masterWallet);

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::ImportReadonlyWallet(
			const std::string &masterWalletID,
			const nlohmann::json &walletJson) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("walletJson: {}", walletJson.dump());

			ErrorChecker::CheckParam(!walletJson.is_object(), Error::KeyStore, "wallet json should be json object");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist 0x{:x}", (long)_masterWalletMap[masterWalletID]);
				return _masterWalletMap[masterWalletID];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, walletJson, _rootPath, _dataPath, _p2pEnable, ImportFromKeyStore);

			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			masterWallet->InitSubWallets();
			ArgInfo("r => {}", (long)masterWallet);

			return masterWallet;
		}

		nlohmann::json
		MasterWalletManager::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													  const std::string &payPassword) const {

			ArgInfo("{}", GetFunName());
			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "Master wallet is null");
			ErrorChecker::CheckPassword(backupPassword, "Backup");
			ArgInfo("masterWallet: {}, 0x{:x}", static_cast<MasterWallet *>(masterWallet)->GetWalletID(), (long)masterWallet);
			ArgInfo("backupPasswd: {}", "*");
			ArgInfo("payPasswd: {}", "*");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			nlohmann::json keystore = wallet->exportKeyStore(backupPassword, payPassword);

			ArgInfo("r => {}", "*");
			return keystore;
		}

		std::string
		MasterWalletManager::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) const {
			ArgInfo("{}", GetFunName());
			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "Master wallet is null");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ArgInfo("masterWallet: {}, 0x{:x}", static_cast<MasterWallet *>(masterWallet)->GetWalletID(), (long)masterWallet);
			ArgInfo("payPasswd: {}", "*");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);

			std::string mnemonic = wallet->exportMnemonic(payPassword);

			ArgInfo("r => {}", "*");

			return mnemonic;
		}

		nlohmann::json MasterWalletManager::ExportReadonlyWallet(IMasterWallet *masterWallet) const {
			ArgInfo("{}", GetFunName());

			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "master wallet is null");
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			ArgInfo("masterWallet: {}, 0x{:x}", wallet->GetWalletID(), (long)masterWallet);

			nlohmann::json keystore = wallet->ExportReadonlyKeyStore();

			ArgInfo("r => {}", keystore.dump());
			return keystore;
		}

		std::string MasterWalletManager::ExportxPrivateKey(IMasterWallet *masterWallet,
														   const std::string &payPasswd) const {
			ArgInfo("{}", GetFunName());

			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "master wallet is null");
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			ArgInfo("masterWallet: {}, 0x{:x}", wallet->GetWalletID(), (long)masterWallet);
			ArgInfo("payPasswd: {}", "*");
			ArgInfo("r => ", "*");

			return wallet->ExportxPrivateKey(payPasswd);
		}

		std::string MasterWalletManager::ExportMasterPublicKey(IMasterWallet *masterWallet) const {
			ArgInfo("{}", GetFunName());

			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "master wallet is null");
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			ArgInfo("masterWallet: {}, 0x{:x}", wallet->GetWalletID(), (long)masterWallet);

			std::string xpub = wallet->ExportMasterPublicKey();
			ArgInfo("r => {}", xpub);

			return xpub;
		}

		std::string MasterWalletManager::GetVersion() const {
			ArgInfo("{}", GetFunName());
			ArgInfo("r => {}", SPVSDK_VERSION_MESSAGE);
			return SPVSDK_VERSION_MESSAGE;
		}

		void MasterWalletManager::initMasterWallets() {

			path rootPath = _rootPath;
			if (_dataPath.empty()) {
				_dataPath = _rootPath;
			}

			ErrorChecker::CheckPathExists(_rootPath);
			ErrorChecker::CheckPathExists(_dataPath);

			Log::registerMultiLogger(_dataPath);

			Log::setLevel(spdlog::level::from_str(SPVSDK_SPDLOG_LEVEL));
			Log::info("spvsdk version {}", SPVSDK_VERSION_MESSAGE);

#ifndef BUILD_SHARED_LIBS
			Log::info("Registering plugin ...");
			REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
			REGISTER_MERKLEBLOCKPLUGIN(SideStandard, getIDPluginComponent);
#endif


			directory_iterator it{_dataPath};
			while (it != directory_iterator{}) {

				path temp = *it;
				if (!exists(temp) || !is_directory(temp)) {
					++it;
					continue;
				}

				std::string masterWalletID = temp.filename().string();
				if (exists((*it) / LOCAL_STORE_FILE) || exists((*it) / MASTER_WALLET_STORE_FILE)) {
					MasterWallet *masterWallet = new MasterWallet(masterWalletID, _rootPath, _dataPath, _p2pEnable, ImportFromLocalStore);

					checkRedundant(masterWallet);
					_masterWalletMap[masterWalletID] = masterWallet;
					masterWallet->InitSubWallets();
				}
				++it;
			}
		}

		std::vector<std::string> MasterWalletManager::GetAllMasterWalletIds() const {
			ArgInfo("{}", GetFunName());

			std::vector<std::string> result;
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [&result](const MasterWalletMap::value_type &item) {
							  result.push_back(item.first);
						  });

			std::string chainID = "";
			for(size_t i = 0; i < result.size(); ++i) {
				chainID += result[i] + ", ";
			}
			ArgInfo("r => size: {} list: {}", result.size(), chainID);

			return result;
		}

		IMasterWallet *MasterWalletManager::GetMasterWallet(const std::string &masterWalletId) const {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletId);

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.cend() &&
				_masterWalletMap[masterWalletId] != nullptr) {
				ArgInfo("r => 0x{:x}", (long)_masterWalletMap[masterWalletId]);
				return _masterWalletMap[masterWalletId];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, _rootPath, _dataPath, _p2pEnable, ImportFromLocalStore);

			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			masterWallet->InitSubWallets();

			ArgInfo("r => 0x{:x}", (long)masterWallet);
			return masterWallet;
		}

		void MasterWalletManager::checkRedundant(IMasterWallet *wallet) const {

			MasterWallet *masterWallet = static_cast<MasterWallet *>(wallet);

			bool hasRedundant = false;
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [masterWallet, &hasRedundant](const MasterWalletMap::value_type &item) {
							  if (item.second != nullptr) {
								  const MasterWallet *createdWallet = static_cast<const MasterWallet *>(item.second);
								  if (!hasRedundant)
									  hasRedundant = masterWallet->IsEqual(*createdWallet);
							  }
						  });

			if (hasRedundant) {
				Log::info("{} Destroying redundant wallet", masterWallet->GetWalletID());

				std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
				for (int i = 0; i < subWallets.size(); ++i) {
					masterWallet->DestroyWallet(subWallets[i]);
				}

				if (masterWallet->_initFrom == ImportFromLocalStore) {
					boost::filesystem::path filepath = _dataPath;
					filepath /= LOCAL_STORE_FILE;
					if (boost::filesystem::exists(filepath)) {
						Log::info("rename {}", filepath.string());
						boost::filesystem::rename(filepath, filepath / ".bak");
					}

					filepath = _dataPath;
					filepath /= MASTER_WALLET_STORE_FILE;
					if (boost::filesystem::exists(filepath)) {
						Log::info("rename {}", filepath.string());
						boost::filesystem::rename(filepath, filepath / ".bak");
					}
				} else {
					Log::info("Clearing local", masterWallet->GetID());
					masterWallet->RemoveLocalStore();
				}

				delete masterWallet;
			}

			ErrorChecker::CheckCondition(hasRedundant, Error::CreateMasterWalletError,
										 "Master wallet already exist.");
		}
	}
}