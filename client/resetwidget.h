/**
 * @file resetwidget.h
 * @brief Виджет восстановления пароля по email.
 *
 * Трёхшаговый процесс:
 * - Шаг 1: ввод email → сервер возвращает reset_code_sent||<sha256hash>
 * - Шаг 2: ввод кода → локальное сравнение SHA-256 (без сетевого запроса)
 * - Шаг 3: ввод нового пароля → set_new_password||email||passwordHash
 */

#ifndef RESETWIDGET_H
#define RESETWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>

class OtpInput;

class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResetWidget(QWidget *parent = nullptr);
    ~ResetWidget();

signals:
    /** @brief Испускается после успешного сброса пароля. */
    void resetSuccess();
    /** @brief Испускается при нажатии кнопки «Назад» на шаге 1. */
    void backToAuth();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void onEmailTextChanged(const QString &text);
    void onContinueClicked();
    void onCodeTextChanged(const QString &text);
    void onVerifyCodeClicked();
    void onNewPasswordTextChanged(const QString &text);
    void onConfirmPasswordTextChanged(const QString &text);
    void onTogglePassword1();
    void onTogglePassword2();
    void onSavePasswordClicked();
    void onBackClicked();
    void onLockTimerFired();
    void onResetResponseReceived(const QString &response);

private:
    enum Step { StepEmail, StepCode, StepPassword };

    // ── Шаг 1: email ──────────────────────────────────────────────────────
    QWidget     *step1Widget;
    QLineEdit   *emailEdit;
    QLabel      *emailErrorLabel;
    QPushButton *continueBtn;
    QPushButton *backBtn;

    // ── Шаг 2: код ────────────────────────────────────────────────────────
    QWidget     *step2Widget;
    OtpInput    *codeEdit;         ///< 6-боксовый OTP-ввод
    QLabel      *codeErrorLabel;
    QLabel      *codeStatusLabel;
    QPushButton *verifyCodeBtn;

    // ── Шаг 3: новый пароль ───────────────────────────────────────────────
    QWidget     *step3Widget;
    QLineEdit   *newPasswordEdit;
    QLineEdit   *confirmPasswordEdit;
    QLabel      *newPasswordErrorLabel;
    QLabel      *confirmErrorLabel;
    QPushButton *togglePassBtn1;
    QPushButton *togglePassBtn2;
    QPushButton *saveBtn;

    // ── Состояние ─────────────────────────────────────────────────────────
    Step    m_currentStep;
    QString m_email;             ///< email, введённый на шаге 1
    QString m_pendingCodeHash;   ///< SHA-256 хэш кода, полученный от сервера
    int     failedAttempts;
    int     lockLevel;
    bool    isLocked;
    QTimer *lockTimer;
    bool    m_waitingForCodeHash; ///< ожидаем ответ reset_code_sent от сервера
    bool    m_waitingForSave;     ///< ожидаем ответ password_changed от сервера

    void setupUI();
    void showStep(Step step);
    bool isEmailValid(const QString &email) const;
    void validatePasswords();
    void applyLock(int minutes, const QString &message);
};

#endif // RESETWIDGET_H
