#include "taskdialog.h"
#include <QApplication>
#include <QScreen>
#include <QPixmap>
#include <QScrollArea>

// ── GitHub-dark palette ────────────────────────────────────────────────────
static const char* GH_BG      = "#0d1117";
static const char* GH_SURFACE = "#161b22";
static const char* GH_BORDER  = "#30363d";
static const char* GH_TEXT    = "#e6edf3";
static const char* GH_MUTED   = "#8b949e";
static const char* GH_ACCENT  = "#58a6ff";

TaskDialog::TaskDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

TaskDialog::~TaskDialog() {}

void TaskDialog::setupUI()
{
    setWindowTitle("Задание");
    setModal(true);

    // ── overall dialog style ──────────────────────────────────────────────
    setStyleSheet(QString(
        "QDialog { background-color: %1; }"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical {"
        "  background: %2; width: 8px; border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: %3; border-radius: 4px; min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    ).arg(GH_BG).arg(GH_SURFACE).arg(GH_BORDER));

    // ── root layout ───────────────────────────────────────────────────────
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── scroll area ───────────────────────────────────────────────────────
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *container = new QWidget();
    container->setStyleSheet(QString("background-color: %1;").arg(GH_BG));

    auto *mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── helper lambda: make a card widget ────────────────────────────────
    auto makeCard = [&](QWidget *content) -> QWidget* {
        auto *card = new QWidget();
        card->setStyleSheet(QString(
            "QWidget { background-color: %1; border: 1px solid %2;"
            " border-radius: 8px; }"
        ).arg(GH_SURFACE).arg(GH_BORDER));
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 14, 16, 14);
        cl->setSpacing(0);
        cl->addWidget(content);
        return card;
    };

    // ── title ─────────────────────────────────────────────────────────────
    titleLabel = new QLabel("\u2139\ufe0f  \u0418\u043d\u0444\u043e\u0440\u043c\u0430\u0446\u0438\u044f \u043e \u0437\u0430\u0434\u0430\u043d\u0438\u0438");
    titleLabel->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 18px; font-weight: bold;"
        " background: transparent; border: none; }"
    ).arg(GH_TEXT));
    mainLayout->addWidget(titleLabel);

    // ── work title card ───────────────────────────────────────────────────
    workTitleLabel = new QLabel(
        "<span style='color:%1; font-size:11pt; font-weight:bold;'>\u0420\u0430\u0431\u043e\u0442\u0430:</span>"
        "<br>"
        "<span style='color:%2; font-size:10pt;'>\u0418\u0441\u0441\u043b\u0435\u0434\u043e\u0432\u0430\u043d\u0438\u0435 \u0438 \u043f\u043e\u0441\u0442\u0440\u043e\u0435\u043d\u0438\u0435 \u0433\u0440\u0430\u0444\u0438\u043a\u0430 \u0444\u0443\u043d\u043a\u0446\u0438\u0438</span>"
        .arg(GH_ACCENT).arg(GH_TEXT)
    );
    workTitleLabel->setTextFormat(Qt::RichText);
    workTitleLabel->setWordWrap(true);
    workTitleLabel->setStyleSheet("background: transparent; border: none;");
    mainLayout->addWidget(makeCard(workTitleLabel));

    // ── group card ────────────────────────────────────────────────────────
    groupLabel = new QLabel(
        "<span style='color:%1; font-size:11pt; font-weight:bold;'>\u0413\u0440\u0443\u043f\u043f\u0430:</span>"
        "<br>"
        "<span style='color:%2; font-size:10pt;'>251-372, \u0432\u0430\u0440\u0438\u0430\u043d\u0442 9</span>"
        .arg(GH_ACCENT).arg(GH_TEXT)
    );
    groupLabel->setTextFormat(Qt::RichText);
    groupLabel->setWordWrap(true);
    groupLabel->setStyleSheet("background: transparent; border: none;");
    mainLayout->addWidget(makeCard(groupLabel));

    // ── members card ──────────────────────────────────────────────────────
    membersLabel = new QLabel(
        "<span style='color:%1; font-size:11pt; font-weight:bold;'>\u0427\u043b\u0435\u043d\u044b \u0433\u0440\u0443\u043f\u043f\u044b:</span>"
        "<br>"
        "<span style='color:%2; font-size:10pt;'>"
        "&#8226; \u041e\u0440\u043b\u043e\u0432 \u0420\u0443\u0441\u043b\u0430\u043d &#8226; \u0422\u0438\u043c\u043e\u0444\u0435\u0435\u0432 \u0410\u043d\u0442\u043e\u043d"
        "</span>"
        .arg(GH_ACCENT).arg(GH_TEXT)
    );
    membersLabel->setTextFormat(Qt::RichText);
    membersLabel->setWordWrap(true);
    membersLabel->setStyleSheet("background: transparent; border: none;");
    mainLayout->addWidget(makeCard(membersLabel));

    // ── formula image card (formula_task.png — без a,b,c) ─────────────────
    {
        auto *card = new QWidget();
        card->setStyleSheet(QString(
            "QWidget { background-color: %1; border: 1px solid %2;"
            " border-radius: 8px; }"
        ).arg(GH_SURFACE).arg(GH_BORDER));
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 14, 16, 14);
        cl->setSpacing(8);

        auto *hdr = new QLabel("<b style='color:%1; font-size:11pt;'>\u0424\u0443\u043d\u043a\u0446\u0438\u044f \u2116\u202f9:</b>"
                               .arg(GH_ACCENT));
        hdr->setTextFormat(Qt::RichText);
        hdr->setStyleSheet("background: transparent; border: none;");
        cl->addWidget(hdr);

        formulaLabel = new QLabel();
        formulaLabel->setAlignment(Qt::AlignCenter);
        formulaLabel->setStyleSheet("background: transparent; border: none;");

        QPixmap pm(":/formula_task.png");
        if (!pm.isNull())
            formulaLabel->setPixmap(pm.scaledToWidth(460, Qt::SmoothTransformation));
        else
            formulaLabel->setText("<span style='color:#ff6b6b;'>\u0418\u0437\u043e\u0431\u0440\u0430\u0436\u0435\u043d\u0438\u0435 \u043d\u0435 \u043d\u0430\u0439\u0434\u0435\u043d\u043e</span>");

        cl->addWidget(formulaLabel);
        mainLayout->addWidget(card);
    }

    mainLayout->addStretch();

    // ── close button ──────────────────────────────────────────────────────
    closeBtn = new QPushButton("\u2715  \u0417\u0430\u043a\u0440\u044b\u0442\u044c");
    closeBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: 6px;"
        "  padding: 8px 20px; font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #21262d; }"
        "QPushButton:pressed { background-color: #161b22; }"
    ).arg(GH_SURFACE).arg(GH_TEXT).arg(GH_BORDER));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);

    // ── dialog size ───────────────────────────────────────────────────────
    QScreen *screen = QApplication::primaryScreen();
    QRect sg = screen ? screen->availableGeometry() : QRect(0,0,1280,800);
    resize(qMin(560, sg.width() - 80),
           qMin(700, sg.height() - 80));
}
