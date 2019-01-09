// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLET_H__
#define __ELASTOS_SDK_MASTERWALLET_H__

#include <SDK/KeyStore/CoinConfig.h>
#include <SDK/KeyStore/Mnemonic.h>
#include <SDK/KeyStore/KeyStore.h>
#include <SDK/KeyStore/CoinInfo.h>
#include <SDK/KeyStore/MasterWalletStore.h>
#include <SDK/Crypto/MasterPubKey.h>
#include <SDK/IdAgent/IdAgentImpl.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Transaction/Transaction.h>

#include <Interface/IMasterWallet.h>
#include <Interface/IIdAgent.h>

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo;

		class ChainParams;

		class SubWallet;

		class KeyStore;

		typedef enum {
			CreateNormal,          // Select newest check point
			CreateMultiSign,       // Select oldest check point
			ImportFromMnemonic,    // Select oldest check point
			ImportFromLocalStore,  // Select check point from local store
			ImportFromKeyStore,    // Select check point from key store
			ImportFromOldKeyStore, // Select oldest check point
		} MasterWalletInitFrom;

		class MasterWallet : public IMasterWallet, public IIdAgent {
		public:
			virtual ~MasterWallet();

			virtual void Save();

			virtual void ClearLocal();

			bool IsEqual(const MasterWallet &wallet) const;

			MasterPubKeyPtr GetMasterPubKey(const std::string &chainID) const;

		public: //override from IMasterWallet

			static std::string GenerateMnemonic(const std::string &language, const std::string &rootPath);

			virtual std::string GetId() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual std::vector<ISubWallet *> GetAllSubWallets() const;

			virtual ISubWallet *CreateSubWallet(
					const std::string &chainID,
					uint64_t feePerKb = 0);

			virtual ISubWallet *RecoverSubWallet(
					const std::string &chainID,
					uint32_t limitGap,
					uint64_t feePerKb = 0);

			virtual void DestroyWallet(ISubWallet *wallet);

			virtual std::string GetPublicKey() const;

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword);

			virtual nlohmann::json CheckSign(
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature);

			virtual bool IsAddressValid(const std::string &address) const;

			virtual std::vector<std::string> GetSupportedChains() const;

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

		public: //override from IIdAgent
			virtual std::string DeriveIdAndKeyForPurpose(
					uint32_t purpose,
					uint32_t index);

			virtual bool IsIdValid(const std::string &id);

			virtual nlohmann::json GenerateProgram(
					const std::string &id,
					const std::string &message,
					const std::string &password);

			virtual std::string Sign(
					const std::string &id,
					const std::string &message,
					const std::string &password);

			virtual std::vector<std::string> GetAllIds() const;

			virtual std::string GetPublicKey(const std::string &id) const;

		protected:

			friend class MasterWalletManager;

			friend class IdAgentImpl;

			friend class SubWallet;

			typedef std::map<std::string, ISubWallet *> WalletMap;

			MasterWallet(
					const boost::filesystem::path &localStore,
					const std::string &rootPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress,
					bool p2pEnable,
					const std::string &rootPath,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const nlohmann::json &keystoreContent,
					const std::string &backupPassword,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const std::string &rootPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const nlohmann::json &keystoreContent,
					const std::string &backupPassword,
					const std::string &payPassword,
					const std::string &rootPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount,
					const std::string &rootPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &privKey,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount,
					const std::string &rootPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount,
					bool p2pEnable,
					const std::string &rootPath,
					MasterWalletInitFrom from);

			// to support old web keystore
			void importFromKeyStore(const nlohmann::json &keystoreContent,
									const std::string &backupPassword,
									const std::string &payPassword,
									const std::string &phrasePassword);

			void importFromKeyStore(const nlohmann::json &keystoreContent,
									const std::string &backupPassword,
									const std::string &payPassword);

			void importFromMnemonic(const std::string &mnemonic,
									const std::string &phrasePassword,
									const std::string &payPassword);

			nlohmann::json exportKeyStore(const std::string &backupPassword,
										  const std::string &payPassword);

			bool exportMnemonic(const std::string &payPassword,
								std::string &mnemonic);

			void initFromLocalStore(const MasterWalletStore &localStore);

			void initFromKeyStore(const KeyStore &keyStore,
								  const std::string &payPassword);

			void initFromMultiSigners(
					const std::string &privKey,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount);

			void initFromMultiSigners(
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount);

			void initSubWallets(const std::vector<CoinInfo> &coinInfoList);

			void restoreSubWallets(const std::vector<CoinInfo> &coinInfoList);

			void restoreKeyStore(KeyStore &keyStore, const std::string &payPassword);

			void restoreLocalStore();

			SubWallet *SubWalletFactoryMethod(const CoinInfo &info,
											  const CoinConfig &config,
											  const ChainParams &chainParams,
											  MasterWallet *parent);


			virtual void startPeerManager(SubWallet *wallet);

			virtual void stopPeerManager(SubWallet *wallet);

			void tryInitCoinConfig() const;

			void initSubWalletsPubKeyMap(const std::string &payPassword);

		protected:
			WalletMap _createdWallets;

			MasterWalletStore _localStore;

			MasterWalletInitFrom _initFrom;

			std::string _id;
			std::string _rootPath;

			mutable CoinConfigReader _coinConfigReader;
			boost::shared_ptr<IdAgentImpl> _idAgentImpl;
			bool _p2pEnable;

		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLET_H__
