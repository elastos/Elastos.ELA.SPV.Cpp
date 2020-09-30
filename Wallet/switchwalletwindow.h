/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: switchwalletwindow.h
 *
 * DESC: This is the head file of the wallet switching window.
 *
 * STRUCTURE:
 *      load registry.hpp
 *          -include "registry.hpp"
 *          -import global variables for registry settings
 *      load wallet_core.hpp
 *          -include "wallet_core.hpp"
 *          -import global variables for wallet core functions
 *      class SwitchWalletWindow
 */
#ifndef SWITCHWALLETWINDOW_H
#define SWITCHWALLETWINDOW_H

#include <QDialog>
#include <QMessageBox>

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
class SwitchWalletWindow;
}

class SwitchWalletWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SwitchWalletWindow(QWidget *parent = nullptr);
    ~SwitchWalletWindow();

protected:
    // delete ptr to window when close
    void closeEvent(QCloseEvent *event) override;

signals:
    void switchWalletFinish();

private slots:
    void updateWalletInformation();

private slots:
    void on_btnSwitch_clicked();

private:
    Ui::SwitchWalletWindow *ui;
};

#endif // SWITCHWALLETWINDOW_H
