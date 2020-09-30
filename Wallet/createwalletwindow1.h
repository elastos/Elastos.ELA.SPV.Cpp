/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: createwalletwindow1.h
 *
 * DESC: This is the head file of the first wallet creating window.
 *
 *       There are total 4 windows to guide user to create a new wallet, and
 *       this is the 1st window. It gets the wallet name, mnemonic language
 *       and mnemonic length from user.
 *
 * STRUCTURE:
 *      class CreateWalletWindow1
 */
#ifndef CREATEWALLETWINDOW1_H
#define CREATEWALLETWINDOW1_H

#include <QDialog>

namespace Ui {
class CreateWalletWindow1;
}

class CreateWalletWindow1 : public QDialog
{
    Q_OBJECT
public:
    static QString walletName;
    static enum MNEMONIC_LANG { Mnemonic_English=0,
                                Mnemonic_Chinese, Mnemonic_Japanese,
                                Mnemonic_French, Mnemonic_Spanish } mnemonicLanguage;
    static QString languageText[5];
    static int wordCount;
    static bool isBtnNextClicked;

public:
    explicit CreateWalletWindow1(QWidget *parent = nullptr);
    ~CreateWalletWindow1();

private slots:
    void on_btnNext_clicked();

    void on_leWalletName_textChanged(const QString &arg1);

private:
    Ui::CreateWalletWindow1 *ui;
};

#endif // CREATEWALLETWINDOW1_H
