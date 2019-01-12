// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterWalletStore.h"

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Account/StandardAccount.h>
#include <SDK/Account/MultiSignAccount.h>
#include <SDK/Account/SimpleAccount.h>
#include <SDK/Account/AccountFactory.h>

#include <fstream>

namespace Elastos {
	namespace ElaWallet {

		MasterWalletStore::MasterWalletStore(const std::string &rootPath) : _rootPath(rootPath) {
		}

		MasterWalletStore::~MasterWalletStore() {

		}

		void MasterWalletStore::Load(const boost::filesystem::path &path) {
			ParamChecker::checkPathExists(path);

			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;
			j >> *this;
		}

		void MasterWalletStore::Save(const boost::filesystem::path &path) {

			nlohmann::json j;
			j << *this;
			std::ofstream o(path.string());
			o << j;
			o.flush();
		}

		const std::vector<CoinInfo> &MasterWalletStore::GetSubWalletInfoList() const {
			return _subWalletsInfoList;
		}

		void MasterWalletStore::SetSubWalletInfoList(const std::vector<CoinInfo> &infoList) {
			_subWalletsInfoList = infoList;
		}

		MasterPubKeyPtr MasterWalletStore::GetMasterPubKey(const std::string &chainID) const {
			if (_subWalletsPubKeyMap.find(chainID) == _subWalletsPubKeyMap.end()) {
				return nullptr;
			}
			return _subWalletsPubKeyMap[chainID];
		}

		const MasterPubKeyMap &MasterWalletStore::GetMasterPubKeyMap() const {
			return _subWalletsPubKeyMap;
		}

		void MasterWalletStore::SetMasterPubKeyMap(const MasterPubKeyMap &map) {
			_subWalletsPubKeyMap = map;
		}

		const VotePubKeyMap &MasterWalletStore::GetVotePublicKeyMap() const {
			return _votePublicKeyMap;
		}

		const CMBlock &MasterWalletStore::GetVotePublicKey(const std::string &chainID) const {
			if (_votePublicKeyMap.find(chainID) == _votePublicKeyMap.end())
				return CMBlock();

			return _votePublicKeyMap[chainID];
		}

		void MasterWalletStore::SetVotePublicKeyMap(const VotePubKeyMap &map) {
			_votePublicKeyMap = map;
		}

		const IdAgentInfo &MasterWalletStore::GetIdAgentInfo() const {
			return _idAgentInfo;
		}

		void MasterWalletStore::SetIdAgentInfo(const IdAgentInfo &info) {
			_idAgentInfo = info;
		}

		nlohmann::json &operator<<(nlohmann::json &j, const MasterWalletStore &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, MasterWalletStore &p) {
			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const MasterWalletStore &p) {
			j["Account"] = p.Account()->ToJson();
			j["AccountType"] = p.Account()->GetType();
			j["IsSingleAddress"] = p.IsSingleAddress();
			j["IdAgent"] = p.GetIdAgentInfo();
			std::vector<nlohmann::json> subWallets;
			for (size_t i = 0; i < p.GetSubWalletInfoList().size(); i++) {
				subWallets.push_back(p.GetSubWalletInfoList()[i]);
			}
			j["SubWallets"] = subWallets;

			nlohmann::json masterPubKey;
			const MasterPubKeyMap &map = p.GetMasterPubKeyMap();
			for (MasterPubKeyMap::const_iterator it = map.cbegin(); it != map.cend(); it++) {
				ByteStream stream;
				stream.setPosition(0);
				it->second->Serialize(stream);
				masterPubKey[it->first] = Utils::encodeHex(stream.getBuffer());
			}
			j["MasterPubKey"] = masterPubKey;

			nlohmann::json votePubKey;
			for (VotePubKeyMap::const_iterator it = p.GetVotePublicKeyMap().cbegin(); it != p.GetVotePublicKeyMap().cend(); ++it) {
				votePubKey[it->first] = Utils::encodeHex(it->second);
			}
			j["VotePublicKey"] = votePubKey;
		}

		void from_json(const nlohmann::json &j, MasterWalletStore &p) {
			p._account = AccountPtr(AccountFactory::CreateFromJson(p._rootPath, j));
			p.SetIdAgentInfo(j["IdAgent"]);
			p.IsSingleAddress() = j["IsSingleAddress"].get<bool>();

			std::vector<CoinInfo> coinInfoList;
			std::vector<nlohmann::json> subWallets = j["SubWallets"];
			for (size_t i = 0; i < subWallets.size(); i++) {
				coinInfoList.push_back(subWallets[i]);
			}
			p.SetSubWalletInfoList(coinInfoList);

			MasterPubKeyMap masterPubKeyMap;
			nlohmann::json masterPubKeyJson = j["MasterPubKey"];
			for (nlohmann::json::iterator it = masterPubKeyJson.begin(); it != masterPubKeyJson.end(); ++it) {
				CMBlock value = Utils::decodeHex(it.value());
				MasterPubKeyPtr masterPubKey = MasterPubKeyPtr(new MasterPubKey());
				ByteStream stream(value);
				masterPubKey->Deserialize(stream);
				masterPubKeyMap[it.key()] = masterPubKey;
			}
			p.SetMasterPubKeyMap(masterPubKeyMap);

			VotePubKeyMap votePubKeyMap;
			nlohmann::json votePubKeyJson = j["VotePublicKey"];
			for (nlohmann::json::iterator it = votePubKeyJson.begin(); it != votePubKeyJson.end(); ++it) {
				votePubKeyMap[it.key()] = Utils::decodeHex(it.value());
			}
			p.SetVotePublicKeyMap(votePubKeyMap);
		}

		IAccount *MasterWalletStore::Account() const {
			return _account.get();
		}

		void MasterWalletStore::Reset(Elastos::ElaWallet::IAccount *account) {
			_account = AccountPtr(account);
		}

		void MasterWalletStore::Reset(const std::string &phrase,
									  const std::string &phrasePassword, const std::string &payPassword) {
			_account = AccountPtr(new StandardAccount(_rootPath, phrase, phrasePassword, payPassword));
		}

		void MasterWalletStore::Reset(const nlohmann::json &coSigners, uint32_t requiredSignCount) {
			_account = AccountPtr(new MultiSignAccount(nullptr, coSigners, requiredSignCount));
		}

		void MasterWalletStore::Reset(const std::string &privKey, const nlohmann::json &coSigners,
									  const std::string &payPassword, uint32_t requiredSignCount) {
			_account = AccountPtr(
					new MultiSignAccount(new SimpleAccount(privKey, payPassword), coSigners, requiredSignCount));
		}

		void MasterWalletStore::Reset(const std::string &phrase,
									  const std::string &phrasePassword, const nlohmann::json &coSigners,
									  const std::string &payPassword, uint32_t requiredSignCount) {
			_account = AccountPtr(new MultiSignAccount(
					new StandardAccount(_rootPath, phrase, phrasePassword, payPassword),
					coSigners, requiredSignCount));
		}

		bool MasterWalletStore::IsSingleAddress() const {
			return _isSingleAddress;
		}

		bool &MasterWalletStore::IsSingleAddress() {
			return _isSingleAddress;
		}
	}
}