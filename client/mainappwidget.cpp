#include "mainappwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QFont>

// ── GitHub dark palette ───────────────────────────────────────────────────────
#define GH_BG           "#0d1117"
#define GH_CARD         "#161b22"
#define GH_BORDER       "#30363d"
#define GH_TEXT         "#e6edf3"
#define GH_MUTED        "#8b949e"
#define GH_RED          "#f85149"
#define GH_RED_H        "#da3633"
#define GH_BLUE         "#388bfd"
#define GH_BLUE_H       "#58a6ff"
#define GH_BTN_GHOST    "#21262d"
#define GH_BTN_GHOST_H  "#30363d"
#define FONT_FAMILY     "Segoe UI"

/**
 * @brief Конструктор главного виджета приложения
 *
 * Устанавливает стили и создаёт пользовательский интерфейс.
 *
 * @param parent родительский виджет
 */
MainAppWidget::MainAppWidget(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: 11pt; }")
        .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY));
    setupUI();
}

/// Деструктор главного виджета приложения
MainAppWidget::~MainAppWidget() {}

/**
 * @brief Инициализация пользовательского интерфейса
 *
 * Создаёт верхнюю панель с приветствием и кнопкой выхода,
 * а также стековый виджет с заглушкой основного контента.
 */
void MainAppWidget::setupUI()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // ── Header bar ──────────────────────────────────────────────────────────
    QWidget *header = new QWidget(this);
    header->setFixedHeight(52);
    header->setStyleSheet(QString(
        "QWidget { background-color: %1; border-bottom: 1px solid %2; }")
        .arg(GH_CARD).arg(GH_BORDER));

    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 20, 0);

    welcomeLabel = new QLabel(QString::fromUtf8("Добро пожаловать!"), header);
    welcomeLabel->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 13pt; font-weight: bold; border: none; background: transparent; }")
        .arg(GH_TEXT));

    logoutBtn = new QPushButton(QString::fromUtf8("Выйти"), header);
    logoutBtn->setFixedHeight(32);
    logoutBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: transparent; color: %1;"
        "  border: 1px solid %2; border-radius: 6px;"
        "  padding: 4px 14px; font-size: 10pt;"
        "}"
        "QPushButton:hover { color: %3; border-color: %3; }"
    ).arg(GH_MUTED).arg(GH_BORDER).arg(GH_RED));
    connect(logoutBtn, &QPushButton::clicked, this, &MainAppWidget::onLogoutClicked);

    headerLayout->addWidget(welcomeLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(logoutBtn);

    outerLayout->addWidget(header);

    // ── Content area ────────────────────────────────────────────────────────
    contentStack = new QStackedWidget(this);
    contentStack->setStyleSheet("QStackedWidget { background-color: " GH_BG "; border: none; }");

    // Placeholder page — replace with real app content
    QWidget *placeholder = new QWidget(contentStack);
    placeholder->setStyleSheet("QWidget { background: transparent; border: none; }");
    auto *phLayout = new QVBoxLayout(placeholder);
    phLayout->addStretch();

    QLabel *infoLabel = new QLabel(
        QString::fromUtf8("Вы успешно вошли в систему.\nЗдесь будет основной контент приложения."),
        placeholder);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 12pt; border: none; }")
        .arg(GH_MUTED));
    phLayout->addWidget(infoLabel);
    phLayout->addStretch();

    contentStack->addWidget(placeholder);
    outerLayout->addWidget(contentStack, 1);
}

/**
 * @brief Устанавливает логин пользователя для отображения в приветствии
 * @param login логин пользователя
 */
void MainAppWidget::setLogin(const QString &login)
{
    welcomeLabel->setText(
        QString::fromUtf8("Добро пожаловать, %1!").arg(login));
}

/// Обработчик нажатия кнопки «Выйти»: испускает сигнал logout
void MainAppWidget::onLogoutClicked()
{
    emit logout();
}
