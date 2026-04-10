#ifndef REGWIDGET_H
#define REGWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

class RegWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegWidget(QWidget *parent = nullptr);
    ~RegWidget();

signals:
    void registrationSuccess();
    void showAuth();

private slots:
    void onLoginTextChanged(const QString &text);
    void onPasswordTextChanged(const QString &text);
    void onConfirmPasswordTextChanged(const QString &text);
    void onTogglePassword1();
    void onTogglePassword2();
    void onContinueClicked();
    void onEmailTextChanged(const QString &text);
    void onBackClicked();
    void onConfirmEmailClicked();
    void onRegistrationResponseReceived(const QString &response);
    void onCodeTextChanged(const QString &text);
    void onVerifyCodeClicked();
    void onCodeLockTimerFired();
    void onShowAuthClicked();

private:
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

    QWidget     *step2Widget;
    QLineEdit   *emailEdit;
    QLabel      *emailErrorLabel;
    QPushButton *backBtn;

    QWidget     *step3Widget;
    QPushButton *confirmEmailBtn;
    QLabel      *codeStatusLabel;
    QLineEdit   *codeEdit;
    QLabel      *codeErrorLabel;
    QPushButton *verifyCodeBtn;

    QPushButton *showAuthBtn;

    int     codeFailedAttempts;
    int     codeLockLevel;
    QTimer  *codeLockTimer;
    bool    codeIsLocked;
    bool    m_verifyingCode;
    QString currentLogin;

    void setupUI();
    void validateStep1();
    bool isEmailValid(const QString &email) const;
    void applyCodeLock(int minutes, const QString &message);
    void showStep(int step);
};

#endif // REGWIDGET_H
