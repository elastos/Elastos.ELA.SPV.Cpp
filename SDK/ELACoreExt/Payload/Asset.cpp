// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>

#include "Asset.h"
#include "BRInt.h"

namespace Elastos {
	namespace ElaWallet {
		Asset::Asset() :
				_name(""),
				_description(""),
				_precision(0),
				_assetType(AssetType::Share),
				_assetRecordType(AssetRecordType::Unspent) {

		}

		Asset::~Asset() {

		}

		void Asset::setName(const std::string &name) {
			_name = name;
		}

		std::string Asset::getName() const {
			return _name;
		}

		void Asset::setDescription(const std::string &desc) {
			_description = desc;
		}

		std::string Asset::getDescription() const {
			return _description;
		}

		void Asset::setAssetType(const Asset::AssetType type) {
			_assetType = type;
		}

		Asset::AssetType Asset::getAssetType() const {
			return _assetType;
		}

		void Asset::setAssetRecordType(const Asset::AssetRecordType type) {
			_assetRecordType = type;
		}

		Asset::AssetRecordType Asset::getAssetRecordType() const {
			return _assetRecordType;
		}

		void Asset::setPrecision(const uint8_t precision) {
			_precision = precision;
		}

		uint8_t Asset::getPrecision() const {
			return _precision;
		}

		void Asset::Serialize(ByteStream &ostream) const {
			uint64_t len = _name.length();
			ostream.putVarUint(len);
			ostream.putBytes((uint8_t *) _name.c_str(), len);

			len = _description.length();
			ostream.putVarUint(len);
			ostream.putBytes((uint8_t *) _description.c_str(), len);

			ostream.put(_precision);

			ostream.put((uint8_t)_assetType);

			ostream.put((uint8_t)_assetRecordType);
		}

		bool Asset::Deserialize(ByteStream &istream) {
			uint64_t len = istream.getVarUint();
			char nameBuf[len + 1];
			istream.getBytes((uint8_t *) nameBuf, len);
			nameBuf[len] = '\0';
			_name = std::string(nameBuf);

			len = istream.getVarUint();
			char description[len + 1];
			istream.getBytes((uint8_t *) description, len);
			description[len] = '\0';
			_description = std::string(description);

			_precision = istream.get();

			_assetType = AssetType(istream.get());

			_assetRecordType = AssetRecordType(istream.get());

			return true;
		}

		nlohmann::json Asset::toJson() {
			nlohmann::json jsonData;

			jsonData["name"] = _name;

			jsonData["description"] = _description;

			jsonData["precision"] = _precision;

			jsonData["assetType"] = _assetType;

			jsonData["assetRecordType"] = _assetRecordType;

			return jsonData;
		}

		void Asset::fromJson(const nlohmann::json &jsonData) {
			_name = jsonData["name"].get<std::string>();

			_description = jsonData["description"].get<std::string>();

			_precision = jsonData["precision"].get<uint8_t>();

			_assetType = jsonData["assetType"].get<AssetType>();

			_assetRecordType = jsonData["assetRecordType"].get<AssetRecordType>();
		}
	}
}