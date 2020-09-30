/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: removewalletwindow.cpp
 *
 * DESC: This is the cpp file of the wallet removing window.
 */
#include "removewalletwindow.h"
#include "ui_removewalletwindow.h"
#include "mainwindow.h"

RemoveWalletWindow::RemoveWalletWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoveWalletWindow)
{
    ui->setupUi(this);
    // set widget attributes
    ui->btnRemove->setDefault(true);
    this->setWindowTitle(tr("Remove Wallet"));
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    // update wallet information
    updateWalletInformation();
}

RemoveWalletWindow::~RemoveWalletWindow()
{
    delete ui;
}

void RemoveWalletWindow::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}

void RemoveWalletWindow::on_btnClearSelected_clicked()
{
    // update AvailableWallets
    ui->lwAvailableWallets->clear();
    auto masterWalletIDs = manager->GetAllMasterWalletID();
    for (auto id : masterWalletIDs)
    {
        ui->lwAvailableWallets->addItem(id.c_str());
    }

    // update ToBeRemoved
    ui->lwToBeRemoved->clear();
}

void RemoveWalletWindow::on_tbAdd_clicked()
{
    bool isPasswordMatch = false;
    try
    {
        QListWidgetItem *item = ui->lwAvailableWallets->currentItem();
        if (item == nullptr)
            return;
        QString walletName = item->text();
        auto masterWallet = manager->GetMasterWallet(walletName.toStdString());

        // get password input
        QString hint = QString::asprintf(tr("Password for wallet '%s'").toStdString().c_str(),
                                         walletName.toStdString().c_str());
        bool ok;
        QString password = QInputDialog::getText(nullptr, tr("Input Password"), hint, QLineEdit::Password, "", &ok);

        // when cancel button clicked
        if (!ok)
            return;

        isPasswordMatch = masterWallet->VerifyPayPassword(password.toStdString());
        // verify wallet password
        if (!isPasswordMatch)
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Wrong password.\nPlease try again."));
            return;
        }

        // move item from Available to ToBeRemoved
        ui->lwAvailableWallets->removeItemWidget(item);
        ui->lwToBeRemoved->addItem(item->text());

        // release item
        delete item;

        // default first row selected
        if (ui->lwToBeRemoved->count() == 1)
        {
            ui->lwToBeRemoved->setCurrentRow(0);
        }
    }
    catch (const std::exception &e)
    {
        if (!isPasswordMatch)
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Wrong password.\nPlease try again."));
        }
        else
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Add wallet failed."));
        }
    }
}

void RemoveWalletWindow::on_tbDelete_clicked()
{
    // move item from ToBeRemoved to Available
    QListWidgetItem *item = ui->lwToBeRemoved->currentItem();
    if (item == nullptr)
        return;
    ui->lwToBeRemoved->removeItemWidget(item);
    ui->lwAvailableWallets->addItem(item->text());
    delete item;
}

void RemoveWalletWindow::on_btnRemove_clicked()
{
    QListWidgetItem *item;
    std::pair<int,std::string> result;
    int removedCount = 0;
    int failedCount = 0;
    for (int i = 0; i < ui->lwToBeRemoved->count(); ++i)
    {
        item = ui->lwToBeRemoved->item(i);
        result = removeWallet(item->text().toStdString());
        if (result.first == ERRNO_APP)
        {
            item->setBackground(QColor(220, 0, 0, 100));
            ui->lblRemoveWalletFailedHint->setStyleSheet("color:red;");
            ui->lblRemoveWalletFailedHint->setText(result.second.c_str());
            ++failedCount;
        }
        else
        {
            ui->lwToBeRemoved->removeItemWidget(item);
            delete item;
            ++removedCount;
        }
    }
    ui->lblRemovedCount->setNum(removedCount);
    ui->lblFailedCount->setNum(failedCount);

    if (failedCount == 0)
        ui->lblRemoveWalletFailedHint->clear();

    // emit signal
    emit removeWalletFinish();
}

void RemoveWalletWindow::updateWalletInformation()
{
    ui->lwAvailableWallets->clear();
    ui->lwToBeRemoved->clear();
    auto masterWalletIDs = manager->GetAllMasterWalletID();
    for (auto id : masterWalletIDs)
    {
        ui->lwAvailableWallets->addItem(id.c_str());
    }
    // default first row selected
    if (ui->lwAvailableWallets->count() > 0)
    {
        ui->lwAvailableWallets->setCurrentRow(0);
    }
}
