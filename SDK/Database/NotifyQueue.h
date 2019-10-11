// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_NOTIFYQUEUE_H__
#define __ELASTOS_SDK_NOTIFYQUEUE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <SDK/Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		class NotifyQueue : public TableBase {
		public:
			class Record {
			public:
				Record(const uint256 &hash, uint32_t height,
					   time_t last_notify_time = time_t(0))
					: tx_hash(hash), height(height), last_notify_time(last_notify_time) {}

			public:
				uint256 tx_hash;
				uint32_t height;
				time_t last_notify_time;
			};

			typedef boost::shared_ptr<Record> RecordPtr;
			typedef std::vector<RecordPtr> Records;

			NotifyQueue(const boost::filesystem::path &path);

			~NotifyQueue();

			/**
			 * Insert or Update a new transaction which may notify later.
			 * @param row, the transaction data to insert or update.
			 */
			bool Upsert(const RecordPtr &record);

			/**
			 * Get all transactions which have 6 or more confirmations.
			 * @param current, the current block height.
			 */
			Records GetAllConfirmed(uint32_t current);

			/**
			 * Delete the transaction data according to the given hash.
			 * @param tx_hash, the transaction hash specify which transaction to delete.
			 */
			bool Delete(const uint256 &tx_hash);

		private:
			// NotifyQueue table
			const std::string NOTIFY_QUEUE_TABLE = "NOTIFY_QUEUE";
			const std::string NOTIFY_QUEUE_COLUMN_TX_HASH = "TX_HASH";
			const std::string NOTIFY_QUEUE_COLUMN_HEIGHT = "HEIGHT";
			const std::string NOTIFY_QUEUE_COLUMN_LAST_NOTIFY_TIME = "LAST_NOTIFY";

			const std::string NOTIFY_QUEUE_TABLE_CREATE = "CREATE TABLE IF NOT EXISTS " +
														  NOTIFY_QUEUE_TABLE + "(" +
														  NOTIFY_QUEUE_COLUMN_TX_HASH + " TEXT PRIMARY KEY, " +
														  NOTIFY_QUEUE_COLUMN_HEIGHT + " INTEGER, " +
														  NOTIFY_QUEUE_COLUMN_LAST_NOTIFY_TIME + " INTEGER);";
		};

	} // namespace ElaWallet
} // namespace Elastos

#endif // __ELASTOS_SDK_NOTIFYQUEUE_H__
