#include "taskdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPixmap>

#define GH_BG          "#0d1117"
#define GH_CARD        "#161b22"
#define GH_BORDER      "#30363d"
#define GH_TEXT        "#e6edf3"
#define GH_MUTED       "#8b949e"
#define GH_BLUE        "#388bfd"
#define GH_BLUE_H      "#58a6ff"
#define FONT_FAMILY    "Segoe UI"

/**
 * @brief Конструктор диалога задания
 *
 * Устанавливает стили и создаёт пользовательский интерфейс.
 *
 * @param parent родительский виджет
 */
TaskDialog::TaskDialog(QWidget *parent)
    : QDialog(parent)
{
    setStyleSheet(QString(
        "QDialog { background-color: %1; color: %2; font-family: '%3'; font-size: 10pt; }"
        "QLabel  { background: transparent; color: %2; }"
    ).arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY));
    setupUI();
}

/// Деструктор диалога задания
TaskDialog::~TaskDialog() {}

/**
 * @brief Инициализация пользовательского интерфейса диалога
 *
 * Создаёт заголовок, информацию о работе, список участников,
 * формулу и кнопку закрытия.
 */
void TaskDialog::setupUI()
{
    setWindowTitle("\xd0\x97\xd0\xb0\xd0\xb4\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5 \xe2\x80\x94 \xd0\x9f\xd0\xbe\xd0\xb4\xd0\xb3\xd1\x80\xd1\x83\xd0\xbf\xd0\xbf\xd0\xb0 5");
    resize(580, 780);
    setFixedSize(580, 780);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 20);
    mainLayout->setSpacing(12);

    auto makeSep = [&]() {
        QFrame *sep = new QFrame(this);
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet(QString(
            "QFrame { background: %1; border: none; max-height: 1px; }"
        ).arg(GH_BORDER));
        return sep;
    };

    // ── Title
    titleLabel = new QLabel(this);
    titleLabel->setText(QString(
        "<b style='font-size:16pt; color:%1;'>\xd0\x97\xd0\xb0\xd0\xb4\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5</b>"
    ).arg(GH_TEXT));
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addWidget(makeSep());

    // ── Work title
    workTitleLabel = new QLabel(this);
    workTitleLabel->setText(QString(
        "<p style='font-size:12pt; font-weight:bold; color:%1; text-align:center;'>"
        "\xd0\x93\xd1\x80\xd0\xb0\xd1\x84\xd0\xb8\xd1\x87\xd0\xb5\xd1\x81\xd0\xba\xd0\xbe\xd0\xb5 \xd0\xbe\xd1\x82\xd0\xbe\xd0\xb1\xd1\x80\xd0\xb0\xd0\xb6\xd0\xb5\xd0\xbd\xd0\xb8\xd0\xb5 \xd0\xb2\xd0\xb5\xd1\x82\xd0\xb2\xd1\x8f\xd1\x89\xd0\xb5\xd0\xb9\xd1\x81\xd1\x8f \xd1\x84\xd1\x83\xd0\xbd\xd0\xba\xd1\x86\xd0\xb8\xd0\xb8<br>"
        "\xd0\xb2 \xd1\x80\xd0\xb0\xd0\xbc\xd0\xba\xd0\xb0\xd1\x85 \xd0\xba\xd0\xbb\xd0\xb8\xd0\xb5\xd0\xbd\xd1\x82-\xd1\x81\xd0\xb5\xd1\x80\xd0\xb2\xd0\xb5\xd1\x80\xd0\xbd\xd0\xbe\xd0\xb3\xd0\xbe \xd0\xbf\xd1\x80\xd0\xbe\xd0\xb5\xd0\xba\xd1\x82\xd0\xb0"
        "</p>"
    ).arg(GH_TEXT));
    workTitleLabel->setTextFormat(Qt::RichText);
    workTitleLabel->setWordWrap(true);
    workTitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(workTitleLabel);

    // ── Group
    groupLabel = new QLabel(this);
    groupLabel->setText(QString(
        "<p style='font-size:11pt; color:%1; text-align:center;'><b>\xd0\x9f\xd0\xbe\xd0\xb4\xd0\xb3\xd1\x80\xd1\x83\xd0\xbf\xd0\xbf\xd0\xb0 5</b></p>"
    ).arg(GH_MUTED));
    groupLabel->setTextFormat(Qt::RichText);
    groupLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(groupLabel);

    // ── Members card
    membersLabel = new QLabel(this);
    membersLabel->setText(QString(
        "<p style='font-size:10pt; color:%1; line-height:1.9;'>"
        "<b style='color:%2; font-size:11pt;'>\xd0\xa3\xd1\x87\xd0\xb0\xd1\x81\xd1\x82\xd0\xbd\xd0\xb8\xd0\xba\xd0\xb8:</b><br><br>"
        "&nbsp;&nbsp;\xe2\x97\x8f&nbsp;&nbsp;\xd0\x9e\xd1\x80\xd0\xbb\xd0\xbe\xd0\xb2 \xd0\xa0\xd1\x83\xd1\x81\xd0\xbb\xd0\xb0\xd0\xbd<br>"
        "&nbsp;&nbsp;\xe2\x97\x8f&nbsp;&nbsp;\xd0\x9a\xd0\xb0\xd1\x80\xd0\xb5\xd0\xbb\xd0\xb8\xd0\xbd \xd0\x9a\xd0\xb8\xd1\x80\xd0\xb8\xd0\xbb\xd0\xbb<br>"
        "&nbsp;&nbsp;\xe2\x97\x8f&nbsp;&nbsp;\xd0\xa1\xd0\xb5\xd1\x80\xd1\x91\xd0\xb3\xd0\xb8\xd0\xbd\xd0\xb0 \xd0\x95\xd0\xbb\xd0\xb8\xd0\xb7\xd0\xb0\xd0\xb2\xd0\xb5\xd1\x82\xd0\xb0<br>"
        "&nbsp;&nbsp;\xe2\x97\x8f&nbsp;&nbsp;\xd0\x92\xd0\xbe\xd1\x80\xd0\xbe\xd0\xb1\xd1\x8c\xd0\xb5\xd0\xb2\xd0\xb0 \xd0\x95\xd0\xbb\xd0\xb8\xd0\xb7\xd0\xb0\xd0\xb2\xd0\xb5\xd1\x82\xd0\xb0<br>"
        "&nbsp;&nbsp;\xe2\x97\x8f&nbsp;&nbsp;\xd0\xa1\xd0\xb0\xd1\x80\xd0\xb0\xd1\x84\xd0\xb0\xd0\xbd\xd0\xbe\xd0\xb2 \xd0\x90\xd0\xbb\xd0\xb5\xd0\xba\xd1\x81\xd0\xb5\xd0\xb9"
        "</p>"
    ).arg(GH_TEXT).arg(GH_TEXT));
    membersLabel->setTextFormat(Qt::RichText);
    membersLabel->setWordWrap(true);
    membersLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    membersLabel->setStyleSheet(QString(
        "QLabel { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px 16px; }"
    ).arg(GH_CARD).arg(GH_BORDER));
    mainLayout->addWidget(membersLabel);

    mainLayout->addWidget(makeSep());

    // ── Formula image (formula_task.png from Qt resources)
    formulaLabel = new QLabel(this);
    QPixmap formulaPix(":/formula_task.png");
    if (!formulaPix.isNull()) {
        // Масштабируем под ширину диалога с сохранением пропорций
        formulaLabel->setPixmap(
            formulaPix.scaledToWidth(520, Qt::SmoothTransformation)
        );
    } else {
        // Fallback если ресурс не загрузился
        formulaLabel->setText("\xd0\xa4\xd0\xbe\xd1\x80\xd0\xbc\xd1\x83\xd0\xbb\xd0\xb0 \xd0\xbd\xd0\xb5 \xd0\xbd\xd0\xb0\xd0\xb9\xd0\xb4\xd0\xb5\xd0\xbd\xd0\xb0");
        formulaLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(GH_MUTED));
    }
    formulaLabel->setAlignment(Qt::AlignCenter);
    formulaLabel->setStyleSheet(
        QString("QLabel { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 10px; }")
        .arg(GH_BG).arg(GH_BORDER)
    );
    mainLayout->addWidget(formulaLabel);

    mainLayout->addStretch(1);

    // ── Close button
    closeBtn = new QPushButton("\xd0\x97\xd0\xb0\xd0\xba\xd1\x80\xd1\x8b\xd1\x82\xd1\x8c", this);
    closeBtn->setMinimumHeight(36);
    closeBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: %1; color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
        "  font-size: 11pt; padding: 4px 20px; font-family: '%3';"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BLUE).arg(GH_BLUE_H).arg(FONT_FAMILY));
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    btnRow->addWidget(closeBtn);
    btnRow->addStretch(1);
    mainLayout->addLayout(btnRow);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    setLayout(mainLayout);
}
