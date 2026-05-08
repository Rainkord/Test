#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class QTimer;

class AuthWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AuthWidget(QWidget *parent = nullptr);
    ~AuthWidget();

    void clearFields();

signals:
    void showRegister();
    void showReset();
    void showVerifyAuth(const QString &login, const QString &codeHash);

private slots:
    void onTogglePassword();
    void onLoginClicked();
    void onRegisterClicked();
    void onForgotClicked();
    void onAuthResponseReceived(const QString &response);
    void onLockTimerFired();

private:
    void setupUI();
    void applyLock(int durationSec, const QString &message);

    QLineEdit   *loginEdit;
    QLineEdit   *passwordEdit;
    QPushButton *togglePasswordBtn;
    QPushButton *loginBtn;
    QPushButton *registerBtn;
    QPushButton *forgotBtn;
    QLabel      *statusLabel;
    QLabel      *attemptsLabel;
    QTimer      *lockTimer;

    int     failedAttempts;
    int     lockLevel;
    bool    isLocked;
    bool    m_waitingForAuth;    // true пока ждём ответ сервера на auth||
    QString m_pendingLogin;      // логин текущей попытки входа
    QString m_pendingCodeHash;   // SHA-256 хэш кода, полученный от сервера
};
