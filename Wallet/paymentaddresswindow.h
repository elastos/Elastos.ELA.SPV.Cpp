/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: paymentaddresswindow.h
 *
 * DESC: This is the head file of the payment-address window,
 *       which provides storage and search functions for address items.
 *
 *       The payment-address data can be writen to the registry.
 *
 *       Reading registry and filling data-table is finished in constructor.
 *       Writing registry and saving changes of data-table is finished in closeEvent(QCloseEvent).
 *
 * STRUCTURE:
 *      load registry.hpp
 *          -include "registry.hpp"
 *          -import global variables for registry settings
 *      class PaymentAddressWindow
 */
#ifndef PAYMENTADDRESSWINDOW_H
#define PAYMENTADDRESSWINDOW_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QTextStream>
#include <QTableWidgetItem>

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

namespace Ui {
class PaymentAddressWindow;
}

class PaymentAddressWindow : public QWidget
{
    Q_OBJECT
private:
    // constants for table-widget
    enum CellType { ctTag=1000, ctAddress };
    enum FieldColNum { colTag=0, colAddress };
    bool isAdding = false;
    bool isDeleting = false;

private:
    // export data
    bool exportPaymentAddressToCsvFile(const QString &filename);
    // import data
    bool importPaymentAddressFromCsvFile(const QString &filename);
    // creating a new tag-addr item
    void createNewItem(int rowNo, QString tag, QString address);

public:
    explicit PaymentAddressWindow(QWidget *parent = nullptr);
    ~PaymentAddressWindow();

protected:
    // delete ptr to window when close
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_btnNew_clicked();

    void on_btnExport_clicked();

    void on_btnDelete_clicked();

    void on_btnCopy_clicked();

    void on_btnImport_clicked();

    void on_leSearch_textChanged(const QString &arg1);

private:
    Ui::PaymentAddressWindow *ui;
};

#endif // PAYMENTADDRESSWINDOW_H
