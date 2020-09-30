/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: main.cpp
 *
 * DESC: main.cpp
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loginwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QDateTime>

///////////////////////
// load registry.hpp //
///////////////////////
#include "registry.hpp"
QString curWalletRoot;
bool isWalletRootUpdate;
QTranslator *trans = nullptr;
QString curLang;
bool isLangUpdate;
QString curNetwork;
bool isNetworkUpdate;
QString networkLastUpdateTime;
QString networkConfigFileDir;
QString curSubWalletChainID;
bool isSubWalletChainIDUpdate;
QString strIsWalletAutoUpdate;
bool isWalletAutoUpdate;
QString strDataAutoUpdateInterval;
int dataAutoUpdateInterval;
QMap<QString,QString> paymentAddressData;
QMap<QString,QVariant> paymentAddressDataVariant;

//////////////////////////
// load wallet_core.hpp //
//////////////////////////
#include "wallet_core.hpp"
extern const std::string CHAINID_ELA;
extern const std::string CHAINID_ID;
extern const std::string CHAINID_ETHSC;
extern std::string walletRoot;
extern std::string network;
extern MasterWalletManager *manager;
extern IMasterWallet *currentWallet;
extern bool verboseMode;
extern const char *SplitLine;
extern MasterWalletData masterWalletData;

int main(int argc, char *argv[])
{
#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGHUP, signalHandler);

    ///////////////////////
    // start application //
    ///////////////////////
    QApplication a(argc, argv);
    // read registry
    // set WalletRoot
    isWalletRootUpdate = updateWalletRoot();
    curWalletRoot = walletRoot.c_str();
    // set Language
    isLangUpdate = updateLanguage(a);
    // set Network
    networkConfigFileDir = (walletRoot + "/Config.json").c_str();
    isNetworkUpdate = updateNetwork();
    curNetwork = network.c_str();
    networkLastUpdateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // set SubWallet / ChainID
    isSubWalletChainIDUpdate = updateSubWalletChainID();
    // set Wallet's Auto-Update
    updateWalletAutoUpdateSettings();
    // set Payment-Address
    updatePaymentAddressData();

    // initialize wallet
    walletInit();

    // show login window
    LoginWindow *login = new LoginWindow();
    if (login->exec() == QDialog::Accepted)
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
    else
        return 0;
}

/* REGISTRY SETTINGS : update preferences
 * read-settings : readSetting, readSettingMap
 *      wallet-root-update
 *      language-update
 *      network-update
 *      sub-wallet-update
 *      wallet-auto-update
 *      payment-address-update
 */
/* read application settings from registry */
QString readSetting(QString settingName, QString settingDefault)
{
    QSettings settings(ORGNAZATION, APP_NAME);
    QString settingValue = settings.value(settingName, settingDefault).toString();
    return settingValue;
}
QMap<QString,QVariant> readSettingMap(QString settingName, QMap<QString,QVariant> settingDefault)
{
    QSettings settings(ORGNAZATION, APP_NAME);
    QVariant variantDefault;
    variantDefault.setValue(settingDefault);
    QMap<QString,QVariant> settingValue = settings.value(settingName, variantDefault).toMap();
    return settingValue;
}
void dataMapFromQStringToQVariant(QMap<QString,QString> &stringMap, QMap<QString,QVariant> &variantMap)
{
    variantMap.clear();
    for (QString key : stringMap.keys())
    {
        QVariant tmpVariant;
        tmpVariant.setValue(stringMap[key]);
        variantMap.insert(key, tmpVariant);
    }
}
void dataMapFromQVariantToQString(QMap<QString,QVariant> &variantMap, QMap<QString,QString> &stringMap)
{
    stringMap.clear();
    for (QString key : variantMap.keys())
    {
        stringMap.insert(key, variantMap[key].toString());
    }
}
/* update walletRoot */
bool updateWalletRoot()
{
    walletRoot = getWalletDir(walletRoot);
    if (mkdirs(walletRoot.c_str(), S_IRWXU) < 0)
        return false;
    else
        return true;
}
/* update Language */
bool updateLanguage(QApplication &a)
{
    trans = new QTranslator();
    curLang = readSetting(LANGUAGE_PREF, LANGUAGE_DFLT);
    if (curLang == "EN")
        trans->load(":/ElastosWalletSPV_us_EN.qm");
    else if (curLang == "CN")
        trans->load(":/ElastosWalletSPV_zh_CN.qm");
    else
        trans->load(":/ElastosWalletSPV_us_EN.qm");
    return a.installTranslator(trans);
}
/* update Network */
bool updateNetwork()
{
    // update network config file
    networkConfigFileDir = readSetting(NETWORK_CONFIG_FILE_PREF, NETWORK_CONFIG_FILE_DFLT);

    // update network type
    nlohmann::json netConfig;
    if (!network.empty())
    {
        // user config
        if (network == "MainNet" || network == "TestNet" || network == "RegTest")
        {
            netConfig = nlohmann::json();
        }
        else
        {
            try
            {
                std::ifstream in(networkConfigFileDir.toStdString());
                netConfig = nlohmann::json::parse(in);
                network = "PrvNet";
            }
            catch (const std::exception &e)
            {
                return false;
            }
        }
    }
    else
    {
        // load from local Config.json
        try
        {
            std::ifstream in(walletRoot + "/Config.json");
            netConfig = nlohmann::json::parse(in);
            network = netConfig["NetType"];
        }
        catch (...)
        {
            // ignore exception, use default "MainNet" as config
            netConfig = nlohmann::json();
            network.clear();
        }
    }

    if (network.empty())
    {
        network = "MainNet";
        netConfig = nlohmann::json();
    }

    // clearn up wallet & release old wallet manager
    if (manager != nullptr)
    {
        walletCleanup();
        delete manager;
    }

    // load new wallet manager
    try
    {
        manager = new MasterWalletManager(walletRoot, network, netConfig);
    }
    catch (const std::exception &e)
    {
        return false;
    }
    return true;
}
/* update SubWallet / ChainID */
bool updateSubWalletChainID()
{
    curSubWalletChainID = readSetting(SUBWALLET_CHAINID_PREF, SUBWALLET_CHAINID_DFLT);
    // default open sub-wallet ELA
    if (curSubWalletChainID.isEmpty())
        curSubWalletChainID = CHAINID_ELA.c_str();
    return true;
}
/* update Wallet's Auto-Update Settings */
bool updateWalletAutoUpdateSettings()
{
    // is Auto-Update?
    strIsWalletAutoUpdate = readSetting(WALLET_AUTO_UPDATE_PREF, WALLET_AUTO_UPDATE_DFLT);
    /* default set Wallet's Auto-Update 'true' */
    if (strIsWalletAutoUpdate == "true")
        isWalletAutoUpdate = true;
    else if (strIsWalletAutoUpdate == "false")
        isWalletAutoUpdate = false;
    else
        isWalletAutoUpdate = true;
    // Auto-Update interval
    strDataAutoUpdateInterval = readSetting(WALLET_AUTO_UPDATE_INTERVAL_PREF, WALLET_AUTO_UPDATE_INTERVAL_DFLT);
    dataAutoUpdateInterval = strDataAutoUpdateInterval.toInt();
    /* default set Auto-Update interval '10s' */
    if (dataAutoUpdateInterval <= 0 || dataAutoUpdateInterval > 1000)
        dataAutoUpdateInterval = 10;

    return true;
}
/* update Payment Address Data */
bool updatePaymentAddressData()
{
    paymentAddressDataVariant = readSettingMap(PAYMENT_ADDRESS_DATA_PREF, PAYMENT_ADDRESS_DATA_DFLT);
    dataMapFromQVariantToQString(paymentAddressDataVariant, paymentAddressData);
    return true;
}
/* Group End : update preferences */
