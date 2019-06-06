// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_NEP5LOGDATASTORE_H__
#define __ELASTOS_SDK_NEP5LOGDATASTORE_H__

#include <SDK/Common/BigInt.h>

#include "Sqlite.h"
#include "TableBase.h"

namespace Elastos {
	namespace ElaWallet {

		struct Nep5LogEntity {
			Nep5LogEntity() {}

			Nep5LogEntity(const std::string &addr, const std::string &from, const std::string &to, const BigInt &amount,
			              const std::string &tid) : nep5Hash(addr), fromAddr(from), toAddr(to), value(amount),
			                                        txid(tid) {
			}

			std::string nep5Hash;
			std::string fromAddr;
			std::string toAddr;
			BigInt value;
			std::string txid;
		};

		class Nep5LogDataStore : public TableBase {
		public:
			Nep5LogDataStore(Sqlite *sqlite);

			~Nep5LogDataStore();

			bool PutNep5Log(const std::string &iso, const Nep5LogEntity &nep5LogEntity);

			bool DeleteAllNep5Logs(const std::string &iso);

			bool DeleteLogByTxID(const std::string &iso, const std::string &hash);

			bool GetNep5LogByTxID(const std::string &iso, const std::string &hash, Nep5LogEntity &logEntity) const;

			bool UpdateNep5Log(const std::string &iso, const Nep5LogEntity &nep5LogEntity);

			std::vector<Nep5LogEntity> GetAllLogs() const;

		private:
			bool SelectLogByHash(const std::string &iso, const std::string &hash, Nep5LogEntity &nep5LogEntity) const;

			bool UpdateNep5LogInternal(const std::string &iso, const Nep5LogEntity &nep5LogEntity);
		private:
			/*
			 * nep5 log table
			 */
			const std::string LOG_TABLE_NAME = "nep5LogTable";
			const std::string TXID = "txid";
			const std::string NEP5_HASH = "nep5hash";
			const std::string FROM_ADDR = "fromAddress";
			const std::string TO_ADDR = "toAddress";
			const std::string AMOUNT = "amount";
			const std::string NEP5LOG_ISO = "nep5LogISO";

			const std::string NEP5LOG_DATABASE_CREATE = "create table if not exists " + LOG_TABLE_NAME + " (" +
			                                            TXID + " text not null, " +
			                                            NEP5_HASH + " text not null, " +
			                                            FROM_ADDR + " text, " +
			                                            TO_ADDR + " text not null, " +
			                                            AMOUNT + " text not null, " +
			                                            NEP5LOG_ISO + " text DEFAULT 'ELA');";
		};
	}
}

#endif //__ELASTOS_SDK_NEP5LOGDATASTORE_H__

//MB_ISO + " text DEFAULT 'ELA');";