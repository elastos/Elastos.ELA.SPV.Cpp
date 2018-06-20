// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <sstream>

#include "MerkleBlockDataSource.h"

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockDataSource::MerkleBlockDataSource(Sqlite *sqlite) :
			TableBase(sqlite) {
			initializeTable(MB_DATABASE_CREATE);
		}

		MerkleBlockDataSource::MerkleBlockDataSource(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			initializeTable(MB_DATABASE_CREATE);
		}

		MerkleBlockDataSource::~MerkleBlockDataSource() {
		}

		bool MerkleBlockDataSource::putMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return doTransaction([&iso, &blockEntity, this](){
				this->putMerkleBlockInternal(iso, blockEntity);
			});
		}

		bool MerkleBlockDataSource::putMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockEntity> &blockEntities) {
			return doTransaction([&iso, &blockEntities, this](){
				for (size_t i = 0; i < blockEntities.size(); ++i) {
					this->putMerkleBlockInternal(iso, blockEntities[i]);
				}
			});
		}

		bool MerkleBlockDataSource::putMerkleBlockInternal(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			std::stringstream ss;

			ss << "INSERT INTO " << MB_TABLE_NAME << " (" <<
			   MB_BUFF   << "," <<
			   MB_HEIGHT << "," <<
			   MB_ISO    <<
			   ") VALUES (?, ?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
				std::stringstream ess;
				ess << "prepare sql " << ss.str() << " fail";
				throw std::logic_error(ess.str());
			}

			_sqlite->bindBlob(stmt, 1, blockEntity.blockBytes, nullptr);
			_sqlite->bindInt(stmt, 2, blockEntity.blockHeight);
			_sqlite->bindText(stmt, 3, iso, nullptr);

			_sqlite->step(stmt);
			_sqlite->finalize(stmt);
			return true;
		}

		bool MerkleBlockDataSource::deleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return doTransaction([&iso, &blockEntity, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << MB_TABLE_NAME <<
				   " WHERE " << MB_COLUMN_ID << " = " << blockEntity.id <<
				   " AND " << MB_ISO << " = " << "'" << iso << "';";

				if (!_sqlite->exec(ss.str(), nullptr, nullptr)) {
					std::stringstream ess;
					ess << "exec sql " << ss.str() << "fail";
					throw std::logic_error(ess.str());
				}
			});
		}

		bool MerkleBlockDataSource::deleteAllBlocks(const std::string &iso) {
			return doTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << MB_TABLE_NAME <<
				   " WHERE " << MB_ISO << " = '" << iso << "';";

				if (!_sqlite->exec(ss.str(), nullptr, nullptr)) {
					std::stringstream ess;
					ess << "exec sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}
			});
		}

		std::vector<MerkleBlockEntity> MerkleBlockDataSource::getAllMerkleBlocks(const std::string &iso) const {
			std::vector<MerkleBlockEntity> merkleBlocks;

			doTransaction([&iso, &merkleBlocks, this]() {
				MerkleBlockEntity merkleBlock;
				std::stringstream ss;
				ss << "SELECT "  <<
				   MB_COLUMN_ID << ", " <<
				   MB_BUFF      << ", " <<
				   MB_HEIGHT    <<
				   " FROM "     << MB_TABLE_NAME <<
				   " WHERE "    << MB_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
					std::stringstream ess;
					ess << "prepare sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}

				while (SQLITE_ROW == _sqlite->step(stmt)) {
					CMBlock blockBytes;
					// id
					merkleBlock.id = _sqlite->columnInt(stmt, 0);

					// blockBytes
					const uint8_t *pblob = (const uint8_t *)_sqlite->columnBlob(stmt, 1);
					size_t len = _sqlite->columnBytes(stmt, 1);

					blockBytes.Resize(len);
					memcpy(blockBytes, pblob, len);

					merkleBlock.blockBytes = blockBytes;

					// blockHeight
					merkleBlock.blockHeight = _sqlite->columnInt(stmt, 2);

					merkleBlocks.push_back(merkleBlock);
				}

				_sqlite->finalize(stmt);
			});

			return merkleBlocks;
		}

	}
}

