/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: loginwindow.h
 *
 * DESC: This is the head file of the launch window (login window).
 *
 *       The launch window does not provide any utility function,
 *       since it only acts as a direct transition between the startup
 *       and the main interface.
 *
 * STRUCTURE:
 *      class LoginWindow
 */
#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT
// members for moving window
private:
    bool m_moving = false;
    QPoint m_lastPos;

// methods for moving window
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *) override;

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
