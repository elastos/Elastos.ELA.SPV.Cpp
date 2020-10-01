/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: mainwindow.h
 *
 * DESC: This is the head file of the main window, which contains
 *       a stack-widget containing 5 different interfaces. This window
 *       is the main component of the application.
 *
 *       It holds all sub-windows' as the static members to keep that only
 *       one specific sub-window exists at the same time. When create a new
 *       sub-window, it first check that whether the pointer to the sub-window
 *       is nullptr, and then decide whether to create a new one or not.
 *
 *       It also holds the status bar's widgets, as those widgets only can be
 *       initialized in code after other components are initalized.
 *
 *       The main gui-update logic is implemented with a set of expandable
 *       slot functions, with the main function called 'updateWalletInformation'.
 *       The 'updateWalletInformation' invoke smaller functions in order to update
 *       wallet-information-related gui. It can be expanded easily as you can just
 *       write a new update-function independently, add it to the set of slot
 *       functions and invoke it in 'updateWalletInformation' in the right place.
 *
 *       Attention that some gui-update is not independent and maybe A-widget's update
 *       is based on B-widget's update information. So, before add any update functions
 *       it is better to check if other gui-update is based on it.
 *
 *       A special class member 'transferTx' is similar to 'operationTx' in GetAddressAmountWindow.
 *       It stores the transfer transaction information and can be used by the true processing function.
 *
 *       Other slot functions are used to receive other sub-windows' signals for updating gui-information
 *       or getting user input. There is also a signal function called 'updateWalletSignal' to broadcast
 *       the updating message to all of the needed sub-windows.
 *
 * STRUCTURE:
 *      load registry.hpp
 *          -include "registry.hpp"
 *          -import global variables for registry settings
 *      load wallet_core.hpp
 *          -include "wallet_core.hpp"
 *          -import global variables for wallet core functions
 *      class MainWindow
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "createwalletwindow1.h"
#include "createwalletwindow2.h"
#include "createwalletwindow3.h"
#include "createwalletwindow4.h"
#include "paymentaddresswindow.h"
#include "qrcodewindow.h"
#include "switchwalletwindow.h"
#include "removewalletwindow.h"
#include "changepasswordwindow.h"
#include "doublepasswordinputwindow.h"
#include "getaddressamountwindow.h"

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include <QSettings>
#include <QApplication>
#include <QDateTime>
#include <QToolButton>
#include <QProgressBar>
#include <QLabel>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QTimer>

#include <iostream>
#include <limits>

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
extern QString curSubWalletChainID;
extern bool isSubWalletChainIDUpdate;
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
extern bool verboseMode;
extern const char *SplitLine;
extern MasterWalletData masterWalletData;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    bool isAppStart = false;
    // static windows
    static CreateWalletWindow1 *cwWindow1;
    static CreateWalletWindow2 *cwWindow2;
    static CreateWalletWindow3 *cwWindow3;
    static CreateWalletWindow4 *cwWindow4;
    static PaymentAddressWindow *paWindow;
    static QRcodeWindow *qrcWindow;
    static SwitchWalletWindow *swWindow;
    static RemoveWalletWindow *rwWindow;
    static ChangePasswordWindow *cpWindow;
    // widget items
    /* status bar widgets */
    QToolButton *btnChangeSyncStatus;
    QLabel *lblSubWalletChainID;
    QLabel *lblSyncStatus;
    QProgressBar *pbSyncProgress;
    QLabel *lblSyncProgress;
    QLabel *lblLastBlockTime;
    QLabel *lblSyncErrorHint;
    // application timer for update
    QTimer *timerRecentTransactionRecordsAutoUpdate;
    QTimer *timerStatusBarAutoUpdate;
    QTimer *timerTransactionRecordsAutoUpdate;
    // transaction data
    nlohmann::json transferTx;

private:
    QString importDataFirstInput;
    QString importDataSecondInput;
    bool walletInformationUpdating = false;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // check current wallet status
    bool checkCurrentWalletStatus();

    // try get recent-transaction records
    std::pair<int,std::string> getRecentTransactionRecords(QStringList &timeList,
                                                           QStringList &typeList,
                                                           QStringList &amountList);

    // try get transaction records
    std::pair<int,std::string> getTransactionRecords(QStringList &timeList, QStringList &confirmedBlockList,
                                                     QStringList &statusList, QStringList &typeList,
                                                     QStringList &amountList, QStringList &txIDList);
    // filter transaction records
    void filterTransactionRecords(QStringList &timeList, QStringList &confirmedBlockList,
                                  QStringList &statusList, QStringList &typeList,
                                  QStringList &amountList, QStringList &txIDList);
    // remove index 'i' from passed lists
    void removeSpecificRecords(int i, QStringList &timeList, QStringList &confirmedBlockList,
                               QStringList &statusList, QStringList &typeList,
                               QStringList &amountList, QStringList &txIDList);

    // export data
    bool exportRecordTableToCsvFile(const QString &filename);
    void exportWalletToMnemonic(const QString &filename);
    void exportWalletToKeystore(const QString &filename);

    // import data
    void importWalletFromMnemonic(const QString &filename);
    void importWalletFromKeystore(const QString &filename);

    // set transfer-transaction & update billing-details
    void setTransferTxAndUpdateBillingDetails(QString addr, double rawAmount);

signals:
    void updateWalletSignal();

private slots:
    /* SLOT for wallet information's gui update */
    void updateWalletInformation();
    void updateSwitchWalletMenuInformation();
    void updateSwitchSubWalletMenuInformation();
    void updateQuickLookInformation();
    void updateOperationButtonInformation();
    void updateRecentRecordTableInformation();
    void updateStatusBarInformation();
    void updateTransferBillingDetails();
    void updateRecordTableInformation();
    void updateNetworkInformation();

    /* SLOT for DoublePasswordInputWindow's input-finish */
    void updateImportData(const QString &firstInput, const QString &secondInput);

    /* SLOT for switch wallet action */
    void switchCurrentWallet();

    /* SLOT for switch sub-wallet action */
    void subWalletStatusChanged(bool checked);

    /* SLOT for sync (start / stop) action */
    void syncStatusChanged(bool checked);

private slots:
    void on_actionCreate_Wallet_triggered();

    void on_actionPayment_Address_triggered();

    void on_actionHome_triggered();

    void on_actionTransfer_triggered();

    void on_actionReceiving_triggered();

    void on_actionTransaction_Record_triggered();

    void on_btnShowRW_clicked();

    void on_btnOpenPA_clicked();

    void on_btnClear_clicked();

    void on_btnExport_clicked();

    void on_actionPreferences_triggered();

    void on_leSearch_textChanged(const QString &arg1);

    void on_cbLang_currentIndexChanged(const QString &arg1);

    void on_actionSwitch_Wallet_Window_triggered();

    void on_actionRemove_Wallet_triggered();

    void on_actionChange_Wallet_Password_triggered();

    void on_actionExport_Mnemonic_Words_triggered();

    void on_actionExport_Keystore_triggered();

    void on_actionImport_Mnemonic_Words_triggered();

    void on_actionImport_Keystore_triggered();

    void on_cbNetwork_currentTextChanged(const QString &arg1);

    void on_btnChooseConfigFile_clicked();

    void on_cbSubWalletChainID_currentTextChanged(const QString &arg1);

    void on_btnCloseSubWalletChainID_clicked();

    void on_btnSend_clicked();

    void on_leAddress_textChanged(const QString &arg1);

    void on_btnDeposit_clicked();

    void on_btnWithdraw_clicked();

    void on_pushButton_clicked();

    void on_cbCoinbase_stateChanged(int arg1);

    void on_btnUpdateTransactionInformation_clicked();

    void on_cbTime_currentIndexChanged(int index);

    void on_cbStatus_currentIndexChanged(int index);

    void on_cbType_currentIndexChanged(int index);

    void on_dsbAmountForFilter_valueChanged(double arg1);

    void on_cbUnitForSearch_currentIndexChanged(int index);

    void on_cbUnitForTransfer_currentIndexChanged(int index);

    void on_cbTransactionRecordAutoUpdate_stateChanged(int arg1);

    void on_sbDataUpdateInterval_valueChanged(int arg1);

    void on_dsbAmountForTransfer_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;
};

/* Keep original foreground text color when item selected */
class KeepForegroundItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit KeepForegroundItemDelegate() { }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem viewOption(option);
        viewOption.palette.setColor(QPalette::HighlightedText, index.data(Qt::ForegroundRole).value<QColor>());
        QItemDelegate::paint(painter, viewOption, index);
    }
};

#endif // MAINWINDOW_H
