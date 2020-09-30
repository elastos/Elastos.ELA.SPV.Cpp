/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: qrcodewindow.cpp
 *
 * DESC: This is the cpp file of the QRcode window.
 */
#include "qrcodewindow.h"
#include "ui_qrcodewindow.h"
#include "qrencode/bitstream.h"
#include "qrencode/config.h"
#include "qrencode/mask.h"
#include "qrencode/mmask.h"
#include "qrencode/mqrspec.h"
#include "qrencode/qrencode.h"
#include "qrencode/qrencode_inner.h"
#include "qrencode/qrinput.h"
#include "qrencode/qrspec.h"
#include "qrencode/rsecc.h"
#include "qrencode/split.h"
#include "mainwindow.h"

QRcodeWindow::QRcodeWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QRcodeWindow)
{
    ui->setupUi(this);
    // set widget attributes
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
}

QRcodeWindow::QRcodeWindow(QString content, QWidget *parent) :
    QRcodeWindow(parent)
{
    generateQRcode(content);
    ui->teContent->setText(content);
}

QRcodeWindow::~QRcodeWindow()
{
    delete ui;
}

// methods for moving window
void QRcodeWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_moving = true;
        m_lastPos = event->globalPos() - pos();
    }
}

void QRcodeWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_moving && (event->buttons() & Qt::LeftButton) &&
            (event->globalPos() - m_lastPos).manhattanLength() > QApplication::startDragDistance())
    {
        move(event->globalPos() - m_lastPos);
        m_lastPos = event->globalPos() - pos();
    }
}

void QRcodeWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_moving = false;
}

///////////////////////////////////////
// Core method for generating QRcode //
///////////////////////////////////////
void QRcodeWindow::generateQRcode(QString text)
{
    QRcode *qrcode;
    qrcode = QRcode_encodeString(text.toStdString().c_str(), 2, QR_ECLEVEL_Q, QR_MODE_8, 1);
    qint32 temp_width = ui->lblQR->width();
    qint32 temp_height = ui->lblQR->height();
    qint32 qrcode_width =  qrcode->width > 0 ? qrcode->width : 1;
    double scale_x = (double) temp_width  / (double) qrcode_width;
    double scale_y = (double) temp_height / (double) qrcode_width;
    QImage mainimg = QImage(temp_width, temp_height, QImage::Format_ARGB32);
    QPainter painter(&mainimg);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, temp_width, temp_height);
    QColor foreground(Qt::black);
    painter.setBrush(foreground);
    for (qint32 y = 0; y < qrcode_width; ++y)
    {
        for (qint32 x = 0; x < qrcode_width; ++x)
        {
            unsigned char b = qrcode->data[y * qrcode_width + x];
            if (b & 0x01)
            {
                QRectF r(x * scale_x, y * scale_y, scale_x, scale_y);
                painter.drawRects(&r, 1);
            }
        }
    }
    image = mainimg;
    QPixmap mainmap = QPixmap::fromImage(mainimg);
    ui->lblQR->setPixmap(mainmap);
}

void QRcodeWindow::on_btnClose_clicked()
{
    MainWindow::qrcWindow = nullptr;
    this->close();
}

void QRcodeWindow::on_btnCopy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->teContent->toPlainText());
}

void QRcodeWindow::on_btnSaveTo_clicked()
{
    QString dlgTitle = tr("Save Image To");
    QString filter = tr("PNG image(*.png);;JPEG image(*.jpeg)");
    QString filename = QFileDialog::getSaveFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    if (!saveQRcodeToImageFile(filename))
        QMessageBox::warning(nullptr, tr("Warning"), tr("Unable to save.\nPlease check relevant permissions."));
}

bool QRcodeWindow::saveQRcodeToImageFile(const QString &filename)
{
    QString format = filename.mid(filename.lastIndexOf(".")+1).toUpper();
    if (format == "JPG" || format == "JPEG" || format == "PNG")
    {
        return image.save(filename, format.toStdString().c_str());
    }
    else
        return false;
}
