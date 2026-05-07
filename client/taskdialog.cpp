#include "taskdialog.h"
#include <QApplication>
#include <QScreen>
#include <QPixmap>
#include <QScrollArea>

static const char* GH_BG      = "#0d1117";
static const char* GH_SURFACE = "#161b22";
static const char* GH_BORDER  = "#30363d";
static const char* GH_TEXT    = "#e6edf3";
static const char* GH_ACCENT  = "#58a6ff";

TaskDialog::TaskDialog(QWidget *parent) : QDialog(parent) { setupUI(); }
TaskDialog::~TaskDialog() {}

void TaskDialog::setupUI()
{
    setWindowTitle(QString::fromUtf8("\xd0\x97\xd0\xb0\xd0\xb4\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5"));
    setModal(true);
    setStyleSheet(QString(
        "QDialog { background-color: %1; }"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: %2; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: %3; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    ).arg(GH_BG).arg(GH_SURFACE).arg(GH_BORDER));

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *container = new QWidget();
    container->setStyleSheet(QString("background-color: %1;").arg(GH_BG));

    auto *mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    auto makeCard = [&](QWidget *content) -> QWidget* {
        auto *card = new QWidget();
        card->setStyleSheet(QString(
            "QWidget { background-color: %1; border: 1px solid %2; border-radius: 8px; }"
        ).arg(GH_SURFACE).arg(GH_BORDER));
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 14, 16, 14);
        cl->setSpacing(0);
        cl->addWidget(content);
        return card;
    };

    // title
    titleLabel = new QLabel(QString::fromUtf8("\xe2\x84\xb9\xef\xb8\x8f  \xd0\x98\xd0\xbd\xd1\x84\xd0\xbe\xd1\x80\xd0\xbc\xd0\xb0\xd1\x86\xd0\xb8\xd1\x8f \xd0\xbe \xd0\xb7\xd0\xb0\xd0\xb4\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb8"));
    titleLabel->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 18px; font-weight: bold; background: transparent; border: none; }"
    ).arg(GH_TEXT));
    mainLayout->addWidget(titleLabel);

    // work title card
    workTitleLabel = new QLabel();
    workTitleLabel->setTextFormat(Qt::RichText);
    workTitleLabel->setWordWrap(true);
    workTitleLabel->setStyleSheet("background: transparent; border: none;");
    workTitleLabel->setText(
        QString("<span style='color:%1; font-size:11pt; font-weight:bold;'>")
            .arg(GH_ACCENT)
        + QString::fromUtf8("\xd0\xa0\xd0\xb0\xd0\xb1\xd0\xbe\xd1\x82\xd0\xb0")
        + ":</span><br>"
        + QString("<span style='color:%1; font-size:10pt;'>").arg(GH_TEXT)
        + QString::fromUtf8("\xd0\x98\xd1\x81\xd1\x81\xd0\xbb\xd0\xb5\xd0\xb4\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5 \xd0\xb8 \xd0\xbf\xd0\xbe\xd1\x81\xd1\x82\xd1\x80\xd0\xbe\xd0\xb5\xd0\xbd\xd0\xb8\xd0\xb5 \xd0\xb3\xd1\x80\xd0\xb0\xd1\x84\xd0\xb8\xd0\xba\xd0\xb0 \xd1\x84\xd1\x83\xd0\xbd\xd0\xba\xd1\x86\xd0\xb8\xd0\xb8")
        + "</span>"
    );
    mainLayout->addWidget(makeCard(workTitleLabel));

    // group card
    groupLabel = new QLabel();
    groupLabel->setTextFormat(Qt::RichText);
    groupLabel->setWordWrap(true);
    groupLabel->setStyleSheet("background: transparent; border: none;");
    groupLabel->setText(
        QString("<span style='color:%1; font-size:11pt; font-weight:bold;'>").arg(GH_ACCENT)
        + QString::fromUtf8("\xd0\x93\xd1\x80\xd1\x83\xd0\xbf\xd0\xbf\xd0\xb0")
        + ":</span><br>"
        + QString("<span style='color:%1; font-size:10pt;'>").arg(GH_TEXT)
        + "251-372, "
        + QString::fromUtf8("\xd0\xb2\xd0\xb0\xd1\x80\xd0\xb8\xd0\xb0\xd0\xbd\xd1\x82")
        + " 9</span>"
    );
    mainLayout->addWidget(makeCard(groupLabel));

    // members card
    membersLabel = new QLabel();
    membersLabel->setTextFormat(Qt::RichText);
    membersLabel->setWordWrap(true);
    membersLabel->setStyleSheet("background: transparent; border: none;");
    membersLabel->setText(
        QString("<span style='color:%1; font-size:11pt; font-weight:bold;'>").arg(GH_ACCENT)
        + QString::fromUtf8("\xd0\xa7\xd0\xbb\xd0\xb5\xd0\xbd\xd1\x8b \xd0\xb3\xd1\x80\xd1\x83\xd0\xbf\xd0\xbf\xd1\x8b")
        + ":</span><br>"
        + QString("<span style='color:%1; font-size:10pt;'>").arg(GH_TEXT)
        + "&#8226; "
        + QString::fromUtf8("\xd0\x9e\xd1\x80\xd0\xbb\xd0\xbe\xd0\xb2 \xd0\xa0\xd1\x83\xd1\x81\xd0\xbb\xd0\xb0\xd0\xbd")
        + " &#8226; "
        + QString::fromUtf8("\xd0\xa2\xd0\xb8\xd0\xbc\xd0\xbe\xd1\x84\xd0\xb5\xd0\xb5\xd0\xb2 \xd0\x90\xd0\xbd\xd1\x82\xd0\xbe\xd0\xbd")
        + "</span>"
    );
    mainLayout->addWidget(makeCard(membersLabel));

    // formula image card
    {
        auto *card = new QWidget();
        card->setStyleSheet(QString(
            "QWidget { background-color: %1; border: 1px solid %2; border-radius: 8px; }"
        ).arg(GH_SURFACE).arg(GH_BORDER));
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 14, 16, 14);
        cl->setSpacing(8);

        auto *hdr = new QLabel();
        hdr->setTextFormat(Qt::RichText);
        hdr->setStyleSheet("background: transparent; border: none;");
        hdr->setText(
            QString("<b style='color:%1; font-size:11pt;'>").arg(GH_ACCENT)
            + QString::fromUtf8("\xd0\xa4\xd1\x83\xd0\xbd\xd0\xba\xd1\x86\xd0\xb8\xd1\x8f \xe2\x84\x969:")
            + "</b>"
        );
        cl->addWidget(hdr);

        formulaLabel = new QLabel();
        formulaLabel->setAlignment(Qt::AlignCenter);
        formulaLabel->setStyleSheet("background: transparent; border: none;");
        QPixmap pm(":/formula_task.png");
        if (!pm.isNull())
            formulaLabel->setPixmap(pm.scaledToWidth(460, Qt::SmoothTransformation));
        else
            formulaLabel->setText("<span style='color:#ff6b6b;'>Image not found</span>");
        cl->addWidget(formulaLabel);
        mainLayout->addWidget(card);
    }

    mainLayout->addStretch();

    closeBtn = new QPushButton();
    closeBtn->setText(QString::fromUtf8("\xe2\x9c\x95  \xd0\x97\xd0\xb0\xd0\xba\xd1\x80\xd1\x8b\xd1\x82\xd1\x8c"));
    closeBtn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: 1px solid %3;"
        " border-radius: 6px; padding: 8px 20px; font-size: 13px; }"
        "QPushButton:hover { background-color: #21262d; }"
        "QPushButton:pressed { background-color: #161b22; }"
    ).arg(GH_SURFACE).arg(GH_TEXT).arg(GH_BORDER));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);

    QScreen *screen = QApplication::primaryScreen();
    QRect sg = screen ? screen->availableGeometry() : QRect(0,0,1280,800);
    resize(qMin(560, sg.width() - 80), qMin(700, sg.height() - 80));
}
