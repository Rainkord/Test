#include "verifywidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QFont>
#include <QString>

VerifyWidget::VerifyWidget(QWidget *parent)
    : QWidget(parent),
      failedAttempts(0),
      lockLevel(0),
      isLocked(false),
      mode(AuthMode)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &VerifyWidget::onLockTimerFired);

    setupUI();
}

VerifyWidget::~VerifyWidget()
{
}

void VerifyWidget::setLogin(const QString &loginVal)
{
    login = loginVal;
    failedAttempts = 0;
    lockLevel = 0;
    isLocked = false;
    if (lockTimer->isActive()) {
        lockTimer->stop();
    }
    statusLabel->hide();
    attemptsLabel->hide();
    verifyBtn->setEnabled(true);
    codeEdit->clear();
}

void VerifyWidget::setMode(Mode m)
{
    mode = m;
}

void VerifyWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(12);

    QLabel *titleLabel = new QLabel("Подтверждение", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(18);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(6);

    infoLabel = new QLabel("Код отправлен на вашу почту", this);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("QLabel { color: #555555; font-size: 11pt; }");
    mainLayout->addWidget(infoLabel);

    mainLayout->addSpacing(10);

    codeEdit = new QLineEdit(this);
    codeEdit->setPlaceholderText("Введите код");
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(36);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; font-size: 14pt; letter-spacing: 4px; }");
    mainLayout->addWidget(codeEdit);

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    mainLayout->addWidget(statusLabel);

    attemptsLabel = new QLabel(this);
    attemptsLabel->setStyleSheet("QLabel { color: #888888; font-size: 10pt; }");
    attemptsLabel->setAlignment(Qt::AlignCenter);
    attemptsLabel->hide();
    mainLayout->addWidget(attemptsLabel);

    mainLayout->addSpacing(8);

    verifyBtn = new QPushButton("Подтвердить", this);
    verifyBtn->setMinimumHeight(38);
    verifyBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 4px; font-size: 13pt; }"
        "QPushButton:hover { background-color: #388E3C; }"
        "QPushButton:disabled { background-color: #cccccc; color: #666666; }"
    );
    connect(verifyBtn, &QPushButton::clicked, this, &VerifyWidget::onVerifyClicked);
    mainLayout->addWidget(verifyBtn);

    mainLayout->addSpacing(8);

    backBtn = new QPushButton("Назад", this);
    backBtn->setFlat(true);
    backBtn->setStyleSheet(
        "QPushButton { color: #2196F3; border: none; text-decoration: underline; font-size: 11pt; }"
        "QPushButton:hover { color: #1565C0; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &VerifyWidget::onBackClicked);
    mainLayout->addWidget(backBtn, 0, Qt::AlignCenter);

    mainLayout->addStretch();
}

void VerifyWidget::applyLock(int minutes, const QString &message)
{
    isLocked = true;
    verifyBtn->setEnabled(false);
    statusLabel->setText(message);
    statusLabel->show();
    attemptsLabel->hide();
    if (minutes == 0) {
        lockTimer->start(30 * 1000);
    } else {
        lockTimer->start(minutes * 60 * 1000);
    }
}

void VerifyWidget::onLockTimerFired()
{
    isLocked = false;
    verifyBtn->setEnabled(true);
    statusLabel->hide();
    attemptsLabel->hide();
}

void VerifyWidget::onVerifyClicked()
{
    if (isLocked) {
        int remainingMs  = lockTimer->remainingTime();
        int remainingSec = remainingMs / 1000;
        int remainingMin = remainingSec / 60;
        int remainingSecMod = remainingSec % 60;

        QString timeStr;
        if (remainingMin > 0) {
            timeStr = QString("Осталось %1 мин %2 сек").arg(remainingMin).arg(remainingSecMod);
        } else {
            timeStr = QString("Осталось %1 сек").arg(remainingSec);
        }

        statusLabel->setText("Аккаунт заблокирован. " + timeStr);
        statusLabel->show();
        return;
    }

    QString code = codeEdit->text().trimmed();
    if (code.isEmpty()) {
        statusLabel->setText("Введите код из письма.");
        statusLabel->show();
        return;
    }

    // Формируем запрос в зависимости от режима
    QString request;
    if (mode == RegMode) {
        request = QString("verify_reg||%1||%2").arg(login, code);
    } else {
        request = QString("verify_auth||%1||%2").arg(login, code);
    }

    QString response = ClientSingleton::instance().sendRequest(request);

    // Определяем префиксы успеха/ошибки по режиму
    QString successPrefix = (mode == RegMode) ? "reg+" : "auth+";
    QString failPrefix    = (mode == RegMode) ? "reg-" : "auth-";
    QString successMsg    = (mode == RegMode) ? "Регистрация прошла успешно!" : "Вход выполнен успешно!";

    if (response.startsWith(successPrefix)) {
        statusLabel->setStyleSheet("QLabel { color: #388E3C; font-size: 10pt; }");
        statusLabel->setText(successMsg);
        statusLabel->show();
        attemptsLabel->hide();
        verifyBtn->setEnabled(false);

        QTimer::singleShot(500, this, [this]() {
            emit verificationSuccess(login);
        });
        return;
    }

    if (response.startsWith(failPrefix)) {
        failedAttempts++;

        if (failedAttempts < 4) {
            int remaining = 4 - failedAttempts;
            statusLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
            statusLabel->setText(QString("Неверный код. Осталось попыток: %1").arg(remaining));
            statusLabel->show();
            attemptsLabel->hide();
            return;
        }

        if (failedAttempts == 4) {
            lockLevel = 1;
            applyLock(0, "Слишком много попыток. Заблокировано на 30 секунд");
            return;
        }

        if (failedAttempts == 5) {
            lockLevel = 2;
            applyLock(5, "Слишком много попыток. Заблокировано на 5 минут");
            return;
        }

        if (failedAttempts == 6) {
            lockLevel = 3;
            applyLock(10, "Слишком много попыток. Заблокировано на 10 минут");
            return;
        }

        lockLevel = 4;
        applyLock(9999, "Слишком много попыток. Аккаунт заблокирован на длительное время");
        return;
    }

    statusLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    statusLabel->setText("Ошибка соединения с сервером.");
    statusLabel->show();
}

void VerifyWidget::onBackClicked()
{
    if (lockTimer->isActive()) {
        lockTimer->stop();
    }
    isLocked = false;
    failedAttempts = 0;
    lockLevel = 0;
    statusLabel->hide();
    attemptsLabel->hide();
    verifyBtn->setEnabled(true);
    codeEdit->clear();

    emit backToAuth();
}
