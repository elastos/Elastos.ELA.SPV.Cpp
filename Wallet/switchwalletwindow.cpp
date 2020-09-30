/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: switchwalletwindow.cpp
 *
 * DESC: This is the cpp file of the wallet switching window.
 */
#include "switchwalletwindow.h"
#include "ui_switchwalletwindow.h"
#include "mainwindow.h"

SwitchWalletWindow::SwitchWalletWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SwitchWalletWindow)
{
    ui->setupUi(this);
    // set widget attributes
    ui->btnSwitch->setDefault(true);
    this->setWindowTitle(tr("Switch Wallet"));
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    // update wallet information
    updateWalletInformation();
}

SwitchWalletWindow::~SwitchWalletWindow()
{
    delete ui;
}

void SwitchWalletWindow::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}

void SwitchWalletWindow::on_btnSwitch_clicked()
{
    QString walletName = ui->cbCurrentWallet->currentText();
    std::pair<int,std::string> switchResult = switchWallet(walletName.toStdString());
    if (switchResult.first == ERRNO_APP)
    {
        QString errorHint = QString::asprintf(tr("Switch wallet failed.\n\nError Message:\n%s").toStdString().c_str(),
                                              switchResult.second.c_str());
        QMessageBox::warning(nullptr, tr("Error"), tr("Switch wallet failed."));
        return;
    }

    // emit signal
    emit switchWalletFinish();

    this->accept();
}

void SwitchWalletWindow::updateWalletInformation()
{
    ui->cbCurrentWallet->clear();

    // check current wallet
    if (!currentWallet)
        return;

    auto masterWalletIDs = manager->GetAllMasterWalletID();
    ui->lblWalletCount->setNum((int)masterWalletIDs.size());
    for (auto id : masterWalletIDs)
    {
        ui->cbCurrentWallet->addItem(id.c_str());
    }
    ui->cbCurrentWallet->setCurrentText(currentWallet->GetID().c_str());
}
