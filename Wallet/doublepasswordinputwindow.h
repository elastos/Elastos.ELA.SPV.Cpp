/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: doublepasswordinputwindow
 *
 * DESC: This is the head file of the 'double-password input' window.
 *
 *       It provides a special constructor that change the labels' text
 *       and line-editors' minimum input count according to 'firstInputName',
 *       'firstConfirmName', 'firstMinInput', 'secondInputName', 'secondConfirmName'
 *       and 'secondMinInput'.
 *
 *       It can be used if you need to get two independent inputs from user, and
 *       those inputs have requirement on input count, such as password and passphrase.
 *
 *      After input finished, you can emit a signal called 'inputFinish(QString, QString)'
 *      to pass the user inputs to the invoker widget. This requires that you connect
 *      a slot function to receive the 'inputFinish' signal when this window is initialized.
 *
 * STRUCTURE:
 *      class DoublePasswordInputWindow
 */
#ifndef DOUBLEPASSWORDINPUTWINDOW_H
#define DOUBLEPASSWORDINPUTWINDOW_H

#include <QDialog>

namespace Ui {
class DoublePasswordInputWindow;
}

class DoublePasswordInputWindow : public QDialog
{
    Q_OBJECT
private:
    bool hasFirstInput = false;
    bool hasFirstConfirmed = false;
    bool hasSecondInput = false;
    bool hasSecondConfirmed = false;
    const int firstMin;
    const int secondMin;

public:
    explicit DoublePasswordInputWindow(QWidget *parent = nullptr);
    explicit DoublePasswordInputWindow(QString firstInputName, QString firstConfirmName, int firstMinInput,
                                       QString secondInputName, QString secondConfirmName, int secondMinInput,
                                       QWidget *parent = nullptr);
    ~DoublePasswordInputWindow();

signals:
    void inputFinish(QString, QString);

private slots:
    void on_leFirstInput_textChanged(const QString &arg1);

    void on_leFirstConfirm_textChanged(const QString &arg1);

    void on_leSecondInput_textChanged(const QString &arg1);

    void on_leSecondConfirm_textChanged(const QString &arg1);

    void on_cbVisibleFirst_stateChanged(int arg1);

    void on_cbVisibleSecond_stateChanged(int arg1);

    void on_btnClearAll_clicked();

    void on_btnOK_clicked();

private:
    Ui::DoublePasswordInputWindow *ui;
};

#endif // DOUBLEPASSWORDINPUTWINDOW_H
