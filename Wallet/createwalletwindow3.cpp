/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: createwalletwindow3.cpp
 *
 * DESC: This is the cpp file of the third wallet creating window.
 */
#include "createwalletwindow3.h"
#include "ui_createwalletwindow3.h"
#include "mainwindow.h"
#include "createwalletwindow2.h"
#include "createwalletwindow4.h"

// static members
QString CreateWalletWindow3::passphrase;

CreateWalletWindow3::CreateWalletWindow3(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateWalletWindow3)
{
    ui->setupUi(this);
    // set widget attributes
    ui->btnNext->setDefault(true);
    this->setWindowTitle(tr("Create Wallet"));
}

CreateWalletWindow3::~CreateWalletWindow3()
{
    delete ui;
}

void CreateWalletWindow3::on_btnPrevious_clicked()
{
    if (MainWindow::cwWindow2 == nullptr)
        MainWindow::cwWindow2 = new CreateWalletWindow2();
    this->hide();
    MainWindow::cwWindow2->show();
}

void CreateWalletWindow3::on_btnNext_clicked()
{
    QString firstStr = ui->lePassphrase->text();
    QString secondStr = ui->leConfirm->text();
    if (firstStr != secondStr)
    {
        // invalid input hint
        ui->lblHint->setStyleSheet("color:red");
        ui->lblHint->setText(tr("The two inputs must be consistent."));
        return;
    }
    // set passphrase
    passphrase = firstStr;
    if (MainWindow::cwWindow4 == nullptr)
        MainWindow::cwWindow4 = new CreateWalletWindow4();
    this->hide();
    MainWindow::cwWindow4->show();
}

void CreateWalletWindow3::on_cbVisible_clicked(bool checked)
{
    if (checked)
    {
        ui->lePassphrase->setEchoMode(QLineEdit::Normal);
        ui->leConfirm->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->lePassphrase->setEchoMode(QLineEdit::Password);
        ui->leConfirm->setEchoMode(QLineEdit::Password);
    }
}

// enable(disable) the next_button according to lePassphrase and leConfirm
void CreateWalletWindow3::on_lePassphrase_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->lePassphrase->text() == ui->leConfirm->text())
            ui->lblHint->clear();
        hasPassphrase = true;
        if (hasConfirmed && ui->lePassphrase->text().length() >= 8)
            ui->btnNext->setEnabled(true);
    }
    else
    {
        hasPassphrase = false;
        ui->btnNext->setEnabled(false);
    }
}

void CreateWalletWindow3::on_leConfirm_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->lePassphrase->text() == ui->leConfirm->text())
            ui->lblHint->clear();
        hasConfirmed = true;
        if (hasPassphrase && ui->leConfirm->text().length() >= 8)
            ui->btnNext->setEnabled(true);
    }
    else
    {
        hasConfirmed = false;
        ui->btnNext->setEnabled(false);
    }
}
