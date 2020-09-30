/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: removewalletwindow.h
 *
 * DESC: This is the head file of the wallet removing window.
 *
 * STRUCTURE:
 *      load registry.hpp
 *          -include "registry.hpp"
 *          -import global variables for registry settings
 *      load wallet_core.hpp
 *          -include "wallet_core.hpp"
 *          -import global variables for wallet core functions
 *      class RemoveWalletWindow
 */
#ifndef REMOVEWALLETWINDOW_H
#define REMOVEWALLETWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QInputDialog>

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

/////////////////////
// load wallet_core.hpp //
/////////////////////
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
class RemoveWalletWindow;
}

class RemoveWalletWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveWalletWindow(QWidget *parent = nullptr);
    ~RemoveWalletWindow();

protected:
    // delete ptr to window when close
    void closeEvent(QCloseEvent *event) override;

signals:
    void removeWalletFinish();

private slots:
    void updateWalletInformation();

private slots:
    void on_btnClearSelected_clicked();

    void on_tbAdd_clicked();

    void on_tbDelete_clicked();

    void on_btnRemove_clicked();

private:
    Ui::RemoveWalletWindow *ui;
};

#endif // REMOVEWALLETWINDOW_H
