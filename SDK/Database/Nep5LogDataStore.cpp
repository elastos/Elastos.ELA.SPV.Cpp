// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Nep5LogDataStore.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {
		Nep5LogDataStore::Nep5LogDataStore(Sqlite *sqlite) : TableBase(sqlite) {
			InitializeTable(NEP5LOG_DATABASE_CREATE);
		}

		Nep5LogDataStore::~Nep5LogDataStore() {
		}

		bool Nep5LogDataStore::PutNep5Log(const std::string &iso, const Nep5LogEntity &nep5LogEntity) {

			Nep5LogEntity LogEntity;
			if (SelectLogByHash(iso, nep5LogEntity.txid, LogEntity)) {
				return DoTransaction([&iso, &nep5LogEntity, this]() {
					UpdateNep5LogInternal(iso, nep5LogEntity);
				});
			}

			return DoTransaction([&iso, &nep5LogEntity, this]() {
				std::stringstream ss;

				ss << "INSERT INTO " << LOG_TABLE_NAME << "("
				   << TXID << ","
				   << NEP5_HASH << ","
				   << FROM_ADDR << ","
				   << TO_ADDR << ","
				   << AMOUNT << ","
				   << NEP5LOG_ISO
				   << ") VALUES (?, ?, ?, ?, ?, ?);";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
				                             "Prepare sql " + ss.str());

				_sqlite->BindText(stmt, 1, nep5LogEntity.txid, nullptr);
				_sqlite->BindText(stmt, 2, nep5LogEntity.nep5Hash, nullptr);
				_sqlite->BindText(stmt, 3, nep5LogEntity.fromAddr, nullptr);
				_sqlite->BindText(stmt, 4, nep5LogEntity.toAddr, nullptr);
				_sqlite->BindText(stmt, 5, nep5LogEntity.value.getDec(), nullptr);
				_sqlite->BindText(stmt, 6, iso, nullptr);

				_sqlite->Step(stmt);

				_sqlite->Finalize(stmt);
			});
		}

		bool Nep5LogDataStore::DeleteAllNep5Logs(const std::string &iso) {
			return DoTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << LOG_TABLE_NAME << ";";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
				                             "Exec sql " + ss.str());
			});
		}

		bool Nep5LogDataStore::DeleteLogByTxID(const std::string &iso, const std::string &hash) {
			return DoTransaction([&iso, &hash, this]() {
				std::stringstream ss;

				ss << "DELETE FROM "
				   << LOG_TABLE_NAME
				   << " WHERE " << TXID << " = '" << hash << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
				                             "Exec sql " + ss.str());
			});
		}

		bool Nep5LogDataStore::GetNep5LogByTxID(const std::string &iso, const std::string &hash,
		                                        Nep5LogEntity &logEntity) const {
			return SelectLogByHash(iso, hash, logEntity);
		}

		bool Nep5LogDataStore::UpdateNep5Log(const std::string &iso, const Nep5LogEntity &nep5LogEntity) {
			return DoTransaction([&iso, &nep5LogEntity, this]() {
				UpdateNep5LogInternal(iso, nep5LogEntity);
			});
		}

		bool Nep5LogDataStore::UpdateNep5LogInternal(const std::string &iso, const Nep5LogEntity &nep5LogEntity) {
			std::stringstream ss;

			ss << "UPDATE " << LOG_TABLE_NAME << " SET "
			   << NEP5_HASH << " = ?, "
			   << FROM_ADDR << " = ?, "
			   << TO_ADDR << ","
			   << AMOUNT
			   << " WHERE " << TXID << " = '" << nep5LogEntity.txid << "';";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
			                             "Prepare sql " + ss.str());

			_sqlite->BindText(stmt, 1, nep5LogEntity.nep5Hash, nullptr);
			_sqlite->BindText(stmt, 2, nep5LogEntity.fromAddr, nullptr);
			_sqlite->BindText(stmt, 3, nep5LogEntity.toAddr, nullptr);
			_sqlite->BindText(stmt, 4, nep5LogEntity.value.getDec(), nullptr);

			_sqlite->Step(stmt);

			_sqlite->Finalize(stmt);

			return true;
		}

		bool Nep5LogDataStore::SelectLogByHash(const std::string &iso, const std::string &hash,
		                                       Nep5LogEntity &nep5LogEntity) const {
			bool found = false;

			DoTransaction([&iso, &hash, &nep5LogEntity, &found, this]() {
				std::stringstream ss;

				ss << "SELECT "
				   << NEP5_HASH << ", "
				   << FROM_ADDR << ", "
				   << TO_ADDR << ", "
				   << AMOUNT
				   << " FROM " << LOG_TABLE_NAME
				   << " WHERE " << TXID << " = '" << hash << "';";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
				                             "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					found = true;

					nep5LogEntity.txid = hash;
					nep5LogEntity.nep5Hash = _sqlite->ColumnText(stmt, 0);
					nep5LogEntity.fromAddr = _sqlite->ColumnText(stmt, 1);
					nep5LogEntity.toAddr = _sqlite->ColumnText(stmt, 2);
					std::string value = _sqlite->ColumnText(stmt, 3);
					nep5LogEntity.value.setDec(value);
				}
			});

			return found;
		}

		std::vector<Nep5LogEntity> Nep5LogDataStore::GetAllLogs() const {
			std::vector<Nep5LogEntity> logEntitys;

			DoTransaction([&logEntitys, this]() {
				Nep5LogEntity logEntity;
				std::stringstream ss;

				ss << "SELECT "
				   << TXID << ", "
				   << NEP5_HASH << ", "
				   << FROM_ADDR << ", "
				   << TO_ADDR << ", "
				   << AMOUNT
				   << " FROM " << LOG_TABLE_NAME << ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
				                             "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					logEntity.txid = _sqlite->ColumnText(stmt, 0);
					logEntity.nep5Hash = _sqlite->ColumnText(stmt, 1);
					logEntity.fromAddr = _sqlite->ColumnText(stmt, 2);
					logEntity.toAddr = _sqlite->ColumnText(stmt, 3);
					std::string value = _sqlite->ColumnText(stmt, 4);
					logEntity.value.setDec(value);

					logEntitys.push_back(logEntity);
				}
			});
			return logEntitys;
		}
	}
}