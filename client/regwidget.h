/**
 * @file regwidget.h
 * @brief Виджет регистрации нового пользователя.
 */

#ifndef REGWIDGET_H
#define REGWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>

class OtpInput;

/**
 * @class RegWidget
 * @brief Трёхшаговая регистрация. На шаге 3 — OTP-ввод 6 боксов.
 * ESC на любом шаге → переход на предыдущий шаг / назад к авторизации.
 */
class RegWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegWidget(QWidget *parent = nullptr);
    ~RegWidget();
    void clearFields();

signals:
    void registrationSuccess(const QString &login);
    void showAuth();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void onLoginTextChanged(const QString &text);
    void onPasswordTextChanged(const QString &text);
    void onConfirmPasswordTextChanged(const QString &text);
    void onTogglePassword1();
    void onTogglePassword2();
    void onContinueClicked();
    void onEmailTextChanged(const QString &text);
    void onEmailNextClicked();
    void onBackToStep1Clicked();
    void onBackToStep2Clicked();
    void onRegistrationResponseReceived(const QString &response);
    void onCodeCompleted(const QString &code);
    void onVerifyCodeClicked();
    void onCodeLockTimerFired();
    void onShowAuthClicked();

private:
    // ── Шаг 1 ──────────────────────────────────────────────────────
    QWidget     *step1Widget;
    QLineEdit   *loginEdit;
    QLabel      *loginErrorLabel;
    QLineEdit   *passwordEdit;
    QLabel      *passwordErrorLabel;
    QPushButton *togglePassBtn1;
    QLineEdit   *confirmPasswordEdit;
    QLabel      *confirmErrorLabel;
    QPushButton *togglePassBtn2;
    QPushButton *continueBtn;

    // ── Шаг 2 ──────────────────────────────────────────────────────
    QWidget     *step2Widget;
    QLineEdit   *emailEdit;
    QLabel      *emailErrorLabel;
    QPushButton *emailNextBtn;
    QPushButton *backToStep1Btn;

    // ── Шаг 3 ──────────────────────────────────────────────────────
    QWidget     *step3Widget;
    QLabel      *emailHintLabel;
    OtpInput    *otpCode;          ///< 6-боксовый OTP-ввод
    QLabel      *codeErrorLabel;
    QLabel      *codeStatusLabel;
    QPushButton *verifyCodeBtn;
    QPushButton *backToStep2Btn;

    QPushButton *showAuthBtn;

    int     codeFailedAttempts;
    int     codeLockLevel;
    QTimer  *codeLockTimer;
    bool    codeIsLocked;
    bool    m_checkingLogin;
    bool    m_waitingForRegCode;
    bool    m_verifyingCode;
    QString m_pendingCodeHash;
    QString m_pendingPassHash;
    QString currentLogin;
    QString currentEmail;

    void setupUI();
    void validateStep1();
    bool isEmailValid(const QString &email) const;
    void applyCodeLock(int minutes, const QString &message);
    void showStep(int step);
    void updateVerifyCodeBtn();

    QString primaryBtnStyle() const;
    QString secondaryBtnStyle() const;
    QString ghostBtnStyle() const;
    QString linkBtnStyle() const;
    QString inputStyle() const;
    QString errorLabelStyle() const;
    QString infoLabelStyle() const;
    QString successLabelStyle() const;
};

#endif // REGWIDGET_H
