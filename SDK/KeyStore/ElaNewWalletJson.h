// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELANEWWALLETJSON_H__
#define __ELASTOS_SDK_ELANEWWALLETJSON_H__

#include "ElaWebWalletJson.h"
#include "CoinInfo.h"
#include "Mstream.h"


namespace Elastos {
	namespace SDK {

		class ElaNewWalletJson :
				public ElaWebWalletJson {
		public:
			ElaNewWalletJson();

			~ElaNewWalletJson();

			const std::string &getID() const;

			void setID(const std::string &id);

			const std::string &getIDInfo() const;

			void setIDInfo(const std::string &value);

			const std::string &getMnemonicLanguage() const;

			void setMnemonicLanguage(const std::string &language);

			void addCoinInfo(const CoinInfo &info);

			void clearCoinInfo();

			const std::vector<CoinInfo> &getCoinInfoList() const;

		private:
			JSON_SM_LS(ElaNewWalletJson);
			JSON_SM_RS(ElaNewWalletJson);
			TO_JSON(ElaNewWalletJson);
			FROM_JSON(ElaNewWalletJson);

		private:
			std::string _id;
			std::string _idInfo;
			std::string _mnemonicLanguage;
			std::vector<CoinInfo> _coinInfoList;
		};
	}
}

#endif //__ELASTOS_SDK_ELANEWWALLETJSON_H__
