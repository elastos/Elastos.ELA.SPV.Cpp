// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLETSTORE_H__
#define __ELASTOS_SDK_MASTERWALLETSTORE_H__

#include <SDK/Account/IAccount.h>
#include <SDK/SpvService/CoinInfo.h>
#include <SDK/IdAgent/IdAgentImpl.h>

#include <boost/filesystem.hpp>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		typedef std::map<std::string, HDKeychain> MasterPubKeyMap;
		typedef std::map<std::string, bytes_t> VotePubKeyMap;

		class MasterWalletStore {
		public:
			MasterWalletStore(const std::string &rootPath);

			~MasterWalletStore();

			void Load(const boost::filesystem::path &path);

			void Save(const boost::filesystem::path &path);

			bool IsSingleAddress() const;

			bool &IsSingleAddress();

			const IdAgentInfo &GetIdAgentInfo() const;

			void SetIdAgentInfo(const IdAgentInfo &info);

			const std::vector<CoinInfo> &GetSubWalletInfoList() const;

			void SetSubWalletInfoList(const std::vector<CoinInfo> &infoList);

			HDKeychain GetMasterPubKey(const std::string &chainID) const;

			const MasterPubKeyMap &GetMasterPubKeyMap() const;

			void SetMasterPubKeyMap(const MasterPubKeyMap &map);

			const VotePubKeyMap &GetVotePublicKeyMap() const;

			bytes_t GetVotePublicKey(const std::string &chainID) const;

			void SetVotePublicKeyMap(const VotePubKeyMap &publicKey);

			IAccount *Account() const;

			void Reset(IAccount *account);

			void Reset(const std::string &phrase,
					   const std::string &phrasePassword,
					   const std::string &payPassword);

			void Reset(const nlohmann::json &coSigners,
					   uint32_t requiredSignCount);

			void Reset(const std::string &privKey,
					   const nlohmann::json &coSigners,
					   const std::string &payPassword,
					   uint32_t requiredSignCount);

			void Reset(const std::string &phrase,
					   const std::string &phrasePassword,
					   const nlohmann::json &coSigners,
					   const std::string &payPassword,
					   uint32_t requiredSignCount);

		private:
			JSON_SM_LS(MasterWalletStore);

			JSON_SM_RS(MasterWalletStore);

			TO_JSON(MasterWalletStore);

			FROM_JSON(MasterWalletStore);

		private:

			bool _isSingleAddress;
			std::string _rootPath;
			AccountPtr _account;
			IdAgentInfo _idAgentInfo;
			std::vector<CoinInfo> _subWalletsInfoList;

			mutable VotePubKeyMap _votePublicKeyMap;
			mutable MasterPubKeyMap _subWalletsPubKeyMap;
		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLETSTORE_H__
