/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: createwalletwindow1.cpp
 *
 * DESC: This is the cpp file of the first wallet creating window.
 */
#include "createwalletwindow1.h"
#include "ui_createwalletwindow1.h"
#include "mainwindow.h"
#include "createwalletwindow2.h"

// static members
QString CreateWalletWindow1::walletName;
CreateWalletWindow1::MNEMONIC_LANG CreateWalletWindow1::mnemonicLanguage;
QString CreateWalletWindow1::languageText[5];
int CreateWalletWindow1::wordCount;
bool CreateWalletWindow1::isBtnNextClicked;

CreateWalletWindow1::CreateWalletWindow1(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateWalletWindow1)
{
    // initialize languageText
    languageText[(int)Mnemonic_English] = "English";
    languageText[(int)Mnemonic_Chinese] = "Chinese";
    languageText[(int)Mnemonic_Japanese] = "Japanese";
    languageText[(int)Mnemonic_French] = "French";
    languageText[(int)Mnemonic_Spanish] = "Spanish";
    ui->setupUi(this);
    // set widget attributes
    ui->btnNext->setDefault(true);
    this->setWindowTitle(tr("Create Wallet"));
}

CreateWalletWindow1::~CreateWalletWindow1()
{
    delete ui;
}

void CreateWalletWindow1::on_btnNext_clicked()
{
    // get input data
    walletName = ui->leWalletName->text();
    mnemonicLanguage = static_cast<MNEMONIC_LANG>(ui->cbMnemonicLang->currentIndex());
    wordCount = ui->cbMnemonicLen->currentText().toInt();
    isBtnNextClicked = true;
    // transfer to cwWindow2
    if (MainWindow::cwWindow2 == nullptr)
        MainWindow::cwWindow2 = new CreateWalletWindow2();
    this->hide();
    MainWindow::cwWindow2->show();
}

// enable(disable) the next_button according to the inputted wallet-name
void CreateWalletWindow1::on_leWalletName_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty())
        ui->btnNext->setEnabled(true);
    else
        ui->btnNext->setEnabled(false);
}
