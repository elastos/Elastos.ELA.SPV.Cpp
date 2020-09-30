/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: createwalletwindow4.cpp
 *
 * DESC: This is the cpp file of the fourth wallet creating window.
 */
#include "createwalletwindow4.h"
#include "ui_createwalletwindow4.h"
#include "mainwindow.h"
#include "createwalletwindow1.h"
#include "createwalletwindow2.h"
#include "createwalletwindow3.h"

// static members
QString CreateWalletWindow4::password;

CreateWalletWindow4::CreateWalletWindow4(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateWalletWindow4)
{
    ui->setupUi(this);
    // set widget attributes
    ui->btnCreate->setDefault(true);
    this->setWindowTitle(tr("Create Wallet"));
}

CreateWalletWindow4::~CreateWalletWindow4()
{
    delete ui;
}

void CreateWalletWindow4::on_btnPrevious_clicked()
{
    if (MainWindow::cwWindow3 == nullptr)
        MainWindow::cwWindow3 = new CreateWalletWindow3();
    this->hide();
    MainWindow::cwWindow3->show();
}

void CreateWalletWindow4::on_cbVisible_clicked(bool checked)
{
    if (checked)
    {
        ui->lePassword->setEchoMode(QLineEdit::Normal);
        ui->leConfirm->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->lePassword->setEchoMode(QLineEdit::Password);
        ui->leConfirm->setEchoMode(QLineEdit::Password);
    }
}

// enable(disable) the create_button according to lePassword and leConfirm
void CreateWalletWindow4::on_lePassword_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->lePassword->text() == ui->leConfirm->text())
            ui->lblHint->clear();
        hasPassword = true;
        if (hasConfirmed && ui->lePassword->text().length() >= 8)
            ui->btnCreate->setEnabled(true);
    }
    else
    {
        hasPassword = false;
        ui->btnCreate->setEnabled(false);
    }
}

void CreateWalletWindow4::on_leConfirm_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->lePassword->text() == ui->leConfirm->text())
            ui->lblHint->clear();
        hasConfirmed = true;
        if (hasPassword && ui->lePassword->text().length() >= 8)
            ui->btnCreate->setEnabled(true);
    }
    else
    {
        hasConfirmed = false;
        ui->btnCreate->setEnabled(false);
    }
}

// create new wallet
void CreateWalletWindow4::on_btnCreate_clicked()
{
    QString firstStr = ui->lePassword->text();
    QString secondStr = ui->leConfirm->text();
    if (firstStr != secondStr)
    {
        ui->lblHint->setStyleSheet("color:red");
        ui->lblHint->setText(tr("The two inputs must be consistent."));
        return;
    }
    // set password
    password = firstStr;

    // create wallet
    createWallet();

    // release createWallet windows
    MainWindow::cwWindow1 = nullptr;
    MainWindow::cwWindow2 = nullptr;
    MainWindow::cwWindow3 = nullptr;
    MainWindow::cwWindow4 = nullptr;
    this->accept();
}

void CreateWalletWindow4::createWallet()
{
    try
    {
        // create masterWallet & subWallet
        IMasterWallet *masterWallet = manager->CreateMasterWallet(
                    CreateWalletWindow1::walletName.toStdString(),
                    CreateWalletWindow2::mnemonic.toStdString(),
                    CreateWalletWindow3::passphrase.toStdString(),
                    CreateWalletWindow4::password.toStdString(), false);

        if (!masterWallet)
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Create master wallet failed."));
            return;
        }

        ISubWallet *subWallet = masterWallet->CreateSubWallet(CHAINID_ELA);
        if (!subWallet)
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Create main chain wallet failed."));
            return;
        }

        subWalletOpen(masterWallet, subWallet);

        currentWallet = masterWallet;

        // emit signal
        emit createWalletFinish();
    }
    catch (const std::exception &e)
    {
        QMessageBox::warning(nullptr, tr("Error"), tr("Create new wallet failed."));
    }
}
