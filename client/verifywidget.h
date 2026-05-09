/**
 * @file verifywidget.h
 * @brief Виджет ввода кода двухфакторной аутентификации (вход).
 *
 * Принимает от AuthWidget логин и SHA-256 хэш кода.
 * Сравнение происходит ЛОКАЛЬНО — без сетевого запроса.
 * 3 попытки, потом блокировка 30 сек.
 * ESC → backToAuth().
 */

#ifndef VERIFYWIDGET_H
#define VERIFYWIDGET_H

#include <QWidget>

class OtpInput;
class QPushButton;
class QLabel;
class QTimer;

class VerifyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VerifyWidget(QWidget *parent = nullptr);
    ~VerifyWidget();

    void setLogin(const QString &login, const QString &codeHash);
    void clearFields();

signals:
    void verificationSuccess(const QString &login);
    void backToAuth();

private slots:
    void onVerifyClicked();
    void onBackClicked();
    void onCodeCompleted(const QString &code);
    void onLockTimerFired();
    void onVerifyResponseReceived(const QString &response); ///< устарел, для совместимости

private:
    void setupUI();
    void applyLock(int seconds, const QString &message);
    void updateVerifyBtn();

    OtpInput    *otpInput;
    QPushButton *verifyBtn;
    QPushButton *backBtn;
    QLabel      *statusLabel;
    QLabel      *hintLabel;
    QTimer      *lockTimer;

    bool    isLocked;
    QString m_login;
    QString m_codeHash;
    int     m_attemptsLeft = 3;
};

#endif // VERIFYWIDGET_H
