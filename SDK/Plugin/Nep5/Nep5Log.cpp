// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Nep5Log.h"

#include <SDK/Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {
		Nep5Log::Nep5Log() : _nep5Hash(""), _from(""), _to(""), _txid("") {

		}

		Nep5Log::Nep5Log(const std::string &nep5Hash, const std::string &from, const std::string &to,
		                 const BigInt &data, const std::string &txid) :
				_nep5Hash(nep5Hash), _from(from), _to(to), _data(data), _txid(txid) {

		}

		Nep5Log::~Nep5Log() {

		}

		void Nep5Log::SetNep5Hash(const std::string &nep5Hash) {
			_nep5Hash = nep5Hash;
		}

		void Nep5Log::SetFrom(const std::string &from) {
			_from = from;
		}

		void Nep5Log::SetTo(const std::string &to) {
			_to = to;
		}

		void Nep5Log::SetData(const BigInt &value) {
			_data = value;
		}

		void Nep5Log::SetTxId(const std::string &txid) {
			_txid = txid;
		}

		const std::string &Nep5Log::GetNep5Hash() const {
			return _nep5Hash;
		}

		const std::string &Nep5Log::GetFrom() const {
			return _from;
		}

		const std::string &Nep5Log::GetTo() const {
			return _to;
		}

		const BigInt &Nep5Log::GetData() const {
			return _data;
		}

		const std::string &Nep5Log::GetTxID() const {
			return _txid;
		}

		void Nep5Log::Serialize(ByteStream &ostream) const {

			ostream.WriteVarString(_nep5Hash);

			ostream.WriteVarString(_from);

			ostream.WriteVarString(_to);

			ostream.WriteVarString(_data.getHex());

			ostream.WriteVarString(_txid);

		}

		bool Nep5Log::Deserialize(const ByteStream &istream) {

			std::string nepHash;
			istream.ReadVarString(nepHash);
			_nep5Hash = nepHash;

			uint160 from;
			istream.ReadBytes(from);
			std::reverse(from.begin(), from.end());
			_from = from.GetHex();

			uint160 to;
			istream.ReadBytes(to);
			std::reverse(to.begin(), to.end());
			_to = to.GetHex();

			bytes_t data;
			istream.ReadVarBytes(data);
			_data.setBytes(data);

			std::string txid;
			istream.ReadVarString(txid);
			_txid = txid;

			return true;
		}

		nlohmann::json Nep5Log::ToJson() const {
			nlohmann::json jsonData;

			jsonData["Nep5Hash"] = _nep5Hash;
			jsonData["From"] = _from;
			jsonData["To"] = _to;
			jsonData["Data"] = _data.getWord();
			jsonData["TxId"] = _txid;

			return jsonData;
		}

		void Nep5Log::FromJson(const nlohmann::json &jsonData) {
			try {

				_nep5Hash = jsonData["Nep5Hash"].get<std::string>();
				_from = jsonData["From"].get<std::string>();
				_to = jsonData["To"].get<std::string>();
				uint64_t value = jsonData["Data"].get<uint64_t>();
				_data.setWord(value);
				_txid = jsonData["TxId"].get<std::string>();

			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowLogicException(Error::Code::JsonFormatError, "Nep5Log from json: " +
				                                                                std::string(e.what()));
			}
		}
	}
}