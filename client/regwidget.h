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

    void clearFields();

signals:
    void registrationSuccess(const QString &login);
    void showAuth();

private slots:
    void onLoginTextChanged(const QString &text);
    void onPasswordTextChanged(const QString &text);
    void onConfirmPasswordTextChanged(const QString &text);
    void onTogglePassword1();
    void onTogglePassword2();
    void onContinueClicked();       // шаг 1 → проверить логин
    void onEmailTextChanged(const QString &text);
    void onEmailNextClicked();      // шаг 2 → послать код
    void onBackToStep1Clicked();    // шаг 2/3 → назад на шаг 1
    void onBackToStep2Clicked();    // шаг 3 → назад на шаг 2
    void onRegistrationResponseReceived(const QString &response);
    void onCodeTextChanged(const QString &text);
    void onVerifyCodeClicked();
    void onCodeLockTimerFired();
    void onShowAuthClicked();

private:
    // Step 1: login + password
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

    // Step 2: email input
    QWidget     *step2Widget;
    QLineEdit   *emailEdit;
    QLabel      *emailErrorLabel;
    QPushButton *emailNextBtn;   // зелёная кнопка "Далее"
    QPushButton *backToStep1Btn;

    // Step 3: code input
    QWidget     *step3Widget;
    QLabel      *emailHintLabel; // "Код отправлен на ..."
    QLineEdit   *codeEdit;
    QLabel      *codeErrorLabel;
    QLabel      *codeStatusLabel;
    QPushButton *verifyCodeBtn;  // зелёная кнопка
    QPushButton *backToStep2Btn;

    QPushButton *showAuthBtn;

    int     codeFailedAttempts;
    int     codeLockLevel;
    QTimer  *codeLockTimer;
    bool    codeIsLocked;
    bool    m_verifyingCode;
    bool    m_checkingLogin;
    QString currentLogin;
    QString currentEmail;

    void setupUI();
    void validateStep1();
    bool isEmailValid(const QString &email) const;
    void applyCodeLock(int minutes, const QString &message);
    void showStep(int step);

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
