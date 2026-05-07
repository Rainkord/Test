#include "taskdialog.h"
#include <QApplication>
#include <QScreen>
#include <QScrollArea>
#include <QPixmap>

static const char* GH_BG      = "#0d1117";
static const char* GH_SURFACE = "#161b22";
static const char* GH_BORDER  = "#30363d";
static const char* GH_TEXT    = "#e6edf3";
static const char* GH_MUTED   = "#8b949e";
static const char* GH_ACCENT  = "#238636";
static const char* GH_BTN_BG  = "#21262d";
static const char* GH_BTN_HV  = "#30363d";

TaskDialog::TaskDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8("\xd0\x97\xd0\xb0\xd0\xb4\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5"));
    setModal(true);

    // ---- root layout ----
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ---- style ----
    setStyleSheet(QString(
        "QDialog { background: %1; }"
        "QScrollArea { background: %1; border: none; }"
        "QWidget#container { background: %1; }"
        "QLabel { color: %4; }"
        "QPushButton {"
        "  background: %7; color: %4;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  padding: 6px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background: %8; }"
    ).arg(GH_BG, GH_SURFACE, GH_BORDER, GH_TEXT,
          GH_MUTED, GH_ACCENT, GH_BTN_BG, GH_BTN_HV));

    // ---- scroll area ----
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *container = new QWidget;
    container->setObjectName("container");
    QVBoxLayout *lay = new QVBoxLayout(container);
    lay->setContentsMargins(28, 28, 28, 28);
    lay->setSpacing(20);

    // ---- title ----
    QLabel *titleLabel = new QLabel;
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setWordWrap(true);
    titleLabel->setText(
        QString("<b style='color:%1; font-size:16pt;'>")
            .arg(GH_TEXT) +
        QString::fromUtf8("\xd0\x97\xd0\xb0\xd0\xb4\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5") +
        "</b>"
    );
    lay->addWidget(titleLabel);

    // ---- separator ----
    QFrame *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("color: %1;").arg(GH_BORDER));
    lay->addWidget(sep);

    // ---- task description ----
    QLabel *taskLabel = new QLabel;
    taskLabel->setTextFormat(Qt::RichText);
    taskLabel->setWordWrap(true);
    taskLabel->setOpenExternalLinks(false);

    QString taskHtml =
        QString("<div style='font-size:11pt; color:%1; line-height:1.6;'>").arg(GH_TEXT) +

        QString("<b style='color:%1; font-size:11pt;'>&#8470;9:</b><br><br>").arg(GH_TEXT) +

        QString::fromUtf8(
            "<b>\xd0\xa4\xd1\x83\xd0\xbd\xd0\xba\xd1\x86\xd0\xb8\xd1\x8f:</b> "
            "y = f(x), \xd0\xb3\xd0\xb4\xd0\xb5 f \xd0\xb7\xd0\xb0\xd0\xb4\xd0\xb0\xd0\xbd\xd0\xb0 "
            "\xd0\xba\xd1\x83\xd1\x81\xd0\xbe\xd1\x87\xd0\xbd\xd0\xbe:<br><br>"
        ) +

        "<table cellspacing='8'>"
        "<tr><td>|x|&nbsp;&minus;&nbsp;2,</td>"
            "<td>x&nbsp;&lt;&nbsp;&minus;2</td></tr>"
        "<tr><td>x&#178;&nbsp;+&nbsp;x&nbsp;+&nbsp;1,</td>"
            "<td>&minus;2&nbsp;&le;&nbsp;x&nbsp;&lt;&nbsp;2</td></tr>"
        "<tr><td>|x&nbsp;&minus;&nbsp;2|&nbsp;+&nbsp;1,</td>"
            "<td>x&nbsp;&ge;&nbsp;2</td></tr>"
        "</table><br>"

        + QString::fromUtf8(
            "<b>\xd0\x9f\xd0\xb0\xd1\x80\xd0\xb0\xd0\xbc\xd0\xb5\xd1\x82\xd1\x80\xd1\x8b:</b> "
            "a, b, c \xe2\x80\x94 \xd0\xb2\xd0\xb5\xd1\x89\xd0\xb5\xd1\x81\xd1\x82\xd0\xb2\xd0\xb5\xd0\xbd\xd0\xbd\xd1\x8b\xd0\xb5 \xd1\x87\xd0\xb8\xd1\x81\xd0\xbb\xd0\xb0.<br><br>"
        )

        + "<table cellspacing='8'>"
        "<tr><td>|x&nbsp;&middot;&nbsp;a|&nbsp;&minus;&nbsp;2,</td>"
            "<td>x&nbsp;&lt;&nbsp;&minus;2</td></tr>"
        "<tr><td>b&nbsp;&middot;&nbsp;x&#178;&nbsp;+&nbsp;x&nbsp;+&nbsp;1,</td>"
            "<td>&minus;2&nbsp;&le;&nbsp;x&nbsp;&lt;&nbsp;2</td></tr>"
        "<tr><td>|x&nbsp;&minus;&nbsp;2|&nbsp;+&nbsp;1&nbsp;&middot;&nbsp;c,</td>"
            "<td>x&nbsp;&ge;&nbsp;2</td></tr>"
        "</table><br>"

        + "</div>";

    taskLabel->setText(taskHtml);
    lay->addWidget(taskLabel);

    // ---- graph image ----
    QLabel *imgLabel = new QLabel;
    imgLabel->setAlignment(Qt::AlignCenter);
    QPixmap px(":/formula_graph.png");
    if (!px.isNull()) {
        imgLabel->setPixmap(px.scaledToWidth(460, Qt::SmoothTransformation));
    }
    lay->addWidget(imgLabel);

    lay->addStretch();

    // ---- close button ----
    QPushButton *closeBtn = new QPushButton(
        QString::fromUtf8("\xd0\x97\xd0\xb0\xd0\xba\xd1\x80\xd1\x8b\xd1\x82\xd1\x8c"));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    lay->addWidget(closeBtn, 0, Qt::AlignRight);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);

    QScreen *screen = QApplication::primaryScreen();
    QRect sg = screen ? screen->availableGeometry() : QRect(0,0,1280,800);
    resize(qMin(560, sg.width() - 80), qMin(700, sg.height() - 80));
}
