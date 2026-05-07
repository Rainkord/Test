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
    void onContinueClicked();
    void onEmailTextChanged(const QString &text);
    void onEmailNextClicked();          // новая кнопка «Далее» на шаге 2
    void onBackFromEmailClicked();      // назад с шага 2 на шаг 1
    void onBackFromCodeClicked();       // назад с шага 3 на шаг 2
    void onConfirmEmailClicked();       // отправить код
    void onRegistrationResponseReceived(const QString &response);
    void onCodeTextChanged(const QString &text);
    void onVerifyCodeClicked();
    void onCodeLockTimerFired();
    void onShowAuthClicked();

private:
    // Шаг 1: логин + пароль
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

    // Шаг 2: ввод email
    QWidget     *step2Widget;
    QLineEdit   *emailEdit;
    QLabel      *emailErrorLabel;
    QPushButton *backFromEmailBtn;
    QPushButton *emailNextBtn;

    // Шаг 3: ввод кода
    QWidget     *step3Widget;
    QLabel      *codeSentToLabel;   // «Код отправлен на ...@mail.ru»
    QPushButton *confirmEmailBtn;   // «Отправить код повторно»
    QLabel      *codeStatusLabel;
    QLineEdit   *codeEdit;
    QLabel      *codeErrorLabel;
    QPushButton *verifyCodeBtn;
    QPushButton *backFromCodeBtn;

    QPushButton *showAuthBtn;

    int     codeFailedAttempts;
    int     codeLockLevel;
    QTimer  *codeLockTimer;
    bool    codeIsLocked;
    bool    m_verifyingCode;
    bool    m_checkingLogin;
    bool    m_codeSent;
    QString currentLogin;
    QString currentEmail;

    void setupUI();
    void validateStep1();
    bool isEmailValid(const QString &email) const;
    void applyCodeLock(int minutes, const QString &message);
    void showStep(int step);
};

#endif // REGWIDGET_H
