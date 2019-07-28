// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERDATASOURCE_H__
#define __ELASTOS_SDK_PEERDATASOURCE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <SDK/Common/uint256.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace Elastos {
	namespace ElaWallet {

		struct PeerEntity {
			PeerEntity() :
				id(0),
				port(0),
				timeStamp(0)
			{
			}

			PeerEntity(long i, const uint128 &addr, uint16_t p, uint64_t ts) :
				id(i),
				address(addr),
				port(p),
				timeStamp(ts)
			{
			}

			long id;
			uint128 address;
			uint16_t port;
			uint64_t timeStamp;
		};

		class PeerDataSource : public TableBase {

		public:
			PeerDataSource(Sqlite *sqlite);
			PeerDataSource(SqliteTransactionType type, Sqlite *sqlite);
			~PeerDataSource();

			bool PutPeer(const std::string &iso, const PeerEntity &peerEntity);
			bool PutPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities);
			bool DeletePeer(const std::string &iso, const PeerEntity &peerEntity);
			bool DeleteAllPeers();
			size_t GetAllPeersCount(const std::string &iso) const;
			std::vector<PeerEntity> GetAllPeers(const std::string &iso) const;

		private:
			bool PutPeerInternal(const std::string &iso, const PeerEntity &peerEntity);

		private:
			/*
			 * peer table
			 */
			const std::string PEER_TABLE_NAME = "peerTable";
			const std::string PEER_COLUMN_ID = "_id";
			const std::string PEER_ADDRESS = "peerAddress";
			const std::string PEER_PORT = "peerPort";
			const std::string PEER_TIMESTAMP = "peerTimestamp";
			const std::string PEER_ISO = "peerISO";

			const std::string PEER_DATABASE_CREATE = "create table if not exists " + PEER_TABLE_NAME + " (" +
				PEER_COLUMN_ID + " integer primary key autoincrement, " +
				PEER_ADDRESS + " blob," +
				PEER_PORT + " integer," +
				PEER_TIMESTAMP + " integer," +
				PEER_ISO + " text default 'ELA');";
		};

	}
}

#endif //__ELASTOS_SDK_PEERDATASOURCE_H__

