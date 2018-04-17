// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <Database/TransactionDataStore.h>

#include "DatabaseManager.h"
#include "catch.hpp"

using namespace Elastos::SDK;

TEST_CASE("DatabaseManager Contructor test", "[DatabaseManager]") {

	SECTION("Contruct default test ") {
		DatabaseManager *databaseManager = new DatabaseManager();
		REQUIRE(databaseManager != nullptr);

		std::fstream _file;
		_file.open("wallet.db", std::ios::in);
		REQUIRE(!_file == false);
		_file.close();
	}

	SECTION("Construct with database path test") {
		std::fstream _file;
		const char *testDb = "testData.db";
		_file.open(testDb, std::ios::in);
		if (_file) {
			remove(testDb);
		}
		_file.close();

		DatabaseManager *databaseManager = new DatabaseManager(testDb);
		REQUIRE(databaseManager != nullptr);

		_file.open(testDb, std::ios::in);
		REQUIRE(!_file == false);
		_file.close();
	}
}

TEST_CASE("DatabaseManager database interface test", "[DatabaseManager]") {

	DatabaseManager databaseManager;
	std::string iso = "ELA";

	SECTION("DatabaseManager putTransaction test") {
		TransactionEntity transactionEntity;
		transactionEntity.buff = ByteData(nullptr, 0);
		transactionEntity.blockHeight = 9000;
		transactionEntity.timeStamp = 1523935043;
		transactionEntity.txHash = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";
		bool result = databaseManager.putTransaction(iso, transactionEntity);
		REQUIRE(result == true);

		std::vector<TransactionEntity> list = databaseManager.getAllTransactions(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);
		REQUIRE(list[len - 1].blockHeight == transactionEntity.blockHeight);
		REQUIRE(list[len - 1].timeStamp == transactionEntity.timeStamp);
		REQUIRE(list[len - 1].txHash == transactionEntity.txHash);
		REQUIRE(list[len - 1].buff.length == 0);
	}

	SECTION("DatabaseManager updateTransaction test") {
		TransactionEntity transactionEntity;
		transactionEntity.buff = ByteData(nullptr, 0);
		transactionEntity.blockHeight = 8000;
		transactionEntity.timeStamp = 1523935888;
		transactionEntity.txHash = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";

		bool result = databaseManager.updateTransaction(iso, transactionEntity);
		REQUIRE(result == true);

		std::vector<TransactionEntity> list = databaseManager.getAllTransactions(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);
		REQUIRE(list[len - 1].blockHeight == transactionEntity.blockHeight);
		REQUIRE(list[len - 1].timeStamp == transactionEntity.timeStamp);
		REQUIRE(list[len - 1].txHash == transactionEntity.txHash);
		REQUIRE(list[len - 1].buff.length == 0);
	}

	SECTION("DatabaseManager deleteTxByHash test") {
		std::vector<TransactionEntity> list = databaseManager.getAllTransactions(iso);
		ssize_t len = list.size();

		bool result = databaseManager.deleteTxByHash(iso, "1234");
		REQUIRE(result == true);

		list = databaseManager.getAllTransactions(iso);
		REQUIRE(list.size() == len);

		result = databaseManager.deleteTxByHash(iso,
		                                        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		REQUIRE(result == true);

		list = databaseManager.getAllTransactions(iso);
		REQUIRE(list.size() < len);

		result = databaseManager.deleteAllTransactions(iso);
		REQUIRE(result == true);

		list = databaseManager.getAllTransactions(iso);
		REQUIRE(list.size() == 0);
	}
}

TEST_CASE("DatabaseManager peer interface test", "[DatabaseManager]") {

	DatabaseManager databaseManager;
	std::string iso = "ELA";

	SECTION("DatabaseManager putPeer test") {
		PeerEntity peerEntity;
		peerEntity.id = 1;
		peerEntity.timeStamp = 1523935888;
		peerEntity.address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
		peerEntity.port = 9090;

		bool result = databaseManager.putPeer(iso, peerEntity);
		REQUIRE(result == true);

		std::vector<PeerEntity> list = databaseManager.getAllPeers(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);
		REQUIRE(list[len - 1].id >= 1);
		REQUIRE(list[len - 1].timeStamp == peerEntity.timeStamp);
		REQUIRE(list[len - 1].port == peerEntity.port);
		int res = UInt128Eq(&list[len - 1].address, &peerEntity.address);
		REQUIRE(res == 1);
	}

	SECTION("DatabaseManager deleteAllPeers and putPeers test") {
		bool result = databaseManager.deleteAllPeers(iso);
		REQUIRE(result == true);

		std::vector<PeerEntity> list = databaseManager.getAllPeers(iso);
		ssize_t len = list.size();
		REQUIRE(len == 0);

		list.clear();
		PeerEntity peerEntity;
		for (int i = 0; i < 10; i++) {
			peerEntity.timeStamp = 1523935888 + i;
			peerEntity.address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
			peerEntity.port = 8080 + i;
			list.push_back(peerEntity);
		}

		result = databaseManager.putPeers(iso, list);
		REQUIRE(result == true);

		std::vector<PeerEntity> peerList = databaseManager.getAllPeers(iso);
		REQUIRE(peerList.size() == list.size());

		for (int i = 0; i < peerList.size(); i++) {
			REQUIRE(peerList[i].id > 0);
			REQUIRE(peerList[i].timeStamp == list[i].timeStamp);
			REQUIRE(peerList[i].port == list[i].port);
			int res = UInt128Eq(&peerList[i].address, &list[i].address);
			REQUIRE(res == 1);
		}
	}

	SECTION("DatabaseManager deletePeer test") {
		std::vector<PeerEntity> list = databaseManager.getAllPeers(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);

		PeerEntity peerEntity;
		bool result = databaseManager.deletePeer(iso, peerEntity);
		REQUIRE(result == true);

		list = databaseManager.getAllPeers(iso);
		REQUIRE(list.size() == len);

		peerEntity.id = list[len - 1].id;
		result = databaseManager.deletePeer(iso, peerEntity);
		REQUIRE(result == true);

		list = databaseManager.getAllPeers(iso);
		REQUIRE(list.size() == len - 1);
	}
}

TEST_CASE("DatabaseManager merkleBlock test", "[DatabaseManager]") {

	DatabaseManager databaseManager;
	std::string iso = "ELA";

	SECTION("DatabaseManager putMerkleBlock test") {
		MerkleBlockEntity blockEntity;
		blockEntity.blockHeight = 8888;
		blockEntity.blockBytes.length = 21;
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		blockEntity.blockBytes.data = s;

		bool result = databaseManager.putMerkleBlock(iso, blockEntity);
		REQUIRE(result == true);

		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);
		REQUIRE(list[len - 1].id > 0);
		REQUIRE(list[len - 1].blockHeight == blockEntity.blockHeight);
		REQUIRE(list[len - 1].blockBytes.length == blockEntity.blockBytes.length);
		for (int i = 0; i < list[len - 1].blockBytes.length; i++) {
			REQUIRE(list[len - 1].blockBytes.data[i] == blockEntity.blockBytes.data[i]);
		}
	}

	SECTION("DatabaseManager deleteAllBlocks and putMerkleBlocks test") {
		bool result = databaseManager.deleteAllBlocks(iso);
		REQUIRE(result == true);

		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks(iso);
		ssize_t len = list.size();
		REQUIRE(len == 0);

		list.clear();
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		for (int i = 0; i < 10; i++) {
			MerkleBlockEntity blockEntity;
			blockEntity.blockBytes.length = 21;
			blockEntity.blockBytes.data = s;
			blockEntity.blockHeight = i + 1;
			list.push_back(blockEntity);
		}

		result = databaseManager.putMerkleBlocks(iso, list);
		REQUIRE(result == true);

		std::vector<MerkleBlockEntity> blockList = databaseManager.getAllMerkleBlocks(iso);
		REQUIRE(blockList.size() == list.size());
		for (int i = 0; i < list.size(); i++) {
			REQUIRE(blockList[i].id > 0);
			REQUIRE(blockList[i].blockHeight == list[i].blockHeight);
			REQUIRE(blockList[i].blockBytes.length == list[i].blockBytes.length);
			for (int j = 0; j < blockList[i].blockBytes.length; j++) {
				REQUIRE(blockList[i].blockBytes.data[j] == list[i].blockBytes.data[j]);
			}
		}
	}

	SECTION("DatabaseManager deleteMerkleBlock test") {
		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);

		MerkleBlockEntity blockEntity;
		bool result = databaseManager.deleteMerkleBlock(iso, blockEntity);
		REQUIRE(result == true);

		list = databaseManager.getAllMerkleBlocks(iso);
		REQUIRE(list.size() == len);

		blockEntity.id = list[len - 1].id;
		result = databaseManager.deleteMerkleBlock(iso, blockEntity);
		REQUIRE(result == true);

		list = databaseManager.getAllMerkleBlocks(iso);
		REQUIRE(list.size() == len - 1);
	}
}