/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: wallet_core.cpp
 *
 * DESC: This is the cpp file of the wallet core.
 */
#include "wallet_core.hpp"

const std::string CHAINID_ELA = "ELA";
const std::string CHAINID_ID = "IDChain";
const std::string CHAINID_ETHSC = "ETHSC";
std::string walletRoot;
std::string network;

MasterWalletManager *manager = nullptr;
IMasterWallet *currentWallet = nullptr;

typedef std::map<std::string, WalletData> SubWalletData;
typedef std::map<std::string, SubWalletData> MasterWalletData;

MasterWalletData masterWalletData;

void signAndPublishTx(ISubWallet *subWallet, const nlohmann::json &tx, const std::string &payPasswd) {
	std::string password = payPasswd;
	if (payPasswd.empty()) {
        return;
	}
	nlohmann::json signedTx = subWallet->SignTransaction(tx, password);
	subWallet->PublishTransaction(signedTx);
}

void subWalletOpen(IMasterWallet *masterWallet, ISubWallet *subWallet) {
    if (subWallet == nullptr)
        return;
	WalletData walletData;
	ISubWalletCallback *callback = new SubWalletCallback(masterWallet->GetID(), subWallet->GetChainID());

	walletData.SetCallback(callback);

	if (masterWalletData.find(masterWallet->GetID()) != masterWalletData.end()) {
		masterWalletData[masterWallet->GetID()][subWallet->GetChainID()] = walletData;
	} else {
		SubWalletData subWalletData;
		subWalletData[subWallet->GetChainID()] = walletData;
		masterWalletData[masterWallet->GetID()] = subWalletData;
	}

	subWallet->AddCallback(callback);
	subWallet->SyncStart();
}

void subWalletClose(IMasterWallet *masterWallet, ISubWallet *subWallet) {
    if (subWallet == nullptr)
        return;
	std::string walletName = masterWallet->GetID();
	std::string chainID = subWallet->GetChainID();

	subWallet->SyncStop();
	subWallet->RemoveCallback();

	auto callback = static_cast<SubWalletCallback *>(masterWalletData[walletName][chainID].GetCallback());
	delete callback;
	callback = nullptr;

	masterWalletData[walletName].erase(chainID);
	if (masterWalletData[walletName].empty())
		masterWalletData.erase(walletName);
}

void walletInit(void) {
	auto masterWalletIDs = manager->GetAllMasterWalletID();
	if (!currentWallet && !masterWalletIDs.empty()) {
		currentWallet = manager->GetMasterWallet(masterWalletIDs[0]);

		auto subWallets = currentWallet->GetAllSubWallets();
		for (ISubWallet *subWallet : subWallets) {
			subWalletOpen(currentWallet, subWallet);
		}
	}
}

void walletCleanup(void) {
	auto masterWalletIDs = manager->GetAllMasterWalletID();
	for (const std::string &masterWalletID : masterWalletIDs) {
		if (manager->WalletLoaded(masterWalletID)) {
			IMasterWallet *masterWallet = manager->GetMasterWallet(masterWalletID);
			auto subWallets = masterWallet->GetAllSubWallets();
			for (ISubWallet *subWallet : subWallets) {
				subWalletClose(masterWallet, subWallet);
			}
		}
	}

	manager->FlushData();
	currentWallet = nullptr;
}

std::pair<int,std::string> removeWallet(std::string walletName) {
	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (!masterWallet) {
            return { ERRNO_APP, "No master wallet" };
		}

		bool needUpdate = false;
		if (masterWallet == currentWallet) {
			needUpdate = true;
		}

		auto subWallets = masterWallet->GetAllSubWallets();

		for (ISubWallet *subWallet : subWallets) {
			subWalletClose(masterWallet, subWallet);
		}

		manager->DestroyWallet(walletName);

		if (needUpdate)  {
			auto masterWalletIDs = manager->GetAllMasterWalletID();
			if (masterWalletIDs.empty()) {
				currentWallet = nullptr;
			} else {
				currentWallet = manager->GetMasterWallet(masterWalletIDs[0]);
			}
		}
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, "" };
}

std::pair<int,std::string> switchWallet(std::string walletName) {
	try {
		bool loaded = manager->WalletLoaded(walletName);
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (masterWallet == nullptr) {
            return { ERRNO_APP, walletName+" no found" };
		}

		currentWallet = masterWallet;
		if (!loaded) {
			auto subWallets = currentWallet->GetAllSubWallets();
			for (ISubWallet *subWallet : subWallets) {
				subWalletOpen(currentWallet, subWallet);
			}
		}
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, "" };
}

std::pair<int,std::string> openChainID(std::string chainID) {
	try {
        if (currentWallet == nullptr)
            return { ERRNO_APP, "No master wallet" };
		ISubWallet *subWallet = currentWallet->CreateSubWallet(chainID);
        if (subWallet == nullptr)
            return { ERRNO_APP, "No subwallet" };
		subWalletOpen(currentWallet, subWallet);
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, "" };
}

std::pair<int,std::string> closeChainID(std::string chainID) {
	try {
        if (currentWallet == nullptr)
            return { ERRNO_APP, "No master wallet" };
        ISubWallet *subWallet = currentWallet->GetSubWallet(chainID);
        if (subWallet == nullptr)
            return { ERRNO_APP, "No subwallet" };
		subWalletClose(currentWallet, subWallet);
		currentWallet->DestroyWallet(chainID);
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, "" };
}

std::pair<int,std::string> depositToSideChain(nlohmann::json tx, std::string password) {
	try {
        if (currentWallet == nullptr)
            return { ERRNO_APP, "No master wallet" };
        IMainchainSubWallet *subWallet = dynamic_cast<IMainchainSubWallet *>(currentWallet->GetSubWallet(CHAINID_ELA));
        
		signAndPublishTx(subWallet, tx, password);
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, "" };
}

std::pair<int,std::string> withdrawToMainChain(std::string chainID, nlohmann::json tx, std::string password) {
	try {
        if (currentWallet == nullptr)
            return { ERRNO_APP, "No master wallet" };
        ISidechainSubWallet *subWallet = dynamic_cast<ISidechainSubWallet *>(currentWallet->GetSubWallet(chainID));
        
		signAndPublishTx(subWallet, tx, password);
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, "" };
}

nlohmann::json getTransferTransaction(std::string chainID, std::string addr, std::string transferAmount) {
    try {
        if (currentWallet == nullptr)
            return nullptr;
        ISubWallet *subWallet = currentWallet->GetSubWallet(chainID);
        if (subWallet == nullptr)
            return nullptr;
        nlohmann::json tx = subWallet->CreateTransaction("", addr, transferAmount, "");
        return tx;
    } catch (const std::exception &e) {
        return nullptr;
    }
}

std::pair<int,std::string> transferFromChainIDToAddress(std::string chainID, nlohmann::json tx, std::string password) {
	try {
        if (currentWallet == nullptr)
            return { ERRNO_APP, "No master wallet" };
        ISubWallet *subWallet = currentWallet->GetSubWallet(chainID);
        if (subWallet == nullptr)
            return { ERRNO_APP, "No subwallet" };
		signAndPublishTx(subWallet, tx, password);
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, "" };
}

std::pair<int,std::string> createReceiveAddress(std::string chainID, std::string &address) {
	try {
        if (currentWallet == nullptr)
            return { ERRNO_APP, "No master wallet" };
        ISubWallet *subWallet = currentWallet->GetSubWallet(chainID);
        if (subWallet == nullptr)
            return { ERRNO_APP, "No subwallet" };
        address = subWallet->CreateAddress();
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, ""} ;
}

std::pair<int,std::string> changeSyncStatus(std::string chainID, bool start) {
	try {
        if (currentWallet == nullptr)
            return { ERRNO_APP, "No master wallet" };
        ISubWallet *subWallet = currentWallet->GetSubWallet(chainID);
        if (subWallet == nullptr)
            return { ERRNO_APP, "No subwallet" };
		if (start) {
			subWallet->SyncStart();
        } else {
			subWallet->SyncStop();
        }
	} catch (const std::exception &e) {
        return { ERRNO_APP, e.what() };
	}
    return { 0, "" };
}

int mkdirInternal(const char *path, mode_t mode) {
    struct stat st;
    int rc = 0;

    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            rc = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        rc = -1;
    }

    return rc;
}

int mkdirs(const char *path, mode_t mode) {
    int rc = 0;
    char *pp;
    char *sp;
    char copypath[PATH_MAX];

    strncpy(copypath, path, sizeof(copypath));
    copypath[sizeof(copypath) - 1] = 0;

    pp = copypath;
    while (rc == 0 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            /* Neither root nor double slash in path */
            *sp = '\0';
            rc = mkdirInternal(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }

    if (rc == 0)
        rc = mkdirInternal(path, mode);

    return rc;
}

std::string getWalletDir(const std::string &dir) {
	std::string result = dir;

	if (dir.empty()) {
		result = getenv("HOME");
		result += "/.wallet";
	}

	if (result.back() == '/' || result.back() == '\\')
		result.pop_back();

	return result;
}

#ifdef HAVE_SYS_RESOURCE_H

#include <sys/resource.h>

int sys_coredump_set(bool enable) {
	const struct rlimit rlim = {
		enable ? RLIM_INFINITY : 0,
		enable ? RLIM_INFINITY : 0
	};

	return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

void signalHandler(int signum) {
	walletCleanup();
	delete manager;
	exit(-1);
}
