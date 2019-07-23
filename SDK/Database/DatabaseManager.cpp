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
			_coinbaseDataStore(&_sqlite),
			_transactionDataStore(&_sqlite),
			_assetDataStore(&_sqlite),
			_merkleBlockDataSource(&_sqlite) {

		}

		DatabaseManager::DatabaseManager() :
			DatabaseManager("spv_wallet.db") {

		}

		DatabaseManager::~DatabaseManager() {

		}

		bool DatabaseManager::PutCoinBase(const std::vector<UTXOPtr> &entitys) {
			return _coinbaseDataStore.Put(entitys);
		}

		bool DatabaseManager::PutCoinBase(const UTXOPtr &entity) {
			return _coinbaseDataStore.Put(entity);
		}

		bool DatabaseManager::DeleteAllCoinBase() {
			return _coinbaseDataStore.DeleteAll();
		}

		size_t DatabaseManager::GetCoinBaseTotalCount() const {
			return _coinbaseDataStore.GetTotalCount();
		}

		std::vector<UTXOPtr> DatabaseManager::GetAllCoinBase() const {
			return _coinbaseDataStore.GetAll();
		}

		bool DatabaseManager::UpdateCoinBase(const std::vector<uint256> &txHashes, uint32_t blockHeight,
											 time_t timestamp) {
			return _coinbaseDataStore.Update(txHashes, blockHeight, timestamp);
		}

		bool DatabaseManager::UpdateSpentCoinBase(const std::vector<uint256> &txHashes) {
			return _coinbaseDataStore.UpdateSpent(txHashes);
		}

		bool DatabaseManager::DeleteCoinBase(const uint256 &hash) {
			return _coinbaseDataStore.Delete(hash);
		}

		bool DatabaseManager::PutTransaction(const std::string &iso, const TransactionPtr &tx) {
			return _transactionDataStore.PutTransaction(iso, tx);
		}

		bool DatabaseManager::DeleteAllTransactions(const std::string &iso) {
			return _transactionDataStore.DeleteAllTransactions(iso);
		}

		size_t DatabaseManager::GetAllTransactionsCount(const std::string &iso) const {
			return _transactionDataStore.GetAllTransactionsCount(iso);
		}

		std::vector<TransactionPtr> DatabaseManager::GetAllTransactions(const std::string &iso) const {
			return _transactionDataStore.GetAllTransactions(iso);
		}

		bool DatabaseManager::UpdateTransaction(const std::vector<uint256> &hashes, uint32_t blockHeight,
												time_t timestamp) {
			return _transactionDataStore.UpdateTransaction(hashes, blockHeight, timestamp);
		}

		bool DatabaseManager::DeleteTxByHash(const std::string &iso, const std::string &hash) {
			return _transactionDataStore.DeleteTxByHash(iso, hash);
		}

		bool DatabaseManager::DeleteTxByHashes(const std::vector<std::string> &hashes) {
			return _transactionDataStore.DeleteTxByHashes(hashes);
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

		bool DatabaseManager::PutMerkleBlock(const std::string &iso, const MerkleBlockPtr &blockPtr) {
			return _merkleBlockDataSource.PutMerkleBlock(iso, blockPtr);
		}

		bool DatabaseManager::PutMerkleBlocks(const std::string &iso,
											  const std::vector<MerkleBlockPtr> &blocks) {
			return _merkleBlockDataSource.PutMerkleBlocks(iso, blocks);
		}

		bool DatabaseManager::DeleteMerkleBlock(const std::string &iso, long id) {
			return _merkleBlockDataSource.DeleteMerkleBlock(iso, id);
		}

		bool DatabaseManager::DeleteAllBlocks(const std::string &iso) {
			return _merkleBlockDataSource.DeleteAllBlocks(iso);
		}

		std::vector<MerkleBlockPtr> DatabaseManager::GetAllMerkleBlocks(const std::string &iso, const std::string &pluginType) const {
			return _merkleBlockDataSource.GetAllMerkleBlocks(iso, pluginType);
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

	}
}
