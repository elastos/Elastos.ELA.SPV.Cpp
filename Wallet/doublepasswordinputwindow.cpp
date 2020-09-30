/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: doublepasswordinputwindow.cpp
 *
 * DESC: This is the cpp file of the 'double-password input' window.
 */
#include "doublepasswordinputwindow.h"
#include "ui_doublepasswordinputwindow.h"

DoublePasswordInputWindow::DoublePasswordInputWindow(QWidget *parent) :
    QDialog(parent), firstMin(0), secondMin(0),
    ui(new Ui::DoublePasswordInputWindow)
{
    ui->setupUi(this);
}

DoublePasswordInputWindow::DoublePasswordInputWindow(QString firstInputName, QString firstConfirmName, int firstMinInput,
                                                     QString secondInputName, QString secondConfirmName, int secondMinInput,
                                                     QWidget *parent) :
    QDialog(parent), firstMin(firstMinInput), secondMin(secondMinInput),
    ui(new Ui::DoublePasswordInputWindow)
{
    ui->setupUi(this);
    // set widget attributes
    ui->btnOK->setDefault(true);
    this->setWindowTitle(firstInputName + tr(" & ") + secondInputName);
    // set window information
    ui->lblFirstInputName->setText(firstInputName);
    ui->leFirstInput->setPlaceholderText(firstInputName);
    ui->lblFirstConfirmName->setText(firstConfirmName);
    ui->leFirstConfirm->setPlaceholderText(firstConfirmName);
    ui->lblFirstMessage->setText(tr("(") + firstInputName + tr(" length must not less than ") + QString::number(firstMin) + tr(")"));
    ui->lblSecondInputName->setText(secondInputName);
    ui->leSecondInput->setPlaceholderText(secondInputName);
    ui->lblSecondConfirmName->setText(secondConfirmName);
    ui->leSecondConfirm->setPlaceholderText(secondConfirmName);
    ui->lblSecondMessage->setText(tr("(") + secondInputName + tr(" length must not less than ") + QString::number(secondMin) + tr(")"));
}

DoublePasswordInputWindow::~DoublePasswordInputWindow()
{
    delete ui;
}

// enable(disable) the ok_button according to firstInput, firstConfirmed, secondInput and secondConfirmed;
void DoublePasswordInputWindow::on_leFirstInput_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->leFirstInput->text() == ui->leFirstConfirm->text())
            ui->lblFirstHint->clear();
        hasFirstInput = true;
        if (hasSecondInput && hasSecondConfirmed && ui->leSecondInput->text().length() >= secondMin
                && hasFirstConfirmed && ui->leFirstInput->text().length() >= firstMin)
            ui->btnOK->setEnabled(true);
    }
    else
    {
        hasFirstInput = false;
        ui->btnOK->setEnabled(false);
    }
}

void DoublePasswordInputWindow::on_leFirstConfirm_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->leFirstConfirm->text() == ui->leFirstInput->text())
            ui->lblFirstHint->clear();
        hasFirstConfirmed = true;
        if (hasSecondInput && hasSecondConfirmed && ui->leSecondInput->text().length() >= secondMin
                && hasFirstInput && ui->leFirstConfirm->text().length() >= firstMin)
            ui->btnOK->setEnabled(true);
    }
    else
    {
        hasFirstConfirmed = false;
        ui->btnOK->setEnabled(false);
    }
}

void DoublePasswordInputWindow::on_leSecondInput_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->leSecondInput->text() == ui->leSecondConfirm->text())
            ui->lblSecondHint->clear();
        hasSecondInput = true;
        if (hasFirstInput && hasFirstConfirmed && ui->leFirstInput->text().length() >= firstMin
                && hasSecondConfirmed && ui->leSecondInput->text().length() >= secondMin)
            ui->btnOK->setEnabled(true);
    }
    else
    {
        hasSecondInput = false;
        ui->btnOK->setEnabled(false);
    }
}

void DoublePasswordInputWindow::on_leSecondConfirm_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
    {
        if (ui->leSecondConfirm->text() == ui->leSecondInput->text())
            ui->lblSecondHint->clear();
        hasSecondConfirmed = true;
        if (hasFirstInput && hasFirstConfirmed && ui->leFirstInput->text().length() >= firstMin
                && hasSecondInput && ui->leSecondConfirm->text().length() >= secondMin)
            ui->btnOK->setEnabled(true);
    }
    else
    {
        hasSecondConfirmed = false;
        ui->btnOK->setEnabled(false);
    }
}

void DoublePasswordInputWindow::on_cbVisibleFirst_stateChanged(int checked)
{
    if (checked)
    {
        ui->leFirstInput->setEchoMode(QLineEdit::Normal);
        ui->leFirstConfirm->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->leFirstInput->setEchoMode(QLineEdit::Password);
        ui->leFirstConfirm->setEchoMode(QLineEdit::Password);
    }
}

void DoublePasswordInputWindow::on_cbVisibleSecond_stateChanged(int checked)
{
    if (checked)
    {
        ui->leSecondInput->setEchoMode(QLineEdit::Normal);
        ui->leSecondConfirm->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->leSecondInput->setEchoMode(QLineEdit::Password);
        ui->leSecondConfirm->setEchoMode(QLineEdit::Password);
    }
}

// clear all inputs
void DoublePasswordInputWindow::on_btnClearAll_clicked()
{
    ui->leFirstInput->clear();
    ui->leFirstConfirm->clear();
    ui->leSecondInput->clear();
    ui->leSecondConfirm->clear();
}

void DoublePasswordInputWindow::on_btnOK_clicked()
{
    // check consistent
    QString firstStr = ui->leFirstInput->text();
    QString firstCon = ui->leFirstConfirm->text();
    QString secondStr = ui->leSecondInput->text();
    QString secondCon = ui->leSecondConfirm->text();
    bool firstEqu = (firstStr == firstCon);
    bool secondEqu = (secondStr == secondCon);
    if (!firstEqu)
    {
        // invalid input hint
        ui->lblFirstHint->setStyleSheet("color:red");
        ui->lblFirstHint->setText(tr("The two inputs must be consistent."));
    }
    if (!secondEqu)
    {
        // invalid input hint
        ui->lblSecondHint->setStyleSheet("color:red");
        ui->lblSecondHint->setText(tr("The two inputs must be consistent."));
    }
    if (!(firstEqu && secondEqu))
        return;

    // emit signal
    emit inputFinish(firstStr, secondStr);

    this->accept();
}
