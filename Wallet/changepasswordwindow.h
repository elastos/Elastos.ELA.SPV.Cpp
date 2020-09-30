/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: changepasswordwindow.h
 *
 * DESC: This is the head file of the wallet password changing window.
 *
 *       It is a independent window and it's better to display this window
 *       in modal mode. When the window is closed, the password should have
 *       been changed already, which means that it never emit a signal to ask
 *       changing password. Thus, it can be used in any condition by any type of
 *       invoker widget.
 *
 * STRUCTURE:
 *      load registry.hpp
 *          -include "registry.hpp"
 *          -import global variables for registry settings
 *      load wallet_core.hpp
 *          -include "wallet_core.hpp"
 *          -import global variables for wallet core functions
 *      class ChangePasswordWindow
 */
#ifndef CHANGEPASSWORDWINDOW_H
#define CHANGEPASSWORDWINDOW_H

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
class ChangePasswordWindow;
}

class ChangePasswordWindow : public QDialog
{
    Q_OBJECT
private:
    bool hasPassword = false;
    bool hasConfirmed = false;

public:
    explicit ChangePasswordWindow(QWidget *parent = nullptr);
    ~ChangePasswordWindow();

protected:
    // delete ptr to window when close
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_leNewPassword_textChanged(const QString &arg1);

    void on_leConfirm_textChanged(const QString &arg1);

    void on_cbVisible_stateChanged(int arg1);

    void on_btnClearAll_clicked();

    void on_btnOK_clicked();

private:
    Ui::ChangePasswordWindow *ui;
};

#endif // CHANGEPASSWORDWINDOW_H
