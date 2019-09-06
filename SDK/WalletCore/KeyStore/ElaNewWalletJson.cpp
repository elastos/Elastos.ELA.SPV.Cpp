// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BIPs/HDKeychain.h>
#include <BIPs/BIP39.h>
#include <BIPs/Base58.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Log.h>
#include "ElaNewWalletJson.h"
#include "CoinInfo.h"

namespace Elastos {
	namespace ElaWallet {

		ElaNewWalletJson::ElaNewWalletJson() :
			_singleAddress(false) {

		}

		ElaNewWalletJson::~ElaNewWalletJson() {
		}

		void ElaNewWalletJson::AddCoinInfo(const CoinInfoPtr &info) {
			_coinInfoList.push_back(info);
		}

		void ElaNewWalletJson::ClearCoinInfo() {
			_coinInfoList.clear();
		}

		const std::vector<CoinInfoPtr> &ElaNewWalletJson::GetCoinInfoList() const {
			return _coinInfoList;
		}

		void ElaNewWalletJson::SetCoinInfoList(const std::vector<CoinInfoPtr> &list) {
			_coinInfoList = list;
		}

		bool ElaNewWalletJson::SingleAddress() const {
			return _singleAddress;
		}

		void ElaNewWalletJson::SetSingleAddress(bool value) {
			_singleAddress = value;
		}

		const std::string &ElaNewWalletJson::OwnerPubKey() const {
			return _ownerPubKey;
		}

		void ElaNewWalletJson::SetOwnerPubKey(const std::string &pubkey) {
			_ownerPubKey = pubkey;
		}

		const std::string &ElaNewWalletJson::xPubKeyHDPM() const {
			return _xPubKeyHDPM;
		}

		void ElaNewWalletJson::SetxPubKeyHDPM(const std::string &xpub) {
			_xPubKeyHDPM = xpub;
		}

		nlohmann::json ElaNewWalletJson::ToJson(bool withPrivKey) const {
			nlohmann::json j = ElaWebWalletJson::ToJson(withPrivKey);
			ToJsonCommon(j);
			return j;
		}

		void ElaNewWalletJson::FromJson(const nlohmann::json &j) {
			ElaWebWalletJson::FromJson(j);
			FromJsonCommon(j);
		}

		void to_json(nlohmann::json &j, const ElaNewWalletJson &p) {
			to_json(j, dynamic_cast<const ElaWebWalletJson &>(p));
			p.ToJsonCommon(j);
		}

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p) {
			from_json(j, dynamic_cast<ElaWebWalletJson &>(p));
			p.FromJsonCommon(j);
		}

		void ElaNewWalletJson::ToJsonCommon(nlohmann::json &j) const {
			nlohmann::json coinInfoList;
			for (size_t i = 0; i < _coinInfoList.size(); ++i)
				coinInfoList.push_back(_coinInfoList[i]->ToJson());

			j["CoinInfoList"] = coinInfoList;
			j["SingleAddress"] = _singleAddress;
			j["OwnerPubKey"] = _ownerPubKey;
			j["xPubKeyHDPM"] = _xPubKeyHDPM;
		}

		void ElaNewWalletJson::FromJsonCommon(const nlohmann::json &j) {
			if (j.find("CoinInfoList") != j.end()) {
				_coinInfoList.clear();
				nlohmann::json coinInfoList = j["CoinInfoList"];
				for (nlohmann::json::iterator it = coinInfoList.begin(); it != coinInfoList.end(); ++it) {
					CoinInfoPtr coinInfo(new CoinInfo());
					coinInfo->FromJson((*it));
					_coinInfoList.push_back(coinInfo);
				}
			}

			if (j.find("SingleAddress") != j.end()) {
				_singleAddress = j["SingleAddress"].get<bool>();
			}

			if (j.find("OwnerPubKey") != j.end()) {
				_ownerPubKey = j["OwnerPubKey"].get<std::string>();
			}

			if (j.find("xPubKeyHDPM") != j.end()) {
				_xPubKeyHDPM = j["xPubKeyHDPM"].get<std::string>();
			}

			if (j.find("CoSigners") != j.end() && j["Type"] == "MultiSign") {
				ErrorChecker::ThrowParamException(Error::KeyStore, "Unsupport old version multi-sign keystore");
			}

			if (j.find("RequiredSignCount") != j.end()) {
				ErrorChecker::ThrowParamException(Error::KeyStore, "Unsupport old version multi-sign keystore");
			}

			std::string passphrase;
			if (j.find("PhrasePassword") != j.end()) {
				passphrase = j["PhrasePassword"].get<std::string>();
				if (!passphrase.empty())
					_mnemonicHasPassphrase = true;
			}

			if (j.find("IsSingleAddress") != j.end()) {
				_singleAddress = j["IsSingleAddress"];
			}

			if (_xPrivKey.empty() && !_mnemonic.empty()) {
				Log::info("Regerate xprv from old keystore");
				HDSeed seed(BIP39::DeriveSeed(_mnemonic, passphrase).bytes());
				HDKeychain rootkey(seed.getExtendedKey(true));

				_ownerPubKey = rootkey.getChild("44'/0'/1'/0/0").pubkey().getHex();

				_xPrivKey = Base58::CheckEncode(rootkey.extkey());
				_xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());

				HDKeychain requestKey = rootkey.getChild("1'/0");
				_requestPrivKey = requestKey.privkey().getHex();
				_requestPubKey = requestKey.pubkey().getHex();

				_publicKeyRing.emplace_back(_requestPubKey, _xPubKeyHDPM);
				if (_m == 0)
					_m = 1;
				_n = _publicKeyRing.size();
				ErrorChecker::CheckParam(_n != 1, Error::KeyStore, "Import keystore should be n == 1");
			}

			if ((_xPubKeyHDPM.empty() || _ownerPubKey.empty()) && !_xPrivKey.empty()) {
				bytes_t bytes;
				Base58::CheckDecode(_xPrivKey, bytes);
				HDKeychain rootkey(bytes);

				if (_ownerPubKey.empty())
					_ownerPubKey = rootkey.getChild("44'/0'/1'/0/0").pubkey().getHex();

				if (_xPubKeyHDPM.empty())
					_xPubKeyHDPM = Base58::CheckEncode(rootkey.getChild("45'").getPublic().extkey());
			}
		}

	}
}
