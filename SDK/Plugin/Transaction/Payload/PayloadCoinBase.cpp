// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadCoinBase.h"
#include <SDK/Common/Utils.h>

#include <Core/BRInt.h>
#include <cstring>

namespace Elastos {
	namespace ElaWallet {
		PayloadCoinBase::PayloadCoinBase() {

		}

		PayloadCoinBase::PayloadCoinBase(const CMBlock &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		PayloadCoinBase::~PayloadCoinBase() {
		}

		void PayloadCoinBase::SetCoinBaseData(const CMBlock &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		const CMBlock &PayloadCoinBase::GetCoinBaseData() const {
			return _coinBaseData;
		}

		void PayloadCoinBase::Serialize(ByteStream &ostream) const {
			ostream.writeVarBytes(_coinBaseData);
		}

		bool PayloadCoinBase::Deserialize(ByteStream &istream) {
			return istream.readVarBytes(_coinBaseData);
		}

		nlohmann::json PayloadCoinBase::toJson() const {
			nlohmann::json j;
			j["CoinBaseData"] = Utils::encodeHex(_coinBaseData);
			return j;
		}

		void PayloadCoinBase::fromJson(const nlohmann::json &j) {
			_coinBaseData = Utils::decodeHex(j["CoinBaseData"].get<std::string>());

		}
	}
}