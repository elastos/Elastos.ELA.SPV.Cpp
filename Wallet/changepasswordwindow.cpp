/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: changepasswordwindow.cpp
 *
 * DESC: This is the cpp file of the password changing window.
 */
#include "changepasswordwindow.h"
#include "ui_changepasswordwindow.h"
#include "mainwindow.h"

ChangePasswordWindow::ChangePasswordWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePasswordWindow)
{
    ui->setupUi(this);
    // set widget attributes
    ui->btnOK->setDefault(true);
    this->setWindowTitle(tr("Change Password"));
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
}

void ChangePasswordWindow::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}

ChangePasswordWindow::~ChangePasswordWindow()
{
    delete ui;
}

// enable(disable) the OK_button according to leNewPassword and leConfirm
void ChangePasswordWindow::on_leNewPassword_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->leNewPassword->text() == ui->leConfirm->text())
            ui->lblHint->clear();
        hasPassword = true;
        if (hasConfirmed && ui->leNewPassword->text().length() >= 8)
            ui->btnOK->setEnabled(true);
    }
    else
    {
        hasPassword = false;
        ui->btnOK->setEnabled(false);
    }
}

void ChangePasswordWindow::on_leConfirm_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->leNewPassword->text() == ui->leConfirm->text())
            ui->lblHint->clear();
        hasConfirmed = true;
        if (hasPassword && ui->leNewPassword->text().length() >= 8)
            ui->btnOK->setEnabled(true);
    }
    else
    {
        hasConfirmed = false;
        ui->btnOK->setEnabled(false);
    }
}

void ChangePasswordWindow::on_cbVisible_stateChanged(int checked)
{
    if (checked)
    {
        ui->leOldPassword->setEchoMode(QLineEdit::Normal);
        ui->leNewPassword->setEchoMode(QLineEdit::Normal);
        ui->leConfirm->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->leOldPassword->setEchoMode(QLineEdit::Password);
        ui->leNewPassword->setEchoMode(QLineEdit::Password);
        ui->leConfirm->setEchoMode(QLineEdit::Password);
    }
}

// clear all inputs
void ChangePasswordWindow::on_btnClearAll_clicked()
{
    ui->leOldPassword->clear();
    ui->leNewPassword->clear();
    ui->leConfirm->clear();
}

void ChangePasswordWindow::on_btnOK_clicked()
{
    // check current wallet
    if (!currentWallet)
        return;

    bool isOldPasswordMatch = false;
    try
    {
        QString firstStr = ui->leNewPassword->text();
        QString secondStr = ui->leConfirm->text();
        // check new password
        if (firstStr != secondStr)
        {
            ui->lblHint->setStyleSheet("color:red");
            ui->lblHint->setText(tr("The two inputs must be consistent."));
            return;
        }

        QString oldPassword = ui->leOldPassword->text();
        QString newPassword = firstStr;

        isOldPasswordMatch = currentWallet->VerifyPayPassword(oldPassword.toStdString());
        // check old password
        if (!isOldPasswordMatch)
        {
            ui->lblWrongOldPasswordHint->setStyleSheet("color:red");
            ui->lblWrongOldPasswordHint->setText(tr("Wrong old password."));
            return;
        }

        // change password
        currentWallet->ChangePassword(oldPassword.toStdString(), newPassword.toStdString());

        // close window
        this->accept();
    }
    catch (const std::exception &e)
    {
        if (!isOldPasswordMatch)
        {
            ui->lblWrongOldPasswordHint->setStyleSheet("color:red");
            ui->lblWrongOldPasswordHint->setText(tr("Wrong old password."));
        }
        else
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Change password failed."));
        }
    }
}
