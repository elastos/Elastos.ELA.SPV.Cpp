// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONDATASTORE_H__
#define __ELASTOS_SDK_TRANSACTIONDATASTORE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <utility>

namespace Elastos {
	namespace ElaWallet {

		struct TransactionEntity {
			TransactionEntity() :
				blockHeight(0),
				timeStamp(0)
			{
			}

			TransactionEntity(const bytes_t &buff, uint32_t blockHeight, uint32_t timeStamp,
							  const std::string &txHash) :
				buff(buff),
				blockHeight(blockHeight),
				timeStamp(timeStamp),
				txHash(txHash)
			{
			}

			bytes_t buff;
			uint32_t blockHeight;
			uint32_t timeStamp;
			std::string txHash;
		};

		class TransactionDataStore : public TableBase {
		public:
			TransactionDataStore(Sqlite *sqlite);
			TransactionDataStore(SqliteTransactionType type, Sqlite *sqlite);
			~TransactionDataStore();

			bool PutTransaction(const std::string &iso, const TransactionEntity &transactionEntity);
			bool DeleteAllTransactions(const std::string &iso);
			size_t GetAllTransactionsCount(const std::string &iso) const;
			std::vector<TransactionEntity> GetAllTransactions(const std::string &iso) const;
			bool UpdateTransaction(const std::string &iso, const TransactionEntity &transactionEntity);
			bool DeleteTxByHash(const std::string &iso, const std::string &hash);

		private:
			bool SelectTxByHash(const std::string &iso, const std::string &hash, TransactionEntity &txEntity) const;

		private:
			/*
			 * transaction table
			 */
			const std::string TX_TABLE_NAME = "transactionTable";
			const std::string TX_COLUMN_ID = "_id";
			const std::string TX_BUFF = "transactionBuff";
			const std::string TX_BLOCK_HEIGHT = "transactionBlockHeight";
			const std::string TX_TIME_STAMP = "transactionTimeStamp";
			const std::string TX_ISO = "transactionISO";
			const std::string TX_REMARK = "transactionRemark";
			const std::string TX_ASSETID = "assetID";

			const std::string TX_DATABASE_CREATE = "create table if not exists " + TX_TABLE_NAME + " (" +
				TX_COLUMN_ID + " text not null, " +
				TX_BUFF + " blob, " +
				TX_BLOCK_HEIGHT + " integer, " +
				TX_TIME_STAMP + " integer, " +
				TX_REMARK + " text DEFAULT '', " +
				TX_ASSETID + " text not null, " +
				TX_ISO + " text DEFAULT 'ELA' );";
		};

	}
}


#endif //SPVCLIENT_TRANSACTIONDATASTORE_H

