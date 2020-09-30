/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: createwalletwindow2.h
 *
 * DESC: This is the head file of the second wallet creating window.
 *
 *       There are total 4 windows to guide user to create a new wallet, and
 *       this is the 2nd window. It generates the mnemonic words according to
 *       the mnemonic language and mnemonic length selected in the first window,
 *       and provides the copy and export functions.
 *
 * STRUCTURE:
 *      load registry.hpp
 *          -include "registry.hpp"
 *          -import global variables for registry settings
 *      load wallet_core.hpp
 *          -include "wallet_core.hpp"
 *          -import global variables for wallet core functions
 *      class CreateWalletWindow2
 */
#ifndef CREATEWALLETWINDOW2_H
#define CREATEWALLETWINDOW2_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

#include <iostream>

///////////////////////
// load registry.hpp //
///////////////////////
#include "registry.hpp"
extern QString curWalletRoot;
extern bool isWalletRootUpdate;
extern QTranslator *trans;
extern QString curLang;
extern bool isLangUpdate;
extern QString curNetwork;
extern bool isNetworkUpdate;
extern QString networkLastUpdateTime;
extern QString networkConfigFileDir;
extern QString strIsWalletAutoUpdate;
extern bool isWalletAutoUpdate;
extern QString strDataAutoUpdateInterval;
extern int dataAutoUpdateInterval;
extern QMap<QString,QString> paymentAddressData;
extern QMap<QString,QVariant> paymentAddressDataVariant;

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

namespace Ui {
class CreateWalletWindow2;
}

class CreateWalletWindow2 : public QDialog
{
    Q_OBJECT
public:
    static QString mnemonic;

private:
    // save data
    bool saveTextFile(const QString &filename);
    void showEvent(QShowEvent *event) override;

public:
    explicit CreateWalletWindow2(QWidget *parent = nullptr);
    ~CreateWalletWindow2();

private slots:
    void on_btnPrevious_clicked();

    void on_btnNext_clicked();

    void on_btnSaveTo_clicked();

    void on_cbConfirm_stateChanged(int arg1);

private:
    Ui::CreateWalletWindow2 *ui;
};

#endif // CREATEWALLETWINDOW2_H
