#include "authwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QCryptographicHash>
#include <QFont>

// ── GitHub dark palette ────────────────────────────────────────────────────
#define GH_BG         "#0d1117"
#define GH_CARD       "#161b22"
#define GH_BORDER     "#30363d"
#define GH_TEXT       "#e6edf3"
#define GH_MUTED      "#8b949e"
#define GH_GREEN      "#238636"
#define GH_GREEN_H    "#2ea043"
#define GH_BLUE       "#388bfd"
#define GH_BLUE_H     "#58a6ff"
#define GH_RED        "#f85149"
#define GH_INPUT_BG   "#0d1117"
#define GH_BTN_GHOST  "#21262d"
#define GH_BTN_GHOST_H "#30363d"

#define FONT_FAMILY   "Segoe UI"
#define FONT_SIZE_TITLE 16
#define FONT_SIZE_BTN   11
#define FONT_SIZE_INPUT 11
#define FONT_SIZE_SMALL 9

AuthWidget::AuthWidget(QWidget *parent)
    : QWidget(parent),
      failedAttempts(0),
      lockLevel(0),
      isLocked(false),
      m_waitingForAuth(false)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &AuthWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &AuthWidget::onAuthResponseReceived);

    // Фон всего виджета — тёмный GitHub
    setStyleSheet(QString("QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
                  .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

AuthWidget::~AuthWidget() {}

static QString inputStyle()
{
    return QString(
        "QLineEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "  font-family: '%4';"
        "  font-size: %5pt;"
        "}"
        "QLineEdit:focus {"
        "  border-color: %6;"
        "}"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE);
}

static QString primaryBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1);"
        "  border-radius: 6px;"
        "  padding: 6px 16px;"
        "  font-family: '%3';"
        "  font-size: %4pt;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(35,134,54,0.4); color: rgba(255,255,255,0.4); }"
    ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString ghostBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %3;"
        "  border: 1px solid %4;"
        "  border-radius: 6px;"
        "  padding: 5px 14px;"
        "  font-family: '%5';"
        "  font-size: %6pt;"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString linkBtnStyle()
{
    return QString(
        "QPushButton {"
        "  color: %1;"
        "  border: none;"
        "  background: transparent;"
        "  font-family: '%2';"
        "  font-size: %3pt;"
        "}"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE).arg(FONT_FAMILY).arg(FONT_SIZE_BTN).arg(GH_BLUE_H);
}

void AuthWidget::setupUI()
{
    // Внешний layout — центрирует карточку по вертикали и горизонтали
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    // Карточка
    QWidget *card = new QWidget(this);
    card->setFixedWidth(340);
    card->setStyleSheet(QString(
        "QWidget {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 10px;"
        "}"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(10);

    // Заголовок
    QLabel *titleLabel = new QLabel("Авторизация", card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(6);

    // Логин
    loginEdit = new QLineEdit(card);
    loginEdit->setPlaceholderText("Логин");
    loginEdit->setMinimumHeight(38);
    loginEdit->setStyleSheet(inputStyle());
    cardLayout->addWidget(loginEdit);

    // Пароль
    QHBoxLayout *passRow = new QHBoxLayout();
    passRow->setSpacing(6);
    passwordEdit = new QLineEdit(card);
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(38);
    passwordEdit->setStyleSheet(inputStyle());
    passRow->addWidget(passwordEdit);

    togglePasswordBtn = new QPushButton("👁", card);
    togglePasswordBtn->setFixedSize(38, 38);
    togglePasswordBtn->setToolTip("Показать/скрыть пароль");
    togglePasswordBtn->setStyleSheet(ghostBtnStyle());
    connect(togglePasswordBtn, &QPushButton::clicked, this, &AuthWidget::onTogglePassword);
    passRow->addWidget(togglePasswordBtn);
    cardLayout->addLayout(passRow);

    // Статусы
    statusLabel = new QLabel(card);
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }")
                               .arg(GH_RED).arg(FONT_SIZE_SMALL));
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    cardLayout->addWidget(statusLabel);

    attemptsLabel = new QLabel(card);
    attemptsLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }")
                                 .arg(GH_MUTED).arg(FONT_SIZE_SMALL));
    attemptsLabel->setAlignment(Qt::AlignCenter);
    attemptsLabel->hide();
    cardLayout->addWidget(attemptsLabel);

    cardLayout->addSpacing(4);

    // Кнопка входа
    loginBtn = new QPushButton("Войти", card);
    loginBtn->setMinimumHeight(38);
    loginBtn->setStyleSheet(primaryBtnStyle());
    connect(loginBtn, &QPushButton::clicked, this, &AuthWidget::onLoginClicked);
    cardLayout->addWidget(loginBtn);

    // Разделитель
    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    cardLayout->addWidget(line);

    // Ссылки
    registerBtn = new QPushButton("Регистрация", card);
    registerBtn->setFlat(true);
    registerBtn->setStyleSheet(linkBtnStyle());
    connect(registerBtn, &QPushButton::clicked, this, &AuthWidget::onRegisterClicked);
    cardLayout->addWidget(registerBtn, 0, Qt::AlignCenter);

    forgotBtn = new QPushButton("Забыли пароль?", card);
    forgotBtn->setFlat(true);
    forgotBtn->setStyleSheet(linkBtnStyle());
    connect(forgotBtn, &QPushButton::clicked, this, &AuthWidget::onForgotClicked);
    cardLayout->addWidget(forgotBtn, 0, Qt::AlignCenter);

    outerH->addWidget(card);
    outerH->addStretch(1);
    outerV->addLayout(outerH);
    outerV->addStretch(1);

    setLayout(outerV);
}

void AuthWidget::onTogglePassword()
{
    passwordEdit->setEchoMode(
        passwordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password
    );
}

void AuthWidget::applyLock(int minutes, const QString &message)
{
    isLocked = true;
    loginBtn->setEnabled(false);
    statusLabel->setText(message);
    statusLabel->show();
    attemptsLabel->hide();
    lockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
}

void AuthWidget::onLockTimerFired()
{
    isLocked = false;
    loginBtn->setEnabled(true);
    statusLabel->hide();
    attemptsLabel->hide();
}

void AuthWidget::onLoginClicked()
{
    if (isLocked) {
        int remainingSec    = lockTimer->remainingTime() / 1000;
        int remainingMin    = remainingSec / 60;
        int remainingSecMod = remainingSec % 60;
        statusLabel->setText(remainingMin > 0
            ? QString("Заблокировано. Осталось %1 мин %2 сек").arg(remainingMin).arg(remainingSecMod)
            : QString("Заблокировано. Осталось %1 сек").arg(remainingSec));
        statusLabel->show();
        return;
    }

    QString login    = loginEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Введите логин и пароль.");
        statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));
        statusLabel->show();
        return;
    }

    loginBtn->setEnabled(false);
    statusLabel->setText("Подключаемся...");
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_MUTED).arg(FONT_SIZE_SMALL));
    statusLabel->show();

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = QString::fromLatin1(hashBytes.toHex());

    m_waitingForAuth = true;
    ClientSingleton::instance().sendRequestAsync(QString("auth||%1||%2").arg(login, passwordHash));
}

void AuthWidget::onAuthResponseReceived(const QString &response)
{
    if (!m_waitingForAuth) return;
    m_waitingForAuth = false;

    QString r = response.trimmed();
    if (r.isEmpty()) {
        loginBtn->setEnabled(true);
        statusLabel->setText("Ошибка соединения с сервером.");
        statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));
        statusLabel->show();
        return;
    }

    if (r == "auth_code_sent") {
        statusLabel->hide();
        emit showVerifyAuth(loginEdit->text().trimmed());
        return;
    }

    loginBtn->setEnabled(true);
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));

    if (r == "auth-") {
        failedAttempts++;
        if (failedAttempts < 4) {
            statusLabel->setText(QString("Неверный логин или пароль. Осталось попыток: %1").arg(4 - failedAttempts));
            statusLabel->show();
            return;
        }
        if (failedAttempts == 4) { lockLevel = 1; applyLock(0,    "Заблокировано на 30 секунд");            return; }
        if (failedAttempts == 5) { lockLevel = 2; applyLock(5,    "Заблокировано на 5 минут");              return; }
        if (failedAttempts == 6) { lockLevel = 3; applyLock(10,   "Заблокировано на 10 минут");             return; }
        lockLevel = 4; applyLock(9999, "Аккаунт заблокирован на длительное время");
        return;
    }

    m_waitingForAuth = false;
}

void AuthWidget::onRegisterClicked() { emit showRegister(); }
void AuthWidget::onForgotClicked()   { emit showReset(); }
