// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLETMANAGER_H__
#define __ELASTOS_SDK_MASTERWALLETMANAGER_H__

#include <map>

#include "IMasterWalletManager.h"

namespace Elastos {
	namespace ElaWallet {

		class Config;

		class MasterWalletManager : public IMasterWalletManager {
		public:
			/**
			 * Constructor
			 * @param rootPath Specify directory for all config files, including mnemonic config files. Root should not be empty, otherwise will throw invalid argument exception.
			 * @param netType Value can only be one of "MainNet", "TestNet", "RegTest", "PrvNet"
			 * @param dataPath The path contains data of wallet created. If empty, data of wallet will store in rootPath.
			 * @param config If netType is "MainNet" or "TestNet" or "RegTest", config will be ignore.
			 *
			 * If netType is "PrvNet", config must be provided as below:
			 * {
			 * 	"ELA": {
			 * 		"GenesisAddress": "",
			 * 		"ChainParameters": {
			 * 			"MagicNumber": 20190808,
			 * 			"StandardPort": 30018,
			 * 			"DNSSeeds": [ "172.26.0.165" ],
			 * 			"CheckPoints": [
			 * 				[0, "8df798783097be3a8f382f6537e47c7168d4bf4d741fa3fa8c48c607a06352cf", 1513936800, 486801407]
			 * 			]
			 * 		}
			 * 	},
			 * 	"IDChain": {
			 * 		"GenesisAddress": "XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ",
			 * 		"ChainParameters": {
			 * 			"MagicNumber": 20190816,
			 * 			"StandardPort": 30138,
			 * 			"DNSSeeds": [ "172.26.0.165" ],
			 * 			"CheckPoints": [
			 * 				[0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1513936800, 486801407]
			 * 			]
			 * 		}
			 * 	}
			 * }
			 */
			explicit MasterWalletManager(const std::string &rootPath, const std::string &netType = "MainNet",
										 const nlohmann::json &config = nlohmann::json(),
										 const std::string &dataPath = "");

			virtual ~MasterWalletManager();

			virtual std::string GenerateMnemonic(const std::string &language, int wordCount = 12) const;

			virtual IMasterWallet *CreateMasterWallet(
				const std::string &masterWalletId,
				const std::string &mnemonic,
				const std::string &phrasePassword,
				const std::string &payPassword,
				bool singleAddress);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
				const std::string &masterWalletId,
				const nlohmann::json &cosigners,
				uint32_t m,
				bool singleAddress,
				bool compatible = false,
				time_t timestamp = 0);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
				const std::string &masterWalletId,
				const std::string &xprv,
				const std::string &payPassword,
				const nlohmann::json &cosigners,
				uint32_t m,
				bool singleAddress,
				bool compatible = false,
				time_t timestamp = 0);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
				const std::string &masterWalletId,
				const std::string &mnemonic,
				const std::string &passphrase,
				const std::string &payPassword,
				const nlohmann::json &cosigners,
				uint32_t m,
				bool singleAddress,
				bool compatible = false,
				time_t timestamp = 0);

			virtual std::vector<IMasterWallet *> GetAllMasterWallets() const;

			virtual std::vector<std::string> GetAllMasterWalletID() const;

			virtual IMasterWallet *GetMasterWallet(
				const std::string &masterWalletId) const;

			virtual void DestroyWallet(
				const std::string &masterWalletId);

			virtual IMasterWallet *ImportWalletWithKeystore(
				const std::string &masterWalletId,
				const nlohmann::json &keystoreContent,
				const std::string &backupPassword,
				const std::string &payPassword);

			virtual IMasterWallet *ImportWalletWithMnemonic(
				const std::string &masterWalletId,
				const std::string &mnemonic,
				const std::string &phrasePassword,
				const std::string &payPassword,
				bool singleAddress,
				time_t timestamp = 0);

			virtual IMasterWallet *ImportReadonlyWallet(
				const std::string &masterWalletID,
				const nlohmann::json &walletJson);

			virtual std::string GetVersion() const;

			virtual void FlushData();

		protected:
			typedef std::map<std::string, IMasterWallet *> MasterWalletMap;

			MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath,
								const std::string &dataPath);

			void Load();

			void checkRedundant(IMasterWallet *wallet) const;

		protected:
			Config *_config;
			std::string _rootPath;
			std::string _dataPath;
			bool _p2pEnable;
			mutable MasterWalletMap _masterWalletMap;
		};
	}
}

#endif //__ELASTOS_SDK_MASTERWALLETMANAGER_H__
