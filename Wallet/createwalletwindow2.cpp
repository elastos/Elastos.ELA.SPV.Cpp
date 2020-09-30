/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: createwalletwindow2.cpp
 *
 * DESC: This is the cpp file of the second wallet creating window.
 */
#include "createwalletwindow2.h"
#include "ui_createwalletwindow2.h"
#include "mainwindow.h"
#include "createwalletwindow1.h"
#include "createwalletwindow3.h"

// static members
QString CreateWalletWindow2::mnemonic;
// mnemonic generated in showEvent

CreateWalletWindow2::CreateWalletWindow2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateWalletWindow2)
{
    ui->setupUi(this);
    // set widget attributes
    ui->btnNext->setDefault(true);
    this->setWindowTitle(tr("Create Wallet"));
}

CreateWalletWindow2::~CreateWalletWindow2()
{
    delete ui;
}

void CreateWalletWindow2::on_btnPrevious_clicked()
{
    if (MainWindow::cwWindow1 == nullptr)
        MainWindow::cwWindow1 = new CreateWalletWindow1();
    this->hide();
    MainWindow::cwWindow1->show();
}

void CreateWalletWindow2::on_btnNext_clicked()
{
    if (MainWindow::cwWindow3 == nullptr)
        MainWindow::cwWindow3 = new CreateWalletWindow3();
    this->hide();
    MainWindow::cwWindow3->show();
}

void CreateWalletWindow2::on_btnSaveTo_clicked()
{
    QString dlgTitle = tr("Save Mnemonic Words To");
    QString filter = tr("Text File(*.txt);;All File(*.*)");
    QString filename = QFileDialog::getSaveFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    if (!saveTextFile(filename))
        QMessageBox::warning(nullptr, tr("Warning"), tr("Unable to save.\nPlease check relevant permissions."));
}

bool CreateWalletWindow2::saveTextFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return false;
    QString str = ui->pteMnemonicWords->toPlainText();
    QByteArray strBytes = str.toUtf8();
    file.write(strBytes);
    file.close();
    return true;
}

void CreateWalletWindow2::on_cbConfirm_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->btnNext->setEnabled(true);
    }
    else
    {
        ui->btnNext->setEnabled(false);
    }
}

void CreateWalletWindow2::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (CreateWalletWindow1::isBtnNextClicked)
    {
        // generate mnemonic words
        QString language = CreateWalletWindow1::languageText[static_cast<int>(CreateWalletWindow1::mnemonicLanguage)];
        std::string lang_tmp = manager->GenerateMnemonic(language.toStdString(), CreateWalletWindow1::wordCount);
        mnemonic = QString(lang_tmp.c_str());
        // show mnemonic words
        ui->pteMnemonicWords->setPlainText(mnemonic);
    }
    CreateWalletWindow1::isBtnNextClicked = false;
}
