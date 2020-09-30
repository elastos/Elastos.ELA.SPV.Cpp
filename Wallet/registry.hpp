/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: registry.hpp
 *
 * DESC: This is the head file for registry settings.
 *
 * STRUCTURE:
 *      registry parameters
 */
#ifndef REGISTRY_HPP
#define REGISTRY_HPP

#include <QApplication>

/////////////////////////
// registry parameters //
/////////////////////////
// prototype
QString readSetting(QString settingName, QString settingDefault);
QMap<QString,QVariant> readSettingMap(QString settingName, QMap<QString,QVariant> settingDefault);
void dataMapFromQStringToQVariant(QMap<QString,QString> &stringMap, QMap<QString,QVariant> &variantMap);
void dataMapFromQVariantToQString(QMap<QString,QVariant> &variantMap, QMap<QString,QString> &stringMap);
bool updateWalletRoot();
bool updateLanguage(QApplication &a);
bool updateNetwork();
bool updateSubWalletChainID();
bool updateWalletAutoUpdateSettings();
bool updatePaymentAddressData();
// WalletRoot
#define WALLET_ROOT_PREF "WalletRoot"
#define WALLET_ROOT_DFLT getWalletDir(walletRoot).c_str()
extern QString curWalletRoot;
extern bool isWalletRootUpdate;
// Language
#define LANGUAGE_PREF "Language"
#define LANGUAGE_DFLT "EN"
extern QTranslator *trans;
extern QString curLang;
extern bool isLangUpdate;
// Network
#define NETWORK_PREF "Network"
#define NETWORK_DFLT "MainNet"
extern QString curNetwork;
extern bool isNetworkUpdate;
extern QString networkLastUpdateTime;
#define NETWORK_CONFIG_FILE_PREF "NetworkConfigFile"
#define NETWORK_CONFIG_FILE_DFLT (walletRoot + "/Config.json").c_str()
extern QString networkConfigFileDir;
// Sub Wallet / ChainID
#define SUBWALLET_CHAINID_PREF "SubWalletChainID"
#define SUBWALLET_CHAINID_DFLT CHAINID_ELA.c_str()
extern QString curSubWalletChainID;
extern bool isSubWalletChainIDUpdate;
// Wallet's Auto-Update Settings
#define WALLET_AUTO_UPDATE_PREF "WalletAutoUpdate"
#define WALLET_AUTO_UPDATE_DFLT "true"
extern QString strIsWalletAutoUpdate;
extern bool isWalletAutoUpdate;
#define WALLET_AUTO_UPDATE_INTERVAL_PREF "WalletAutoUpdateInterval"
#define WALLET_AUTO_UPDATE_INTERVAL_DFLT "10"
extern QString strDataAutoUpdateInterval;
extern int dataAutoUpdateInterval;
// Payment Address Data
#define PAYMENT_ADDRESS_DATA_PREF "PaymentAddressData"
#define PAYMENT_ADDRESS_DATA_DFLT paymentAddressDataVariant
extern QMap<QString,QString> paymentAddressData;
extern QMap<QString,QVariant> paymentAddressDataVariant;

#endif // REGISTRY_HPP
