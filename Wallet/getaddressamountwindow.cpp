/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: getaddressamountwindow.cpp
 *
 * DESC: This is the cpp file of the 'address & amount' window.
 */
#include "getaddressamountwindow.h"
#include "ui_getaddressamountwindow.h"

GetAddressAmountWindow::GetAddressAmountWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetAddressAmountWindow)
{
    ui->setupUi(this);
}

GetAddressAmountWindow::GetAddressAmountWindow(QString operation, QString chainID, QString title, QWidget *parent) :
    QDialog(parent), operation(operation), chainID(chainID), ui(new Ui::GetAddressAmountWindow)
{
    ui->setupUi(this);
    // set widget attributes
    ui->btnPerformOperation->setDefault(true);
    this->setWindowTitle(title);
    // set window information
    if (operation == "Deposit")
    {
        ui->lblAddressType->setText(tr("SideChain Address"));
        ui->leAddress->setPlaceholderText(tr("Input Deposit Address..."));
        ui->lblAmountType->setText(tr("Deposit Amount"));
        ui->lblOperationToType->setText(tr("Deposit To:"));
    }
    else if (operation == "Withdraw")
    {
        ui->lblAddressType->setText(tr("MainChain Address"));
        ui->leAddress->setPlaceholderText(tr("Input Withdraw Address..."));
        ui->lblAmountType->setText(tr("Withdraw Amount"));
        ui->lblOperationToType->setText(tr("Withdraw To:"));
    }
    else
    {
        ui->lblAddressType->setText(tr("Address"));
        ui->leAddress->setPlaceholderText(tr("Input Address..."));
        ui->lblAmountType->setText(tr("Amount"));
    }
}

GetAddressAmountWindow::~GetAddressAmountWindow()
{
    delete ui;
}

void GetAddressAmountWindow::on_btnPerformOperation_clicked()
{
    ///////////////////////////
    // try perform operation //
    ///////////////////////////
    try
    {
        std::pair<int,std::string> result;
        /* verify payment-password */
        // get password input
        bool ok;
        QString password = QInputDialog::getText(nullptr, tr("Verify Operation"), tr("Input payment-password:"), QLineEdit::Password, "", &ok);

        // when cancel button clicked
        if (!ok)
            return;

        bool isPasswordMatch = currentWallet->VerifyPayPassword(password.toStdString());
        // verify wallet password
        if (!isPasswordMatch)
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Wrong password.\nPlease try again."));
            return;
        }
        /* perform operation */
        if (operation == "Deposit")
            result = depositToSideChain(operationTx, password.toStdString());
        else if (operation == "Withdraw")
            result = withdrawToMainChain(chainID.toStdString(), operationTx, password.toStdString());
        else
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Invalid Operation.\nOnly support 'Deposit' and 'Withdraw'."));
            return;
        }
        /* check operation result */
        if (result.first == ERRNO_APP)
        {
            QString errorMessage = QString::asprintf(
                        tr("Perform operation failed.\n\nError Message:\n%s").toStdString().c_str(),
                        result.second.c_str());
            QMessageBox::warning(nullptr, tr("Error"), errorMessage);
            return;
        }
    }
    catch (const std::exception &e)
    {
        QString errorMessage = QString::asprintf(
                    tr("Perform operation failed.\n\nError Message:\n%s").toStdString().c_str(),
                    e.what());
        QMessageBox::warning(nullptr, tr("Error"), errorMessage);
        return;
    }
    ///////////////////
    // end operation //
    ///////////////////

    // close window
    this->accept();
}

void GetAddressAmountWindow::on_leAddress_textChanged(const QString &addr)
{
    // clear error hint
    if (!addr.isEmpty())
    {
        ui->lblErrorHint->clear();
    }

    // update operation-tx & operation-details
    getOperationTxAndUpdateOperationDetails(addr, ui->dsbAmount->value());
}

void GetAddressAmountWindow::on_cbUnit_currentIndexChanged(int index)
{
    if (index == 0)
    {
        // ELA
        ui->dsbAmount->setDecimals(8);
        ui->dsbAmount->setMaximum(100);
    }
    else if (index == 1)
    {
        // sELA
        ui->dsbAmount->setDecimals(0);
        ui->dsbAmount->setMaximum(100 * SELA_PER_ELA);
    }
}

void GetAddressAmountWindow::getOperationTxAndUpdateOperationDetails(QString addr, double rawAmount)
{
    ///////////////////////////////////
    // try get operation transaction //
    ///////////////////////////////////
    if (operation == "Deposit")
    {
        IMainchainSubWallet *subWallet = dynamic_cast<IMainchainSubWallet *>(currentWallet->GetSubWallet(CHAINID_ELA));
        // calculate tranfer amount
        /* amount unit : 'sELA' */
        QString amount = "0";
        if (ui->cbUnit->currentText() == "ELA")
            amount = QString("%1").arg(static_cast<int>(rawAmount*SELA_PER_ELA));
        else if (ui->cbUnit->currentText() == "sELA")
            amount = QString("%1").arg(static_cast<int>(rawAmount));

        try
        {
            // get operation Tx
            operationTx = subWallet->CreateDepositTransaction(
                        "", chainID.toStdString(), amount.toStdString(), addr.toStdString(), "");
        }
        catch (const std::exception &e)
        {
            ui->btnPerformOperation->setEnabled(false);
            return;
        }
    }
    else if (operation == "Withdraw")
    {
        ISidechainSubWallet *subWallet = dynamic_cast<ISidechainSubWallet *>(currentWallet->GetSubWallet(chainID.toStdString()));
        // calculate tranfer amount
        /* amount unit : 'sELA' */
        QString amount = "0";
        if (ui->cbUnit->currentText() == "ELA")
            amount = QString("%1").arg(static_cast<int>(rawAmount*SELA_PER_ELA));
        else if (ui->cbUnit->currentText() == "sELA")
            amount = QString("%1").arg(static_cast<int>(rawAmount));

        try
        {
            // get operation Tx
            operationTx = subWallet->CreateWithdrawTransaction(
                        "", amount.toStdString(), addr.toStdString(), "");
        }
        catch (const std::exception &e)
        {
            ui->btnPerformOperation->setEnabled(false);
            return;
        }
    }
    ///////////////////////////////////
    // end get operation transaction //
    ///////////////////////////////////

    // check result
    if (operationTx == nullptr)
    {
        ui->btnPerformOperation->setEnabled(false);
        return;
    }
    else
        ui->btnPerformOperation->setEnabled(true);

    //////////////////////////////
    // update operation details //
    //////////////////////////////
    QString fromText;
    QString toText;
    if (operation == "Deposit")
    {
        /* From: */
        fromText = (currentWallet->GetID()+" : "+CHAINID_ELA).c_str();
        /* To: */
        toText  = (currentWallet->GetID()+" : "+chainID.toStdString()).c_str();
    }
    else if (operation == "Withdraw")
    {
        /* From: */
        fromText = (currentWallet->GetID()+" : "+chainID.toStdString()).c_str();
        /* To: */
        toText  = (currentWallet->GetID()+" : "+CHAINID_ELA).c_str();
    }
    /* Amount, Fee & Total Amount */
    int rawFee = operationTx["Fee"];
    QString feeText;

    QString amountUnit = ui->cbUnit->currentText();
    QString totalAmountText;
    if (amountUnit == "ELA")
    {
        feeText = QString::asprintf("%.8lf",static_cast<double>(rawFee)/SELA_PER_ELA)+" ELA";
        totalAmountText = QString::asprintf("%.8lf",rawAmount + static_cast<double>(rawFee)/SELA_PER_ELA)+" ELA";
    }
    else if (amountUnit == "sELA")
    {
        feeText = QString::asprintf("%d",rawFee)+" sELA";
        totalAmountText = QString::asprintf("%d",static_cast<int>(rawAmount) + rawFee)+" sELA";
    }
    /* set text */
    ui->lblOperationFromText->setText(fromText);
    ui->lblOperationToText->setText(toText);
    ui->lblOperationFeeText->setText(feeText);
    ui->lblOperationTotalAmountText->setText(totalAmountText);
    ////////////////////////////////
    // end update operation details //
    ////////////////////////////////
}

void GetAddressAmountWindow::on_dsbAmount_valueChanged(double rawAmount)
{
    // update operation-tx & operation-details
    getOperationTxAndUpdateOperationDetails(ui->leAddress->text(), rawAmount);
}
