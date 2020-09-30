/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: paymentaddresswindow.cpp
 *
 * DESC: This is the cpp file of the payment-address window.
 */
#include "paymentaddresswindow.h"
#include "ui_paymentaddresswindow.h"
#include "newpaymentaddresswindow.h"
#include "mainwindow.h"

PaymentAddressWindow::PaymentAddressWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaymentAddressWindow)
{
    ui->setupUi(this);
    // set widget attributes
    ui->twAddress->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->twAddress->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->twAddress->setStyleSheet("selection-background-color:lightgray");
    ui->twAddress->setItemDelegate(new KeepForegroundItemDelegate());
    this->setWindowTitle(tr("Payment Address"));
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    // update payment-address data
    while (ui->twAddress->rowCount() != 0)
        ui->twAddress->removeRow(0);
    int row = 0;
    for (QString addr : paymentAddressData.keys())
    {
        ui->twAddress->insertRow(row);
        // set tag
        QString tag = paymentAddressData[addr];
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignCenter);
        item->setText(tag);
        ui->twAddress->setItem(row, PaymentAddressWindow::colTag, item);
        // set address
        QTableWidgetItem *anotherItem = new QTableWidgetItem();
        anotherItem->setTextAlignment(Qt::AlignCenter);
        anotherItem->setText(addr);
        ui->twAddress->setItem(row, PaymentAddressWindow::colAddress, anotherItem);
        ++row;
    }
}

PaymentAddressWindow::~PaymentAddressWindow()
{
    delete ui;
}

void PaymentAddressWindow::on_btnNew_clicked()
{
    NewPaymentAddressWindow *npaWindow = new NewPaymentAddressWindow();
    npaWindow->setWindowFlag(Qt::FramelessWindowHint);
    if (npaWindow->exec() == QDialog::Accepted)
        createNewItem(ui->twAddress->rowCount(), npaWindow->tag, npaWindow->address);
}

// delete ptr to window when close
void PaymentAddressWindow::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);

    paymentAddressData.clear();
    // write to registry
    for (int i = 0; i < ui->twAddress->rowCount(); ++i)
    {
        QTableWidgetItem *tagItem = ui->twAddress->item(i, PaymentAddressWindow::colTag);
        QTableWidgetItem *addrItem = ui->twAddress->item(i, PaymentAddressWindow::colAddress);
        QString tag = tagItem->text();
        QString addr = addrItem->text();
        paymentAddressData.insert(addr, tag);
    }
    dataMapFromQStringToQVariant(paymentAddressData, paymentAddressDataVariant);
    QVariant variantData;
    variantData.setValue(paymentAddressDataVariant);
    QSettings settings(ORGNAZATION, APP_NAME);
    settings.setValue(PAYMENT_ADDRESS_DATA_PREF, variantData);
}

void PaymentAddressWindow::createNewItem(int rowNo, QString tag, QString address)
{
    // insert new tag-addr item
    ui->twAddress->insertRow(rowNo);
    QTableWidgetItem *item;
    item = new QTableWidgetItem(tag, PaymentAddressWindow::ctTag);
    item->setTextAlignment(Qt::AlignCenter);
    ui->twAddress->setItem(rowNo, PaymentAddressWindow::colTag, item);
    QTableWidgetItem *anotherItem = new QTableWidgetItem(address, PaymentAddressWindow::ctAddress);
    anotherItem->setTextAlignment(Qt::AlignCenter);
    ui->twAddress->setItem(rowNo, PaymentAddressWindow::colAddress, anotherItem);
}

void PaymentAddressWindow::on_btnExport_clicked()
{
    QString dlgTitle = tr("Export To");
    QString filter = tr("CSV File(*.csv);;All File(*.*)");
    QString filename = QFileDialog::getSaveFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    if (!exportPaymentAddressToCsvFile(filename))
        QMessageBox::warning(nullptr, tr("Warning"), tr("Unable to export.\nPlease check relevant permissions."));
}

bool PaymentAddressWindow::exportPaymentAddressToCsvFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QString str = tr("\"Payment Address Data\",\"Tag\",\"Address\"\n");
    QTableWidgetItem *item;
    for (int i = 0; i < ui->twAddress->rowCount(); ++i)
    {
        str += QString::asprintf("\"%d\",", i+1);
        for (int j = 0; j < ui->twAddress->columnCount()-1; ++j)
        {
            item = ui->twAddress->item(i,j);
            str += QString::asprintf("\"%s\",", item->text().toStdString().c_str());
        }
        item = ui->twAddress->item(i,ui->twAddress->columnCount()-1);
        str += QString::asprintf("\"%s\"\n", item->text().toStdString().c_str());
    }
    QByteArray strBytes = str.toUtf8();
    file.write(strBytes, strBytes.length());
    file.close();
    return true;
}

void PaymentAddressWindow::on_btnDelete_clicked()
{
    // remove tag-addr item
    int selectedRow = ui->twAddress->currentRow();
    if (selectedRow < 0 || selectedRow >= ui->twAddress->rowCount())
        return;
    QString addr = ui->twAddress->item(selectedRow, 1)->text();
    ui->twAddress->removeRow(selectedRow);
}

void PaymentAddressWindow::on_btnCopy_clicked()
{
    int selectedRow = ui->twAddress->currentRow();
    if (selectedRow < 0 || selectedRow >= ui->twAddress->rowCount())
        return;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->twAddress->item(selectedRow, PaymentAddressWindow::colAddress)->text());
}

void PaymentAddressWindow::on_btnImport_clicked()
{
    QString dlgTitle = tr("Import From");
    QString filter = tr("CSV File(*.csv);;All File(*.*)");
    QString filename = QFileDialog::getOpenFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    if (!importPaymentAddressFromCsvFile(filename))
        QMessageBox::warning(nullptr, tr("Warning"), tr("Unable to import.\nThe file may be corrupted."));
}

bool PaymentAddressWindow::importPaymentAddressFromCsvFile(const QString &filename)
{
    QFile file(filename);
    if (!file.exists())
        return false;
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    QString str;
    QStringList list;
    QString tmp;
    int row = 0;
    int totalRow = ui->twAddress->rowCount();
    stream.readLine();
    if (stream.atEnd())
        return false;
    while (!stream.atEnd())
    {
        str = stream.readLine();
        list = str.split(",");
        if (list.length() != 3)
            return false;
        for (int i = 1; i < list.length(); ++i)
        {
            tmp = list.at(i);
            tmp = tmp.trimmed();
            if (tmp.startsWith("\"") && tmp.endsWith("\""))
            {
                tmp = tmp.mid(1,tmp.length()-2);
                list.replace(i, tmp);
            }
            else
                return false;
        }
        createNewItem(totalRow+row, list.at(1), list.at(2));
        row++;
    }
    file.close();
    return true;
}

void PaymentAddressWindow::on_leSearch_textChanged(const QString &arg1)
{
    QTableWidgetItem *item;
    if (!arg1.isEmpty())
    {
        int matches = 0;
        for (int i = 0; i < ui->twAddress->rowCount(); ++i)
        {
            for (int j = 0; j < ui->twAddress->columnCount(); ++j)
            {
                item = ui->twAddress->item(i,j);
                if (item->text().contains(arg1))
                {
                    matches++;
                    item->setBackground(QColor(220,180,0,100));
                }
                else
                {
                    item->setBackground(QColor(0,0,0,0));
                }
            }
        }
        ui->lblMatches->setText(QString::asprintf(tr("%d Matches").toStdString().c_str(), matches));
    }
    else
    {
        for (int i = 0; i < ui->twAddress->rowCount(); ++i)
        {
            for (int j = 0; j < ui->twAddress->columnCount(); ++j)
            {
                item = ui->twAddress->item(i,j);
                item->setBackground(QColor(0,0,0,0));
            }
        }
        ui->lblMatches->setText(tr("0 Matches"));
    }
}
