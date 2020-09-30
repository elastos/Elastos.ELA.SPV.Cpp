/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: qrcodewindow.h
 *
 * DESC: This is the head file of the QRcode window, which will
 *       generate a QRcode according to the address parameter.
 *       It also provides image-saving and address-copying functions.
 *
 *       The constructor receives a string and the window will show it
 *       as a QRcode, and also display the raw text in the text area.
 *
 *       The QRcode generation is based on qrencode library.
 *
 * STRUCTURE:
 *      class QRcodeWindow
 */
#ifndef QRCODEWINDOW_H
#define QRCODEWINDOW_H

#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>

namespace Ui {
class QRcodeWindow;
}

class QRcodeWindow : public QWidget
{
    Q_OBJECT
private:
    // members for moving window
    bool m_moving = false;
    QPoint m_lastPos;
    // QRcode image
    QImage image;

private:
    // save image
    bool saveQRcodeToImageFile(const QString &filename);

// methods for moving window
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public:
    explicit QRcodeWindow(QWidget *parent = nullptr);
    explicit QRcodeWindow(QString content, QWidget *parent = nullptr);
    ~QRcodeWindow();

private slots:
    void on_btnClose_clicked();

    void on_btnCopy_clicked();

    void on_btnSaveTo_clicked();

private:
    Ui::QRcodeWindow *ui;
    void generateQRcode(QString text);
};

#endif // QRCODEWINDOW_H
