/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: newpaymentaddresswindow.h
 *
 * DESC: This is the head file of the payment-address item creating window,
 *       which creates a new address item and adds it to the payment-address.
 *
 * STRUCTURE:
 *      class NewPaymentAddressWindow
 */
#ifndef NEWPAYMENTADDRESSWINDOW_H
#define NEWPAYMENTADDRESSWINDOW_H

#include <QDialog>
#include <QMouseEvent>
#include <QMessageBox>

namespace Ui {
class NewPaymentAddressWindow;
}

class NewPaymentAddressWindow : public QDialog
{
    Q_OBJECT
public:
    QString tag;
    QString address;

// members for moving window
private:
    bool m_moving = false;
    QPoint m_lastPos;

public:
    explicit NewPaymentAddressWindow(QWidget *parent = nullptr);
    ~NewPaymentAddressWindow();

// methods for moving window
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::NewPaymentAddressWindow *ui;
};

#endif // NEWPAYMENTADDRESSWINDOW_H
