/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: getaddressamountwindow.h
 *
 * DESC: This is the head file of the 'address & amount' window.
 *
 *       At first this window is used to get address and amount from user, but
 *       in fact it also processes the corresponding operations, so it loads
 *       wallet_core.hpp.
 *
 *       Since it is a independent window that processes operations itself, it
 *       is better to display this window in modal mode, and thus it can be used
 *       in any condition by any invoker widget.
 *
 *       It is used only to process 'Deposit' and 'Withdraw' operations now. More
 *       operations can be added in the future as needed.
 *
 *       For 'Deposit' and 'Withdraw', it has a function 'getOperationTxAndUpdateOperationDetails'
 *       to get the corresponding operation's transaction and store it in the class member 'operationTx'.
 *       When operationTx is setted properly, the 'perform operation' button will be enabled and the
 *       true processing operation will be performed based on 'operationTx'.
 *
 * STRUCTURE:
 *      load wallet_core.hpp
 *          -include "wallet_core.hpp"
 *          -import global variables for wallet core functions
 *      class GetAddressAmountWindow
 */
#ifndef GETADDRESSAMOUNTWINDOW_H
#define GETADDRESSAMOUNTWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QInputDialog>

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
class GetAddressAmountWindow;
}

class GetAddressAmountWindow : public QDialog
{
    Q_OBJECT
private:
    void getOperationTxAndUpdateOperationDetails(QString addr, double rawAmount);

private:
    QString operation;
    QString chainID;
    /* operation data */
    nlohmann::json operationTx;

public:
    explicit GetAddressAmountWindow(QWidget *parent = nullptr);
    explicit GetAddressAmountWindow(QString operation, QString chainID, QString title, QWidget *parent = nullptr);
    ~GetAddressAmountWindow();

private slots:
    void on_btnPerformOperation_clicked();

    void on_leAddress_textChanged(const QString &arg1);

    void on_cbUnit_currentIndexChanged(int index);

    void on_dsbAmount_valueChanged(double arg1);

private:
    Ui::GetAddressAmountWindow *ui;
};

#endif // GETADDRESSAMOUNTWINDOW_H
