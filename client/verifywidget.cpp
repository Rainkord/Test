#include "verifywidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QFont>

// ── GitHub dark palette ──────────────────────────────────────────────────
#define GH_BG          "#0d1117"
#define GH_CARD        "#161b22"
#define GH_BORDER      "#30363d"
#define GH_TEXT        "#e6edf3"
#define GH_MUTED       "#8b949e"
#define GH_GREEN       "#238636"
#define GH_GREEN_H     "#2ea043"
#define GH_BLUE        "#388bfd"
#define GH_BLUE_H      "#58a6ff"
#define GH_RED         "#f85149"
#define GH_INPUT_BG    "#0d1117"
#define GH_BTN_GHOST   "#21262d"
#define GH_BTN_GHOST_H "#30363d"

#define FONT_FAMILY     "Segoe UI"
#define FONT_SIZE_TITLE 16
#define FONT_SIZE_BTN   11
#define FONT_SIZE_INPUT 11
#define FONT_SIZE_SMALL 9

static QString inputStyle()
{
    return QString(
        "QLineEdit {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: 6px;"
        "  padding: 6px 10px; font-family: '%4'; font-size: %5pt;"
        "}"
        "QLineEdit:focus { border-color: %6; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE);
}

static QString primaryBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(35,134,54,0.35); color: rgba(255,255,255,0.4); }"
    ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString linkBtnStyle()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        "  font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE).arg(FONT_FAMILY).arg(FONT_SIZE_BTN).arg(GH_BLUE_H);
}

// ── Constructor ──────────────────────────────────────────────────────────
VerifyWidget::VerifyWidget(QWidget *parent)
    : QWidget(parent),
      failedAttempts(0),
      lockLevel(0),
      isLocked(false),
      m_waitingForVerify(false)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &VerifyWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &VerifyWidget::onVerifyResponseReceived);

    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

VerifyWidget::~VerifyWidget() {}

void VerifyWidget::setLogin(const QString &loginVal)
{
    login = loginVal;
    failedAttempts = 0;
    lockLevel = 0;
    isLocked = false;
    m_waitingForVerify = false;
    if (lockTimer->isActive()) lockTimer->stop();

    statusLabel->hide();
    attemptsLabel->hide();
    verifyBtn->setEnabled(true);
    codeEdit->clear();
}

void VerifyWidget::setupUI()
{
    // Центрирующий outer layout
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    // Карточка
    QWidget *card = new QWidget(this);
    card->setFixedWidth(340);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }")
        .arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(10);

    // Заголовок
    QLabel *titleLabel = new QLabel("Подтверждение входа", card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    cardLayout->addWidget(titleLabel);

    infoLabel = new QLabel("Код отправлен на вашу почту", card);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }")
                             .arg(GH_MUTED).arg(FONT_SIZE_SMALL));
    cardLayout->addWidget(infoLabel);
    cardLayout->addSpacing(4);

    codeEdit = new QLineEdit(card);
    codeEdit->setPlaceholderText("Введите код");
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(38);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet(
        inputStyle() +
        "QLineEdit { font-size: 15pt; letter-spacing: 4px; }"
    );
    cardLayout->addWidget(codeEdit);

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

    verifyBtn = new QPushButton("Подтвердить", card);
    verifyBtn->setMinimumHeight(38);
    verifyBtn->setStyleSheet(primaryBtnStyle());
    connect(verifyBtn, &QPushButton::clicked, this, &VerifyWidget::onVerifyClicked);
    cardLayout->addWidget(verifyBtn);

    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    cardLayout->addWidget(line);

    backBtn = new QPushButton("Назад", card);
    backBtn->setFlat(true);
    backBtn->setStyleSheet(linkBtnStyle());
    connect(backBtn, &QPushButton::clicked, this, &VerifyWidget::onBackClicked);
    cardLayout->addWidget(backBtn, 0, Qt::AlignCenter);

    outerH->addWidget(card);
    outerH->addStretch(1);
    outerV->addLayout(outerH);
    outerV->addStretch(1);

    setLayout(outerV);
}

void VerifyWidget::applyLock(int minutes, const QString &message)
{
    isLocked = true;
    verifyBtn->setEnabled(false);
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }")
                               .arg(GH_RED).arg(FONT_SIZE_SMALL));
    statusLabel->setText(message);
    statusLabel->show();
    attemptsLabel->hide();
    lockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
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
        int remainingSec    = lockTimer->remainingTime() / 1000;
        int remainingMin    = remainingSec / 60;
        int remainingSecMod = remainingSec % 60;
        statusLabel->setText(remainingMin > 0
            ? QString("Аккаунт заблокирован. Осталось %1 мин %2 сек").arg(remainingMin).arg(remainingSecMod)
            : QString("Аккаунт заблокирован. Осталось %1 сек").arg(remainingSec));
        statusLabel->show();
        return;
    }

    QString code = codeEdit->text().trimmed();
    if (code.isEmpty()) {
        statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));
        statusLabel->setText("Введите код из письма.");
        statusLabel->show();
        return;
    }

    m_waitingForVerify = true;
    verifyBtn->setEnabled(false);
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_MUTED).arg(FONT_SIZE_SMALL));
    statusLabel->setText("Проверяем код...");
    statusLabel->show();

    ClientSingleton::instance().sendRequestAsync(QString("verify_auth||%1||%2").arg(login, code));
}

void VerifyWidget::onVerifyResponseReceived(const QString &response)
{
    if (!m_waitingForVerify) return;
    m_waitingForVerify = false;

    QString r = response.trimmed();

    if (r.isEmpty()) {
        verifyBtn->setEnabled(true);
        statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));
        statusLabel->setText("Ошибка соединения с сервером.");
        statusLabel->show();
        return;
    }

    if (r.startsWith("auth+")) {
        statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_GREEN_H).arg(FONT_SIZE_SMALL));
        statusLabel->setText("Вход выполнен успешно!");
        statusLabel->show();
        attemptsLabel->hide();
        verifyBtn->setEnabled(false);
        QTimer::singleShot(500, this, [this]() { emit verificationSuccess(login); });
        return;
    }

    verifyBtn->setEnabled(true);
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));

    if (r.startsWith("auth-")) {
        failedAttempts++;
        if (failedAttempts < 4) {
            statusLabel->setText(QString("Неверный код. Осталось попыток: %1").arg(4 - failedAttempts));
            statusLabel->show();
            attemptsLabel->hide();
            return;
        }
        if (failedAttempts == 4) { lockLevel = 1; applyLock(0,    "Слишком много попыток. Заблокировано на 30 секунд"); return; }
        if (failedAttempts == 5) { lockLevel = 2; applyLock(5,    "Слишком много попыток. Заблокировано на 5 минут");  return; }
        if (failedAttempts == 6) { lockLevel = 3; applyLock(10,   "Слишком много попыток. Заблокировано на 10 минут"); return; }
        lockLevel = 4; applyLock(9999, "Слишком много попыток. Аккаунт заблокирован");
        return;
    }

    m_waitingForVerify = false;
    verifyBtn->setEnabled(true);
    statusLabel->hide();
}

void VerifyWidget::onBackClicked()
{
    if (lockTimer->isActive()) lockTimer->stop();
    isLocked = false;
    failedAttempts = 0;
    lockLevel = 0;
    m_waitingForVerify = false;
    statusLabel->hide();
    attemptsLabel->hide();
    verifyBtn->setEnabled(true);
    codeEdit->clear();
    emit backToAuth();
}
