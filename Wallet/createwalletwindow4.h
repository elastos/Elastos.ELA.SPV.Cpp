/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: createwalletwindow4.h
 *
 * DESC: This is the head file of the fourth wallet creating window.
 *
 *       There are total 4 windows to guide user to create a new wallet, and
 *       this is the 4th window. It asks user to input payment-password of the
 *       new wallet.
 *
 * STRUCTURE:
 *      load registry.hpp
 *          -include "registry.hpp"
 *          -import global variables for registry settings
 *      load wallet_core.hpp
 *          -include "wallet_core.hpp"
 *          -import global variables for wallet core functions
 *      class CreateWalletWindow4
 */
#ifndef CREATEWALLETWINDOW4_H
#define CREATEWALLETWINDOW4_H

#include <QDialog>
#include <QMessageBox>

#include <exception>

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
class CreateWalletWindow4;
}

class CreateWalletWindow4 : public QDialog
{
    Q_OBJECT
public:
    static QString password;

private:
    bool hasPassword = false;
    bool hasConfirmed = false;
    void createWallet();

public:
    explicit CreateWalletWindow4(QWidget *parent = nullptr);
    ~CreateWalletWindow4();

signals:
    void createWalletFinish();

private slots:
    void on_btnPrevious_clicked();

    void on_cbVisible_clicked(bool checked);

    void on_lePassword_textChanged(const QString &arg1);

    void on_leConfirm_textChanged(const QString &arg1);

    void on_btnCreate_clicked();

private:
    Ui::CreateWalletWindow4 *ui;
};

#endif // CREATEWALLETWINDOW4_H
