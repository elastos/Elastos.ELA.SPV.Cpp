/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: loginwindow.cpp
 *
 * DESC: This is the cpp file of the launch window (login window).
 */
#include "loginwindow.h"
#include "ui_loginwindow.h"

LoginWindow::LoginWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    // set widget attributes
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

// methods for moving window
void LoginWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_moving = true;
        m_lastPos = event->globalPos() - pos();
    }
}

void LoginWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_moving && (event->buttons() & Qt::LeftButton) &&
            (event->globalPos() - m_lastPos).manhattanLength() > QApplication::startDragDistance())
    {
        move(event->globalPos() - m_lastPos);
        m_lastPos = event->globalPos() - pos();
    }
}

void LoginWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_moving = false;
}

// launch or exit the application according to the key pressed
void LoginWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        reject();
    else
        accept();
}
