// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "DatabaseManager.h"

namespace Elastos {
	namespace ElaWallet {

		DatabaseManager::DatabaseManager(const boost::filesystem::path &path) :
			_path(path),
			_sqlite(path),
			_peerDataSource(&_sqlite),
			_transactionDataStore(&_sqlite),
			_assetDataStore(&_sqlite),
			_merkleBlockDataSource(&_sqlite),
			_nep5LogDataStore(&_sqlite) {

		}

		DatabaseManager::DatabaseManager() :
			DatabaseManager("spv_wallet.db") {

		}

		DatabaseManager::~DatabaseManager() {

		}

		bool DatabaseManager::PutTransaction(const std::string &iso, const TransactionEntity &tx) {
			return _transactionDataStore.PutTransaction(iso, tx);
		}

		bool DatabaseManager::DeleteAllTransactions(const std::string &iso) {
			return _transactionDataStore.DeleteAllTransactions(iso);
		}

		size_t DatabaseManager::GetAllTransactionsCount(const std::string &iso) const {
			return _transactionDataStore.GetAllTransactionsCount(iso);
		}

		std::vector<TransactionEntity> DatabaseManager::GetAllTransactions(const std::string &iso) const {
			return _transactionDataStore.GetAllTransactions(iso);
		}

		bool DatabaseManager::UpdateTransaction(const std::string &iso, const TransactionEntity &txEntity) {
			return _transactionDataStore.UpdateTransaction(iso, txEntity);
		}

		bool DatabaseManager::DeleteTxByHash(const std::string &iso, const std::string &hash) {
			return _transactionDataStore.DeleteTxByHash(iso, hash);
		}

		bool DatabaseManager::PutPeer(const std::string &iso, const PeerEntity &peerEntity) {
			return _peerDataSource.PutPeer(iso, peerEntity);
		}

		bool DatabaseManager::PutPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities) {
			return _peerDataSource.PutPeers(iso, peerEntities);
		}

		bool DatabaseManager::DeletePeer(const std::string &iso, const PeerEntity &peerEntity) {
			return _peerDataSource.DeletePeer(iso, peerEntity);
		}

		bool DatabaseManager::DeleteAllPeers(const std::string &iso) {
			return _peerDataSource.DeleteAllPeers(iso);
		}

		std::vector<PeerEntity> DatabaseManager::GetAllPeers(const std::string &iso) const {
			return _peerDataSource.GetAllPeers(iso);
		}

		size_t DatabaseManager::GetAllPeersCount(const std::string &iso) const {
			return _peerDataSource.GetAllPeersCount(iso);
		}

		bool DatabaseManager::PutMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return _merkleBlockDataSource.PutMerkleBlock(iso, blockEntity);
		}

		bool DatabaseManager::PutMerkleBlocks(const std::string &iso,
											  const std::vector<MerkleBlockEntity> &blockEntities) {
			return _merkleBlockDataSource.PutMerkleBlocks(iso, blockEntities);
		}

		bool DatabaseManager::DeleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return _merkleBlockDataSource.DeleteMerkleBlock(iso, blockEntity);
		}

		bool DatabaseManager::DeleteAllBlocks(const std::string &iso) {
			return _merkleBlockDataSource.DeleteAllBlocks(iso);
		}

		std::vector<MerkleBlockEntity> DatabaseManager::GetAllMerkleBlocks(const std::string &iso) const {
			return _merkleBlockDataSource.GetAllMerkleBlocks(iso);
		}

		const boost::filesystem::path &DatabaseManager::GetPath() const {
			return _path;
		}

		bool DatabaseManager::PutAsset(const std::string &iso, const AssetEntity &asset) {
			return _assetDataStore.PutAsset(iso, asset);
		}

		bool DatabaseManager::DeleteAsset(const std::string &assetID) {
			return _assetDataStore.DeleteAsset(assetID);
		}

		bool DatabaseManager::DeleteAllAssets() {
			return _assetDataStore.DeleteAllAssets();
		}

		bool DatabaseManager::GetAssetDetails(const std::string &assetID, AssetEntity &asset) const {
			return _assetDataStore.GetAssetDetails(assetID, asset);
		}

		std::vector<AssetEntity> DatabaseManager::GetAllAssets() const {
			return _assetDataStore.GetAllAssets();
		}

		bool DatabaseManager::PutNep5Log(const std::string &iso, const Nep5LogEntity &nep5LogEntity) {
			return _nep5LogDataStore.PutNep5Log(iso, nep5LogEntity);
		}

		bool DatabaseManager::DeleteNep5Log(const std::string &iso, const std::string &txid) {
			return _nep5LogDataStore.DeleteLogByTxID(iso, txid);
		}

		bool DatabaseManager::DeleteAllNep5Logs(const std::string &iso) {
			return _nep5LogDataStore.DeleteAllNep5Logs(iso);
		}

		bool DatabaseManager::GetNep5Log(const std::string &iso, const std::string &txid, Nep5LogEntity &nep5LogEntity) {
			return _nep5LogDataStore.GetNep5LogByTxID(iso, txid, nep5LogEntity);
		}

		const std::vector<Nep5LogEntity> DatabaseManager::GetAllLogs() const {
			return _nep5LogDataStore.GetAllLogs();
		}

	}
}
