#ifndef AUTHWIDGET_H
#define AUTHWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

class AuthWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AuthWidget(QWidget *parent = nullptr);
    ~AuthWidget();

signals:
    void loginSuccess(QString login);
    void showRegister();
    void showVerifyAuth(QString login);
    void showReset();                      // NEW

private slots:
    void onLoginClicked();
    void onTogglePassword();
    void onLockTimerFired();
    void onRegisterClicked();
    void onForgotClicked();                // NEW
    void onAuthResponseReceived(const QString &response);

private:
    QLineEdit   *loginEdit;
    QLineEdit   *passwordEdit;
    QPushButton *loginBtn;
    QPushButton *registerBtn;
    QPushButton *forgotBtn;                // NEW
    QPushButton *togglePasswordBtn;
    QLabel      *statusLabel;
    QLabel      *attemptsLabel;

    int    failedAttempts;
    int    lockLevel;
    QTimer *lockTimer;
    bool   isLocked;
    bool   m_waitingForAuth;

    void applyLock(int minutes, const QString &message);
    void setupUI();
};

#endif // AUTHWIDGET_H
