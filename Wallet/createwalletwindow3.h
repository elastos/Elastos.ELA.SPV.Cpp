/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: createwalletwindow3.h
 *
 * DESC: This is the head file of the third wallet creating window.
 *
 *       There are total 4 windows to guide user to create a new wallet, and
 *       this is the 3rd window. It asks user to input the passphrase of the
 *       new wallet.
 *
 * STRUCTURE:
 *      class CreateWalletWindow3
 */
#ifndef CREATEWALLETWINDOW3_H
#define CREATEWALLETWINDOW3_H

#include <QDialog>

namespace Ui {
class CreateWalletWindow3;
}

class CreateWalletWindow3 : public QDialog
{
    Q_OBJECT
public:
    static QString passphrase;

private:
    bool hasPassphrase = false;
    bool hasConfirmed = false;

public:
    explicit CreateWalletWindow3(QWidget *parent = nullptr);
    ~CreateWalletWindow3();

private slots:
    void on_btnPrevious_clicked();

    void on_btnNext_clicked();

    void on_cbVisible_clicked(bool checked);

    void on_lePassphrase_textChanged(const QString &arg1);

    void on_leConfirm_textChanged(const QString &arg1);

private:
    Ui::CreateWalletWindow3 *ui;
};

#endif // CREATEWALLETWINDOW3_H
