#include "resetwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QFont>
#include <QCryptographicHash>
#include <QRegularExpression>

// ── GitHub dark palette ────────────────────────────────────────────────────
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

// ── Shared style helpers ───────────────────────────────────────────────────
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
        "QLineEdit:focus { border-color: %6; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER)
     .arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE);
}

static QString primaryBtnStyle(bool enabled)
{
    if (enabled) {
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
        ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    } else {
        return QString(
            "QPushButton {"
            "  background-color: rgba(35,134,54,0.35);"
            "  color: rgba(255,255,255,0.4);"
            "  border: 1px solid rgba(240,246,252,0.05);"
            "  border-radius: 6px;"
            "  padding: 6px 16px;"
            "  font-family: '%1';"
            "  font-size: %2pt;"
            "  font-weight: bold;"
            "}"
        ).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    }
}

static QString blueBtnStyle(bool enabled)
{
    if (enabled) {
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
        ).arg(GH_BLUE).arg(GH_BLUE_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    } else {
        return QString(
            "QPushButton {"
            "  background-color: rgba(56,139,253,0.3);"
            "  color: rgba(255,255,255,0.4);"
            "  border: 1px solid rgba(240,246,252,0.05);"
            "  border-radius: 6px;"
            "  padding: 6px 16px;"
            "  font-family: '%1';"
            "  font-size: %2pt;"
            "  font-weight: bold;"
            "}"
        ).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    }
}

static QString ghostBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %3;"
        "  border: 1px solid %4;"
        "  border-radius: 6px;"
        "  padding: 5px 10px;"
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

static QString hintLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

static QString errorLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

static QString successLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

// ── Constructor ───────────────────────────────────────────────────────────
ResetWidget::ResetWidget(QWidget *parent)
    : QWidget(parent),
      failedAttempts(0),
      isLocked(false),
      m_currentStep(StepEmail),
      m_waitingForResponse(false)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &ResetWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &ResetWidget::onResetResponseReceived);

    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

ResetWidget::~ResetWidget() {}

// ── setupUI ────────────────────────────────────────────────────────────────
void ResetWidget::setupUI()
{
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(380);
    card->setStyleSheet(QString(
        "QWidget {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 10px;"
        "}"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 24, 28, 24);
    cardLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel("\xd0\x92\xd0\xbe\xd1\x81\xd1\x81\xd1\x82\xd0\xb0\xd0\xbd\xd0\xbe\xd0\xb2\xd0\xbb\xd0\xb5\xd0\xbd\xd0\xb8\xd0\xb5 \xd0\xbf\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8f", card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(8);

    // ── STEP 1 — Email ─────────────────────────────
    step1Widget = new QWidget(card);
    step1Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s1 = new QVBoxLayout(step1Widget);
    s1->setContentsMargins(0, 0, 0, 0);
    s1->setSpacing(6);

    QLabel *emailHint = new QLabel("\xd0\x92\xd0\xb2\xd0\xb5\xd0\xb4\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xbf\xd0\xbe\xd1\x87\xd1\x82\xd1\x83, \xd0\xbf\xd1\x80\xd0\xb8\xd0\xb2\xd1\x8f\xd0\xb7\xd0\xb0\xd0\xbd\xd0\xbd\xd1\x83\xd1\x8e \xd0\xba \xd0\xb0\xd0\xba\xd0\xba\xd0\xb0\xd1\x83\xd0\xbd\xd1\x82\xd1\x83:", step1Widget);
    emailHint->setStyleSheet(hintLabelStyle());
    s1->addWidget(emailHint);

    emailEdit = new QLineEdit(step1Widget);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(38);
    emailEdit->setStyleSheet(inputStyle());
    s1->addWidget(emailEdit);
    connect(emailEdit, &QLineEdit::textChanged, this, &ResetWidget::onEmailTextChanged);

    emailErrorLabel = new QLabel(step1Widget);
    emailErrorLabel->setStyleSheet(errorLabelStyle());
    emailErrorLabel->hide();
    s1->addWidget(emailErrorLabel);

    s1->addSpacing(4);
    continueBtn = new QPushButton("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb4\xd0\xbe\xd0\xbb\xd0\xb6\xd0\xb8\xd1\x82\xd1\x8c", step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setStyleSheet(primaryBtnStyle(false));
    connect(continueBtn, &QPushButton::clicked, this, &ResetWidget::onContinueClicked);
    s1->addWidget(continueBtn);

    cardLayout->addWidget(step1Widget);

    // ── STEP 2 — Code ──────────────────────────────
    step2Widget = new QWidget(card);
    step2Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s2 = new QVBoxLayout(step2Widget);
    s2->setContentsMargins(0, 0, 0, 0);
    s2->setSpacing(6);

    codeStatusLabel = new QLabel(step2Widget);
    codeStatusLabel->setStyleSheet(successLabelStyle());
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    s2->addWidget(codeStatusLabel);

    codeEdit = new QLineEdit(step2Widget);
    codeEdit->setPlaceholderText("\