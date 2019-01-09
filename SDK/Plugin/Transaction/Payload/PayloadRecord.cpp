// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRecord.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <Core/BRAddress.h>
#include <Core/BRInt.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadRecord::PayloadRecord() :
				_recordType("") {

		}

		PayloadRecord::PayloadRecord(const std::string &recordType, const CMBlock &recordData) {
			_recordType = recordType;
			_recordData = recordData;
		}

		PayloadRecord::PayloadRecord(const PayloadRecord &payload) {
			operator=(payload);
		}

		PayloadRecord::~PayloadRecord() {
		}

		void PayloadRecord::setRecordType(const std::string &recordType) {
			_recordType = recordType;
		}

		void PayloadRecord::setRecordData(const CMBlock &recordData) {
			_recordData = recordData;
		}

		std::string PayloadRecord::getRecordType() const {
			return _recordType;
		}

		CMBlock PayloadRecord::getRecordData() const {
			return _recordData;
		}

		void PayloadRecord::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.writeVarString(_recordType);
			ostream.writeVarBytes(_recordData);
		}

		bool PayloadRecord::Deserialize(ByteStream &istream, uint8_t version) {
			if (!istream.readVarString(_recordType)) {
				Log::error("Payload record deserialize type fail");
				return false;
			}

			if (!istream.readVarBytes(_recordData)) {
				Log::error("Payload record deserialize data fail");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRecord::toJson(uint8_t version) const {
			nlohmann::json j;

			j["RecordType"] = _recordType;
			j["RecordData"] = Utils::encodeHex(_recordData);

			return j;
		}

		void PayloadRecord::fromJson(const nlohmann::json &j, uint8_t version) {
			_recordType = j["RecordType"].get<std::string>();
			_recordData = Utils::decodeHex(j["RecordData"].get<std::string>());
		}

		IPayload &PayloadRecord::operator=(const IPayload &payload) {
			try {
				const PayloadRecord &payloadRecord = dynamic_cast<const PayloadRecord &>(payload);
				operator=(payloadRecord);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadRecord");
			}

			return *this;
		}

		PayloadRecord &PayloadRecord::operator=(const PayloadRecord &payload) {
			_recordData.Memcpy(payload._recordData);
			_recordType = payload._recordType;

			return *this;
		}

	}
}