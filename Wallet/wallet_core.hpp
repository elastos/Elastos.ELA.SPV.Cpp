/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: wallet_core.hpp
 *
 * DESC: This is the head file of the wallet core.
 *
 * STRUCTURE:
 *      wallet core
 */
#ifndef WALLET_CORE_HPP
#define WALLET_CORE_HPP

#define ORGNAZATION "Elastos"
#define APP_NAME "ElastosWalletSPV"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <termios.h>

#include <MasterWalletManager.h>
#include <IMasterWallet.h>
#include <ISubWallet.h>
#include <IMainchainSubWallet.h>
#include <IIDChainSubWallet.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace Elastos::ElaWallet;

#define ERRNO_APP -255

#define SELA_PER_ELA 100000000UL
extern const std::string CHAINID_ELA;
extern const std::string CHAINID_ID;
extern const std::string CHAINID_ETHSC;
extern std::string walletRoot;
extern std::string network;

extern MasterWalletManager *manager;
extern IMasterWallet *currentWallet;

class WalletData {
public:
	WalletData() :
		_callback(nullptr),
		_syncProgress(0),
		_speed(0),
		_lastBlockTime(0)
		{}

	void SetCallback(ISubWalletCallback *callback) {
		_callback = callback;
	}

	ISubWalletCallback *GetCallback() const {
		return _callback;
	}

	void SetSyncProgress(int progress) {
		_syncProgress = progress;
	}

	int GetSyncProgress() const {
		return _syncProgress;
	}

	void SetDownloadPeer(const std::string &peer) {
		_downloadPeer = peer;
	}

	const std::string &GetDownloadPeer() const {
		return _downloadPeer;
	}

	void SetSpeed(int speed) {
		_speed = speed;
	}

	int GetSpeed() const {
		return _speed;
	}

	void SetLastBlockTime(time_t t) {
		_lastBlockTime = t;
	}

	time_t GetLastBlockTime() const {
		return _lastBlockTime;
	}

	void SetLastPublishedTx(const std::string &txHash) {
		_lastPublishedTx = txHash;
	}

	const std::string &GetLastPublishedTx() const  {
		return _lastPublishedTx;
	}

	void SetLastPublishedTxResult(const std::string &result) {
		_lastPublishedTxResult = result;
	}

	const std::string &GetLastPublishedTxResult() const {
		return _lastPublishedTxResult;
	}

private:
	ISubWalletCallback *_callback;
	int _syncProgress;
	std::string _downloadPeer;
	int _speed;
	time_t _lastBlockTime;

	std::string _lastPublishedTx;
	std::string _lastPublishedTxResult;
};

typedef std::map<std::string, WalletData> SubWalletData;
typedef std::map<std::string, SubWalletData> MasterWalletData;

extern MasterWalletData masterWalletData;

class SubWalletCallback : public ISubWalletCallback {
public:
    ~SubWalletCallback() {
    }

	SubWalletCallback(const std::string &masterWalletID, const std::string &walletID) :
		_walletID(masterWalletID),
        _chainID(walletID) {
    }

    virtual void OnTransactionStatusChanged(
            const std::string &txid, const std::string &status,
            const nlohmann::json &desc, uint32_t confirms) {
	}

	virtual void OnBlockSyncStarted() {
	}

	virtual void OnBlockSyncProgress(const nlohmann::json &progressInfo) {
		masterWalletData[_walletID][_chainID].SetSyncProgress(progressInfo["Progress"].get<int>());
		masterWalletData[_walletID][_chainID].SetDownloadPeer(progressInfo["DownloadPeer"].get<std::string>());
		masterWalletData[_walletID][_chainID].SetSpeed(progressInfo["BytesPerSecond"].get<int>());
		masterWalletData[_walletID][_chainID].SetLastBlockTime(progressInfo["LastBlockTime"].get<time_t>());
	}

	virtual void OnBlockSyncStopped() {
	}

	virtual void OnBalanceChanged(const std::string &asset, const std::string &balance) {
	}

	virtual void OnTxPublished(const std::string &hash, const nlohmann::json &result) {
	}

	virtual void OnAssetRegistered(const std::string &asset, const nlohmann::json &info) {
	}

	virtual void OnConnectStatusChanged(const std::string &status) {
	}

	virtual void OnETHSCEventHandled(const nlohmann::json &event) {
	}

private:
	std::string _walletID;
	std::string _chainID;
};

void signAndPublishTx(ISubWallet *subWallet, const nlohmann::json &tx, const std::string &payPasswd);

void subWalletOpen(IMasterWallet *masterWallet, ISubWallet *subWallet);

void subWalletClose(IMasterWallet *masterWallet, ISubWallet *subWallet);

void walletInit(void);

void walletCleanup(void);
                                            
std::pair<int,std::string> removeWallet(std::string walletName);

std::pair<int,std::string> switchWallet(std::string walletName);

std::pair<int,std::string> openChainID(std::string chainID);

std::pair<int,std::string> closeChainID(std::string chainID);

std::pair<int,std::string> depositToSideChain(nlohmann::json tx, std::string password);

std::pair<int,std::string> withdrawToMainChain(std::string chainID, nlohmann::json tx, std::string password);

nlohmann::json getTransferTransaction(std::string chainID, std::string addr, std::string transferAmount);

std::pair<int,std::string> transferFromChainIDToAddress(std::string chainID, nlohmann::json tx, std::string password);

std::pair<int,std::string> createReceiveAddress(std::string chainID, std::string &address);

std::pair<int,std::string> changeSyncStatus(std::string chainID, bool start);

int mkdirInternal(const char *path, mode_t mode);

int mkdirs(const char *path, mode_t mode);

std::string getWalletDir(const std::string &dir);

#ifdef HAVE_SYS_RESOURCE_H

#include <sys/resource.h>

int sys_coredump_set(bool enable);
#endif

void signalHandler(int signum);

#endif /* WALLET_CORE_HPP */
