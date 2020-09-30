/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: newpaymentaddresswindow.cpp
 *
 * DESC: This is the cpp file of the payment-address item creating window.
 */
#include "newpaymentaddresswindow.h"
#include "ui_newpaymentaddresswindow.h"

NewPaymentAddressWindow::NewPaymentAddressWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewPaymentAddressWindow)
{
    ui->setupUi(this);
}

NewPaymentAddressWindow::~NewPaymentAddressWindow()
{
    delete ui;
}

// methods for moving windwo
void NewPaymentAddressWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_moving = true;
        m_lastPos = event->globalPos() - pos();
    }
}

void NewPaymentAddressWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_moving && (event->buttons() & Qt::LeftButton) &&
            (event->globalPos() - m_lastPos).manhattanLength() > QApplication::startDragDistance())
    {
        move(event->globalPos() - m_lastPos);
        m_lastPos = event->globalPos() - pos();
    }
}

void NewPaymentAddressWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_moving = false;
}

void NewPaymentAddressWindow::on_buttonBox_accepted()
{
    tag = ui->leTag->text().trimmed();
    address = ui->leAddress->text().trimmed();
    if (tag.isEmpty() || address.isEmpty())
    {
        ui->lblHint->setStyleSheet("color:red");
        ui->lblHint->setText(tr("Tag and Address must not be empty"));
    }
    else
        accept();
}
