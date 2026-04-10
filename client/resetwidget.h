#ifndef RESETWIDGET_H
#define RESETWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResetWidget(QWidget *parent = nullptr);
    ~ResetWidget();

signals:
    void backToAuth();
    void resetSuccess();

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
    // ── Step 1: email ─────────────────────────────
    QWidget     *step1Widget;
    QLineEdit   *emailEdit;
    QLabel      *emailErrorLabel;
    QPushButton *continueBtn;

    // ── Step 2: code ──────────────────────────────
    QWidget     *step2Widget;
    QLabel      *codeStatusLabel;
    QLineEdit   *codeEdit;
    QLabel      *codeErrorLabel;
    QPushButton *verifyCodeBtn;

    // ── Step 3: new password ──────────────────────
    QWidget     *step3Widget;
    QLineEdit   *newPasswordEdit;
    QLabel      *newPasswordErrorLabel;
    QPushButton *togglePassBtn1;
    QLineEdit   *confirmPasswordEdit;
    QLabel      *confirmErrorLabel;
    QPushButton *togglePassBtn2;
    QPushButton *saveBtn;

    QPushButton *backBtn;

    // ── State ─────────────────────────────────────
    QString m_email;
    QString m_code;
    int     failedAttempts;
    bool    isLocked;
    QTimer  *lockTimer;

    enum Step { StepEmail, StepCode, StepPassword };
    Step    m_currentStep;

    bool    m_waitingForResponse;

    // ── Helpers ───────────────────────────────────
    void setupUI();
    void showStep(Step step);
    bool isEmailValid(const QString &email) const;
    void validatePasswords();
    void applyLock(int minutes, const QString &message);
};

#endif // RESETWIDGET_H
