#include "verifywidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QFont>

#define GH_BG        "#0d1117"
#define GH_CARD      "#161b22"
#define GH_BORDER    "#30363d"
#define GH_TEXT      "#e6edf3"
#define GH_MUTED     "#8b949e"
#define GH_GREEN     "#238636"
#define GH_GREEN_H   "#2ea043"
#define GH_BLUE      "#388bfd"
#define GH_BLUE_H    "#58a6ff"
#define GH_RED       "#f85149"
#define GH_INPUT_BG  "#0d1117"

#define FONT_FAMILY     "Segoe UI"
#define FONT_SIZE_TITLE 16
#define FONT_SIZE_BTN   11
#define FONT_SIZE_INPUT 11
#define FONT_SIZE_SMALL 9

static QString primaryBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: #fff;"
        "  border: 1px solid rgba(240,246,252,0.1);"
        "  border-radius: 6px; padding: 6px 16px;"
        "  font-family: '%3'; font-size: %4pt; font-weight: bold;"
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

VerifyWidget::VerifyWidget(QWidget *parent)
    : QWidget(parent), lockLevel(0), isLocked(false)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &VerifyWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &VerifyWidget::onVerifyResponseReceived);

    setStyleSheet(QString("QWidget { background-color: %1; color: %2; "
                          "font-family: '%3'; font-size: %4pt; }")
                  .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

VerifyWidget::~VerifyWidget() {}

void VerifyWidget::setLogin(const QString &login)
{
    m_login = login;
    verifyBtn->setEnabled(true);
    codeEdit->clear();
    statusLabel->hide();
    statusLabel->setText("");

    promptLabel->setText(
        QString::fromUtf8("\xd0\x9d\xd0\xb0 \xd0\xbf\xd0\xbe\xd1\x87\xd1\x82\xd1\x83, \xd0\xbf\xd1\x80\xd0\xb8\xd0\xb2\xd1\x8f\xd0\xb7\xd0\xb0\xd0\xbd\xd0\xbd\xd1\x83\xd1\x8e \xd0\xba \xd0\xb0\xd0\xba\xd0\xba\xd0\xb0\xd1\x83\xd0\xbd\xd1\x82\xd1\x83 ")
        + QString("<b>") + login + QString("</b>")
        + QString::fromUtf8(", \xd0\xbe\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xbb\xd0\xb5\xd0\xbd \xd0\xba\xd0\xbe\xd0\xb4 \xd0\xbf\xd0\xbe\xd0\xb4\xd1\x82\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb6\xd0\xb4\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x8f. \xd0\x92\xd0\xb2\xd0\xb5\xd0\xb4\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xb5\xd0\xb3\xd0\xbe \xd0\xbd\xd0\xb8\xd0\xb6\xd0\xb5."));

    isLocked = false;
    lockLevel = 0;
    if (lockTimer->isActive()) lockTimer->stop();
}

static QString inputStyle()
{
    return QString(
        "QLineEdit { background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 6px; padding: 6px 10px;"
        "  font-family: '%4'; font-size: %5pt; }"
        "QLineEdit:focus { border-color: %6; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE);
}

void VerifyWidget::setupUI()
{
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(360);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel(QString::fromUtf8("\xd0\x9f\xd0\xbe\xd0\xb4\xd1\x82\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb6\xd0\xb4\xd0\xb5\xd0\xbd\xd0\xb8\xd0\xb5 \xd0\xb2\xd1\x85\xd0\xbe\xd0\xb4\xd0\xb0"), card);
    QFont tf(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(tf);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(4);

    promptLabel = new QLabel(card);
    promptLabel->setStyleSheet(QString("QLabel { color: %1; font-size: %2pt; border: none; }")
                               .arg(GH_MUTED).arg(FONT_SIZE_SMALL));
    promptLabel->setWordWrap(true);
    promptLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(promptLabel);
    cardLayout->addSpacing(4);

    codeEdit = new QLineEdit(card);
    codeEdit->setPlaceholderText(QString::fromUtf8("\xd0\x92\xd0\xb2\xd0\xb5\xd0\xb4\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xba\xd0\xbe\xd0\xb4"));
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(38);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet(
        inputStyle() +
        "QLineEdit { letter-spacing: 4px; font-size: 16pt; font-weight: bold; }"
    );
    cardLayout->addWidget(codeEdit);

    statusLabel = new QLabel(card);
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }")
                               .arg(GH_RED).arg(FONT_SIZE_SMALL));
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    cardLayout->addWidget(statusLabel);
    cardLayout->addSpacing(4);

    verifyBtn = new QPushButton(QString::fromUtf8("\xd0\x9f\xd0\xbe\xd0\xb4\xd1\x82\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb4\xd0\xb8\xd1\x82\xd1\x8c"), card);
    verifyBtn->setMinimumHeight(38);
    verifyBtn->setDefault(true);
    verifyBtn->setAutoDefault(true);
    verifyBtn->setStyleSheet(primaryBtnStyle());
    connect(verifyBtn, &QPushButton::clicked, this, &VerifyWidget::onVerifyClicked);
    connect(codeEdit, &QLineEdit::returnPressed, verifyBtn, &QPushButton::click);
    cardLayout->addWidget(verifyBtn);

    backBtn = new QPushButton(QString::fromUtf8("\xd0\x9d\xd0\xb0\xd0\xb7\xd0\xb0\xd0\xb4"), card);
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

void VerifyWidget::onVerifyClicked()
{
    if (isLocked) {
        int sec = lockTimer->remainingTime() / 1000;
        statusLabel->setText(QString::fromUtf8("\xd0\x97\xd0\xb0\xd0\xb1\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xbd\xd0\xbe. \xd0\x9e\xd1\x81\xd1\x82\xd0\xb0\xd0\xbb\xd0\xbe\xd1\x81\xd1\x8c: %1 \xd1\x81\xd0\xb5\xd0\xba.").arg(sec));
        statusLabel->show();
        return;
    }

    QString code = codeEdit->text().trimmed();
    if (code.length() != 6) {
        statusLabel->setText(QString::fromUtf8("\xd0\x9a\xd0\xbe\xd0\xb4 \xd0\xb4\xd0\xbe\xd0\xbb\xd0\xb6\xd0\xb5\xd0\xbd \xd1\x81\xd0\xbe\xd0\xb4\xd0\xb5\xd1\x80\xd0\xb6\xd0\xb0\xd1\x82\xd1\x8c 6 \xd1\x86\xd0\xb8\xd1\x84\xd1\x80."));
        statusLabel->show();
        return;
    }

    verifyBtn->setEnabled(false);
    statusLabel->setText(QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb5\xd1\x80\xd1\x8f\xd0\xb5\xd0\xbc..."));
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_MUTED).arg(FONT_SIZE_SMALL));
    statusLabel->show();

    ClientSingleton::instance().sendRequestAsync(
        QString("verify_auth||%1||%2").arg(m_login, code));
}

void VerifyWidget::onLockTimerFired()
{
    isLocked = false;
    verifyBtn->setEnabled(true);
    statusLabel->hide();
}

void VerifyWidget::onVerifyResponseReceived(const QString &response)
{
    QString r = response.trimmed();
    if (r.isEmpty()) return;

    if (r.startsWith("verify+")) {
        QString login = r.mid(7);
        statusLabel->hide();
        emit verificationSuccess(login);
        return;
    }

    if (r == "wrong_verify_code") {
        lockLevel++;
        if (lockLevel >= 3) {
            isLocked = true;
            lockTimer->start(30 * 1000);
            statusLabel->setText(QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xb5\xd0\xb2\xd1\x8b\xd1\x88\xd0\xb5\xd0\xbd \xd0\xbb\xd0\xb8\xd0\xbc\xd0\xb8\xd1\x82. \xd0\x91\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xba\xd0\xb0 \xd0\xbd\xd0\xb0 30 \xd1\x81\xd0\xb5\xd0\xba."));
        } else {
            statusLabel->setText(QString::fromUtf8("\xd0\x9d\xd0\xb5\xd0\xb2\xd0\xb5\xd1\x80\xd0\xbd\xd1\x8b\xd0\xb9 \xd0\xba\xd0\xbe\xd0\xb4. \xd0\x9f\xd0\xbe\xd0\xbf\xd1\x8b\xd1\x82\xd0\xba\xd0\xb0 %1 \xd0\xb8\xd0\xb7 3.").arg(lockLevel));
            verifyBtn->setEnabled(true);
        }
        statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));
        statusLabel->show();
        return;
    }

    if (r == "verify_expired") {
        statusLabel->setText(QString::fromUtf8("\xd0\x9a\xd0\xbe\xd0\xb4 \xd0\xbf\xd1\x80\xd0\xbe\xd1\x81\xd1\x80\xd0\xbe\xd1\x87\xd0\xb5\xd0\xbd. \xd0\x9f\xd0\xbe\xd0\xb2\xd1\x82\xd0\xbe\xd1\x80\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xb2\xd1\x85\xd0\xbe\xd0\xb4."));
        statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));
        statusLabel->show();
        verifyBtn->setEnabled(true);
        return;
    }

    statusLabel->setText(QString::fromUtf8("\xd0\x9e\xd1\x88\xd0\xb8\xd0\xb1\xd0\xba\xd0\xb0 \xd1\x81\xd0\xb5\xd1\x80\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb0. \xd0\x9f\xd0\xbe\xd0\xbf\xd1\x80\xd0\xbe\xd0\xb1\xd1\x83\xd0\xb9\xd1\x82\xd0\xb5 \xd0\xbf\xd0\xbe\xd0\xb7\xd0\xb6\xd0\xb5."));
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }").arg(GH_RED).arg(FONT_SIZE_SMALL));
    statusLabel->show();
    verifyBtn->setEnabled(true);
}

void VerifyWidget::onBackClicked()
{
    if (lockTimer->isActive()) lockTimer->stop();
    isLocked  = false;
    lockLevel = 0;
    codeEdit->clear();
    statusLabel->hide();
    emit backToAuth();
}
