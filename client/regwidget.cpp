#include "regwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QFont>

RegWidget::RegWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(60, 40, 60, 40);
    layout->setSpacing(12);

    // Title
    QLabel *titleLabel = new QLabel("Регистрация", this);
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

    // Confirm password field
    confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setPlaceholderText("Повторите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(32);
    layout->addWidget(confirmPasswordEdit);

    // Email field
    emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(32);
    layout->addWidget(emailEdit);

    layout->addSpacing(8);

    // Register button
    registerBtn = new QPushButton("Зарегистрироваться", this);
    registerBtn->setMinimumHeight(36);
    layout->addWidget(registerBtn);

    // Back button
    backBtn = new QPushButton("Назад", this);
    backBtn->setMinimumHeight(36);
    layout->addWidget(backBtn);

    // Status label
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    layout->addWidget(statusLabel);

    layout->addStretch();

    // Connections
    connect(registerBtn, &QPushButton::clicked, this, &RegWidget::onRegisterClicked);
    connect(backBtn, &QPushButton::clicked, this, &RegWidget::showAuth);
}

void RegWidget::onRegisterClicked()
{
    statusLabel->setStyleSheet("color: red;");
    statusLabel->clear();

    QString login    = loginEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirm  = confirmPasswordEdit->text();
    QString email    = emailEdit->text().trimmed();

    if (login.isEmpty() || password.isEmpty() || confirm.isEmpty() || email.isEmpty()) {
        statusLabel->setText("Все поля обязательны для заполнения.");
        return;
    }

    if (password != confirm) {
        statusLabel->setText("Пароли не совпадают.");
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

    QString request = QString("registration||%1||%2||%3||%4")
                        .arg(login, password, confirm, email);
    QString response = client.sendRequest(request);

    if (response.startsWith("reg+")) {
        statusLabel->setStyleSheet("color: green;");
        statusLabel->setText("Регистрация прошла успешно! Войдите в систему.");
        loginEdit->clear();
        passwordEdit->clear();
        confirmPasswordEdit->clear();
        emailEdit->clear();
        emit registrationSuccess();
    } else {
        // reg-||reason
        QStringList parts = response.split("||");
        QString reason = (parts.size() >= 2) ? parts[1].trimmed() : "Неизвестная ошибка.";
        statusLabel->setText("Ошибка регистрации: " + reason);
    }
}
