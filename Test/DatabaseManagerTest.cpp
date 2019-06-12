// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <SDK/Database/TransactionDataStore.h>
#include <SDK/Database/DatabaseManager.h>
#include <SDK/SpvService/BackgroundExecutor.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

#include <fstream>
using namespace Elastos::ElaWallet;

#define ISO "els"
#define DBFILE "wallet.db"

TEST_CASE("DatabaseManager test", "[DatabaseManager]") {
	Log::registerMultiLogger();

	SECTION("Prepare to test") {
		srand(time(nullptr));

		if (boost::filesystem::exists(DBFILE) && boost::filesystem::is_regular_file(DBFILE)) {
			boost::filesystem::remove(DBFILE);
		}

		REQUIRE(!boost::filesystem::exists(DBFILE));
	}

	SECTION("Asset test") {
		DatabaseManager dm(DBFILE);

		// save
		std::vector<AssetEntity> assets;
		for (int i = 0; i < 20; ++i) {
			AssetEntity asset;
			asset.Asset = getRandBytes(100);
			asset.AssetID = getRandString(64);
			asset.Amount = rand();
			assets.push_back(asset);
			REQUIRE(dm.PutAsset(ISO, asset));
		}

		// verify save
		std::vector<AssetEntity> assetsVerify = dm.GetAllAssets();
		REQUIRE(assetsVerify.size() > 0);
		REQUIRE(assetsVerify.size() == assets.size());
		for (size_t i = 0; i < assets.size(); ++i) {
			REQUIRE(assets[i].Asset == assetsVerify[i].Asset);
			REQUIRE(assets[i].AssetID == assetsVerify[i].AssetID);
			REQUIRE(assets[i].Amount == assetsVerify[i].Amount);
		}

		// delete random one
		int idx = rand() % assetsVerify.size();
		REQUIRE(dm.DeleteAsset(assets[idx].AssetID));

		// verify deleted
		AssetEntity assetGot;
		REQUIRE(!dm.GetAssetDetails(assets[idx].AssetID, assetGot));

		// verify count after delete
		assetsVerify = dm.GetAllAssets();
		REQUIRE(assetsVerify.size() == assets.size() - 1);

		// update already exist assetID
		idx = rand() % assetsVerify.size();
		AssetEntity assetsUpdate;
		assetsVerify[idx].Amount = rand();
		assetsVerify[idx].Asset = getRandBytes(200);
		assetsUpdate = assetsVerify[idx];
		REQUIRE(dm.PutAsset("Test", assetsUpdate));

		REQUIRE(dm.GetAssetDetails(assetsVerify[idx].AssetID, assetGot));
		REQUIRE(assetsVerify[idx].Amount == assetGot.Amount);
		REQUIRE(assetsVerify[idx].Asset == assetGot.Asset);

		// delete all
		REQUIRE(dm.DeleteAllAssets());
		assetsVerify = dm.GetAllAssets();
		REQUIRE(assetsVerify.size() == 0);
	}

	SECTION("Merkle Block test ") {
#define TEST_MERKLEBLOCK_RECORD_CNT uint64_t(20)

		static std::vector<MerkleBlockEntity> blocksToSave;

		SECTION("Merkle Block prepare for testing") {
			for (uint64_t i = 0; i < TEST_MERKLEBLOCK_RECORD_CNT; ++i) {
				MerkleBlockEntity block;

				block.blockBytes = getRandBytes(40);
				block.blockHeight = static_cast<uint32_t>(i);

				blocksToSave.push_back(block);
			}
		}

		SECTION("Merkle Block save test") {
			DatabaseManager *dbm = new DatabaseManager(DBFILE);
			REQUIRE(dbm->PutMerkleBlocks(ISO, blocksToSave));
			delete dbm;
		}

		SECTION("Merkle Block read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<MerkleBlockEntity> blocksRead = dbm.GetAllMerkleBlocks(ISO);
			REQUIRE(blocksRead.size() == blocksToSave.size());
			for (int i = 0; i < blocksRead.size(); ++i) {
				REQUIRE(blocksToSave[i].blockBytes == blocksRead[i].blockBytes);
				REQUIRE(blocksRead[i].blockHeight == blocksToSave[i].blockHeight);
			}
		}

		SECTION("Merkle Block delete test") {
			DatabaseManager dbm(DBFILE);

			REQUIRE(dbm.DeleteAllBlocks(ISO));

			std::vector<MerkleBlockEntity> blocksAfterDelete = dbm.GetAllMerkleBlocks(ISO);
			REQUIRE(0 == blocksAfterDelete.size());
		}

		SECTION("Merkle Block save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < blocksToSave.size(); ++i) {
				REQUIRE(dbm.PutMerkleBlock(ISO, blocksToSave[i]));
			}
		}

		SECTION("Merkle Block read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<MerkleBlockEntity> blocksRead = dbm.GetAllMerkleBlocks(ISO);
			REQUIRE(blocksRead.size() == blocksToSave.size());
			for (int i = 0; i < blocksRead.size(); ++i) {
				REQUIRE(blocksRead[i].blockHeight == blocksToSave[i].blockHeight);
				REQUIRE(blocksToSave[i].blockBytes == blocksRead[i].blockBytes);
			}
		}

		SECTION("Merkle Block delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<MerkleBlockEntity> blocksBeforeDelete = dbm.GetAllMerkleBlocks(ISO);

			for (int i = 0; i < blocksBeforeDelete.size(); ++i) {
				REQUIRE(dbm.DeleteMerkleBlock(ISO, blocksBeforeDelete[i]));
			}

			std::vector<MerkleBlockEntity> blocksAfterDelete = dbm.GetAllMerkleBlocks(ISO);
			REQUIRE(0 == blocksAfterDelete.size());
		}
	}

#define TEST_PEER_RECORD_CNT 20
	SECTION("Peer test") {

		static std::vector<PeerEntity> peerToSave;

		SECTION("Peer Prepare for test") {
			for (int i = 0; i < TEST_PEER_RECORD_CNT; i++) {
				PeerEntity peer;
				peer.address = getRandUInt128();
				peer.port = (uint16_t)rand();
				peer.timeStamp = (uint64_t)rand();
				peerToSave.push_back(peer);
			}

			REQUIRE(TEST_PEER_RECORD_CNT == peerToSave.size());
		}

		SECTION("Peer save test") {
			DatabaseManager dbm(DBFILE);
			REQUIRE(dbm.PutPeers(ISO, peerToSave));
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.GetAllPeers(ISO);
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(peers[i].address == peerToSave[i].address);
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete test") {
			DatabaseManager *dbm = new DatabaseManager(DBFILE);
			REQUIRE(dbm->DeleteAllPeers(ISO));
			std::vector<PeerEntity> peers = dbm->GetAllPeers(ISO);
			REQUIRE(peers.size() == 0);
			delete dbm;
		}

		SECTION("Peer save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < peerToSave.size(); ++i) {
				REQUIRE(dbm.PutPeer(ISO, peerToSave[i]));
			}
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.GetAllPeers(ISO);
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(peers[i].address == peerToSave[i].address);
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<PeerEntity> PeersBeforeDelete = dbm.GetAllPeers(ISO);
			REQUIRE(PeersBeforeDelete.size() == peerToSave.size());

			for (int i = 0; i < PeersBeforeDelete.size(); ++i) {
				REQUIRE(dbm.DeletePeer(ISO, PeersBeforeDelete[i]));
			}

			std::vector<PeerEntity> PeersAfterDelete = dbm.GetAllPeers(ISO);
			REQUIRE(0 == PeersAfterDelete.size());
		}

	}

#define TEST_TX_RECORD_CNT 20
	SECTION("Transaction test") {
		static std::vector<TransactionEntity> txToSave;
		static std::vector<TransactionEntity> txToUpdate;

		SECTION("Transaction prepare for testing") {
			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionEntity tx;

				tx.buff = getRandBytes(35);
				tx.blockHeight = (uint32_t)rand();
				tx.timeStamp = (uint32_t)rand();
				tx.txHash = getRandString(25);
				txToSave.push_back(tx);
			}

			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionEntity tx;

				tx.buff = getRandBytes(49);
				tx.blockHeight = (uint32_t)rand();
				tx.timeStamp = (uint32_t)rand();
				tx.txHash = txToSave[i].txHash;
				txToUpdate.push_back(tx);
			}
		}

		SECTION("Transaction save test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < txToSave.size(); ++i) {
				REQUIRE(dbm.PutTransaction(ISO, txToSave[i]));
			}
		}

		SECTION("Transaction read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<TransactionEntity> readTx = dbm.GetAllTransactions(ISO);
			REQUIRE(txToSave.size() == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				REQUIRE(txToSave[i].buff == readTx[i].buff);
				REQUIRE(readTx[i].txHash == txToSave[i].txHash);
				REQUIRE(readTx[i].timeStamp == txToSave[i].timeStamp);
				REQUIRE(readTx[i].blockHeight == txToSave[i].blockHeight);
			}
		}

		SECTION("Transaction udpate test") {
			DatabaseManager dbm(DBFILE);

			for (int i = 0; i < txToUpdate.size(); ++i) {
				REQUIRE(dbm.UpdateTransaction(ISO, txToUpdate[i]));
			}
		}

		SECTION("Transaction read after update test") {
			DatabaseManager dbm(DBFILE);
			std::vector<TransactionEntity> readTx = dbm.GetAllTransactions(ISO);
			REQUIRE(TEST_TX_RECORD_CNT == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				REQUIRE(txToSave[i].buff == readTx[i].buff);
				REQUIRE(readTx[i].txHash == txToUpdate[i].txHash);
				REQUIRE(readTx[i].timeStamp == txToUpdate[i].timeStamp);
				REQUIRE(readTx[i].blockHeight == txToUpdate[i].blockHeight);
			}
		}

		SECTION("Transaction delete by txHash test") {
			DatabaseManager dbm(DBFILE);

			for (int i = 0; i < txToUpdate.size(); ++i) {
				REQUIRE(dbm.DeleteTxByHash(ISO, txToUpdate[i].txHash));
			}

			std::vector<TransactionEntity> readTx = dbm.GetAllTransactions(ISO);
			REQUIRE(0 == readTx.size());
		}

	}

#define TEST_NEP5LOG_CNT 20
	SECTION("Nep5Log test") {
		static std::vector<Nep5LogEntity> neo5LogToSave;
		static std::vector<Nep5LogEntity> neo5LogToUpdate;
		SECTION("Nep5Log Prepare for test") {
			for (int i = 0; i < TEST_NEP5LOG_CNT; i++) {
				Nep5LogEntity nep5LogEntity;
				nep5LogEntity.txid = getRandBytes(32).getHex();
				nep5LogEntity.nep5Hash = getRandBytes(32).getHex();
				nep5LogEntity.fromAddr = getRandBytes(20).getHex();
				nep5LogEntity.toAddr = getRandBytes(20).getHex();
				nep5LogEntity.value = getRandUInt64();

				neo5LogToSave.push_back(nep5LogEntity);
			}
			REQUIRE(TEST_NEP5LOG_CNT == neo5LogToSave.size());

			for (uint64_t i = 0; i < TEST_NEP5LOG_CNT; ++i) {
				Nep5LogEntity nep5LogEntity;

				nep5LogEntity.txid = neo5LogToSave[i].txid;
				nep5LogEntity.nep5Hash = getRandBytes(32).getHex();
				nep5LogEntity.fromAddr = getRandBytes(20).getHex();
				nep5LogEntity.toAddr = getRandBytes(20).getHex();
				nep5LogEntity.value = getRandUInt64();
				neo5LogToUpdate.push_back(nep5LogEntity);
			}
		}

		SECTION("Nep5Log save test") {
			DatabaseManager dbm(DBFILE);
			for (size_t i = 0; i < neo5LogToSave.size(); ++i) {
				bool result = dbm.PutNep5Log(ISO, neo5LogToSave[i]);
				REQUIRE(result == true);
			}
		}

		SECTION("Nep5Log read test") {
			DatabaseManager dbm(DBFILE);

			for (int i = 0; i < TEST_NEP5LOG_CNT; ++i) {
				Nep5LogEntity nep5LogEntity;
				bool result = dbm.GetNep5Log(ISO, neo5LogToSave[i].txid, nep5LogEntity);
				REQUIRE(result == true);
				REQUIRE(nep5LogEntity.txid == neo5LogToSave[i].txid);
				REQUIRE(nep5LogEntity.nep5Hash == neo5LogToSave[i].nep5Hash);
				REQUIRE(nep5LogEntity.fromAddr == neo5LogToSave[i].fromAddr);
				REQUIRE(nep5LogEntity.toAddr == neo5LogToSave[i].toAddr);
				REQUIRE(nep5LogEntity.value == neo5LogToSave[i].value);
			}
		}

		SECTION("Nep5Log udpate test") {
			DatabaseManager dbm(DBFILE);

			for (int i = 0; i < neo5LogToUpdate.size(); ++i) {
				REQUIRE(dbm.UpdateNep5Log(ISO, neo5LogToUpdate[i]));
			}
		}

		SECTION("Nep5Log read after update test") {
			DatabaseManager dbm(DBFILE);
			const std::vector<Nep5LogEntity> allLogs = dbm.GetAllLogs();
			REQUIRE(allLogs.size() == TEST_NEP5LOG_CNT);

			for (int i = 0; i < allLogs.size(); ++i) {
				REQUIRE(allLogs[i].txid == neo5LogToUpdate[i].txid);
				REQUIRE(allLogs[i].nep5Hash == neo5LogToUpdate[i].nep5Hash);
				REQUIRE(allLogs[i].fromAddr == neo5LogToUpdate[i].fromAddr);
				REQUIRE(allLogs[i].toAddr == neo5LogToUpdate[i].toAddr);
				REQUIRE(allLogs[i].value == neo5LogToUpdate[i].value);
			}
		}

		SECTION("Nep5Log delete one by one test") {
			DatabaseManager dbm(DBFILE);

			const std::vector<Nep5LogEntity> neo5LogToDeletes = dbm.GetAllLogs();
			REQUIRE(neo5LogToDeletes.size() == TEST_NEP5LOG_CNT);

			for (int i = 0; i < TEST_NEP5LOG_CNT; ++i) {
				REQUIRE(dbm.DeleteNep5Log(ISO, neo5LogToDeletes[i].txid));
			}

			std::vector<Nep5LogEntity> neo5LogDeleted = dbm.GetAllLogs();
			REQUIRE(0 == neo5LogDeleted.size());
		}
	}
}
