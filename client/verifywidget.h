/**
 * @file verifywidget.h
 * @brief Виджет ввода кода двухфакторной аутентификации (вход).
 *
 * Принимает от AuthWidget логин и SHA-256 хэш кода.
 * Сравнение происходит ЛОКАЛЬНО — без сетевого запроса.
 * 3 попытки, потом блокировка 30 сек.
 */

#ifndef VERIFYWIDGET_H
#define VERIFYWIDGET_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class QTimer;

class VerifyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VerifyWidget(QWidget *parent = nullptr);
    ~VerifyWidget();

    /**
     * @brief Устанавливает логин и хэш кода для верификации.
     * @param login    Логин пользователя.
     * @param codeHash SHA-256 хэш кода, полученный от сервера.
     */
    void setLogin(const QString &login, const QString &codeHash);

    void clearFields();

signals:
    void verificationSuccess(const QString &login);
    void backToAuth();

private slots:
    void onVerifyClicked();
    void onBackClicked();
    void onCodeTextChanged(const QString &text);
    void onLockTimerFired();
    void onVerifyResponseReceived(const QString &response); ///< устарел, оставлен для обратной совместимости

private:
    void setupUI();
    void applyLock(int seconds, const QString &message);

    QLineEdit   *codeEdit;
    QPushButton *verifyBtn;
    QPushButton *backBtn;
    QLabel      *statusLabel;
    QLabel      *hintLabel;
    QTimer      *lockTimer;

    bool    isLocked;
    QString m_login;            ///< Логин текущего пользователя.
    QString m_codeHash;         ///< SHA-256 хэш кода (для локального сравнения).
    int     m_attemptsLeft = 3; ///< Оставшиеся попытки ввода кода.
};

#endif // VERIFYWIDGET_H
