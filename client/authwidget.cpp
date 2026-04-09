#include "authwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

AuthWidget::AuthWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(60, 40, 60, 40);
    layout->setSpacing(12);

    // Title
    QLabel *titleLabel = new QLabel("Авторизация", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(16);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    layout->addSpacing(10);

    // Login field
    loginEdit = new QLineEdit(this);
    loginEdit->setPlaceholderText("Логин");
    loginEdit->setMinimumHeight(32);
    layout->addWidget(loginEdit);

    // Password field
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(32);
    layout->addWidget(passwordEdit);

    layout->addSpacing(8);

    // Login button
    loginBtn = new QPushButton("Войти", this);
    loginBtn->setMinimumHeight(36);
    layout->addWidget(loginBtn);

    // Register button
    registerBtn = new QPushButton("Регистрация", this);
    registerBtn->setMinimumHeight(36);
    layout->addWidget(registerBtn);

    // Status label
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("color: red;");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    layout->addWidget(statusLabel);

    layout->addStretch();

    // Connections
    connect(loginBtn, &QPushButton::clicked, this, &AuthWidget::onLoginClicked);
    connect(registerBtn, &QPushButton::clicked, this, &AuthWidget::showRegister);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &AuthWidget::onLoginClicked);
}

void AuthWidget::onLoginClicked()
{
    statusLabel->clear();

    QString login = loginEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Введите логин и пароль.");
        return;
    }

    ClientSingleton &client = ClientSingleton::instance();
    if (!client.isConnected()) {
        bool connected = client.connectToServer("127.0.0.1", 33333);
        if (!connected) {
            statusLabel->setText("Не удалось подключиться к серверу.");
            return;
        }
    }

    QString request = QString("auth||%1||%2").arg(login, password);
    QString response = client.sendRequest(request);

    if (response.startsWith("auth+")) {
        // auth+||login
        QStringList parts = response.split("||");
        QString userLogin = (parts.size() >= 2) ? parts[1].trimmed() : login;
        emit loginSuccess(userLogin);
        loginEdit->clear();
        passwordEdit->clear();
    } else {
        statusLabel->setText("Ошибка авторизации. Проверьте логин и пароль.");
    }
}
