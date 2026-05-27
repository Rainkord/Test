#include "otpinput.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QFocusEvent>

// ── Palette (GitHub dark) ─────────────────────────────────────────────────
#define GH_BG         "#0d1117"
#define GH_CARD       "#161b22"
#define GH_BORDER     "#30363d"
#define GH_TEXT       "#e6edf3"
#define GH_BLUE       "#388bfd"
#define GH_GREEN      "#2ea043"
#define GH_GREEN_DIM  "#238636"
#define GH_GREEN_BG   "rgba(46,160,67,0.10)"
#define GH_RED        "#f85149"
#define GH_RED_BG     "rgba(248,81,73,0.10)"
#define GH_DISABLED   "#21262d"
#define GH_TEXT_DIS   "#484f58"
#define FONT_FAMILY   "Segoe UI"

// ── box style по состоянию ────────────────────────────────────────────────
// state: 0=empty, 1=focused, 2=filled, 3=error, 4=disabled
/**
 * @brief Генерирует CSS-стиль для поля ввода цифры по его состоянию
 * @param state состояние поля: 0 — пустое, 1 — фокус, 2 — заполнено, 3 — ошибка, 4 — отключено
 * @return строка со стилем CSS
 */
static QString boxStyle(int state)
{
    const char *bg          = GH_CARD;
    const char *color       = GH_TEXT;
    const char *border      = GH_BORDER;
    const char *focusBorder = GH_BLUE;

    switch (state) {
    case 1: // focused, пустой
        border = GH_BLUE;
        break;
    case 2: // filled
        bg          = GH_GREEN_BG;
        border      = GH_GREEN_DIM;
        focusBorder = GH_GREEN;
        break;
    case 3: // error
        bg          = GH_RED_BG;
        border      = GH_RED;
        focusBorder = GH_RED;
        break;
    case 4: // disabled
        bg     = GH_DISABLED;
        color  = GH_TEXT_DIS;
        border = GH_DISABLED;
        break;
    default: // 0 — empty, no focus
        break;
    }

    return QString(
        "QLineEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 2px solid %3;"
        "  border-radius: 10px;"
        "  font-family: '%4';"
        "  font-size: 22pt;"
        "  font-weight: bold;"
        "  selection-background-color: transparent;"
        "}"
        "QLineEdit:focus { border-color: %5; }"
    ).arg(bg, color, border, FONT_FAMILY, focusBorder);
}

// ── OtpInput ──────────────────────────────────────────────────────────────
/**
 * @brief Конструктор виджета OTP-ввода
 *
 * Создаёт шесть полей ввода и вызывает настройку интерфейса.
 *
 * @param parent родительский виджет
 */
OtpInput::OtpInput(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

/**
 * @brief Инициализация пользовательского интерфейса
 *
 * Создаёт шесть QLineEdit с фиксированным размером, настраивает
 * фильтр событий и подключает сигнал изменения текста.
 */
void OtpInput::setupUI()
{
    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(4, 4, 4, 4);
    row->setSpacing(10);

    for (int i = 0; i < N; ++i) {
        m_boxes[i] = new QLineEdit(this);
        m_boxes[i]->setMaxLength(1);
        m_boxes[i]->setFixedSize(52, 62);
        m_boxes[i]->setAlignment(Qt::AlignCenter);
        m_boxes[i]->setStyleSheet(boxStyle(0));
        m_boxes[i]->installEventFilter(this);
        m_boxes[i]->setContextMenuPolicy(Qt::NoContextMenu);

        connect(m_boxes[i], &QLineEdit::textChanged, this, [this, i](const QString &t) {
            if (!t.isEmpty() && !t[0].isDigit()) {
                m_boxes[i]->blockSignals(true);
                m_boxes[i]->clear();
                m_boxes[i]->blockSignals(false);
                updateBoxStyle(i);
                return;
            }
            onDigitChanged(i, t);
        });

        row->addWidget(m_boxes[i]);
    }
}

// ── style helpers ─────────────────────────────────────────────────────────
/**
 * @brief Обновляет стиль одного поля в зависимости от его состояния
 *
 * Учитывает включённость виджета, флаг ошибки, наличие текста и фокус.
 *
 * @param i индекс поля (0–5)
 */
void OtpInput::updateBoxStyle(int i)
{
    if (!m_boxes[i]->isEnabled()) { m_boxes[i]->setStyleSheet(boxStyle(4)); return; }
    if (m_error)                  { m_boxes[i]->setStyleSheet(boxStyle(3)); return; }
    bool filled  = !m_boxes[i]->text().isEmpty();
    bool focused =  m_boxes[i]->hasFocus();
    if (filled)  { m_boxes[i]->setStyleSheet(boxStyle(2)); return; }
    if (focused) { m_boxes[i]->setStyleSheet(boxStyle(1)); return; }
    m_boxes[i]->setStyleSheet(boxStyle(0));
}

/// Обновляет стили всех 6 полей ввода
void OtpInput::updateAllStyles()
{
    for (int i = 0; i < N; ++i) updateBoxStyle(i);
}

/**
 * @brief Устанавливает или снимает флаг ошибки
 *
 * При установке все поля подсвечиваются красным.
 *
 * @param error true для включения режима ошибки
 */
void OtpInput::setError(bool error)
{
    m_error = error;
    updateAllStyles();
}

// ── public API ────────────────────────────────────────────────────────────
/**
 * @brief Возвращает текущий введённый шестизначный код
 * @return строка из 6 цифр
 */
QString OtpInput::code() const
{
    QString s;
    for (int i = 0; i < N; ++i) s += m_boxes[i]->text();
    return s;
}

/// Очищает все поля ввода и сбрасывает флаг ошибки, устанавливает фокус на первое поле
void OtpInput::clear()
{
    m_error = false;
    for (int i = 0; i < N; ++i) {
        m_boxes[i]->blockSignals(true);
        m_boxes[i]->clear();
        m_boxes[i]->blockSignals(false);
        updateBoxStyle(i);
    }
    if (isEnabled()) m_boxes[0]->setFocus();
}

/**
 * @brief Проверяет, заполнены ли все 6 полей
 * @return true если все поля содержат цифру
 */
bool OtpInput::isComplete() const
{
    for (int i = 0; i < N; ++i)
        if (m_boxes[i]->text().isEmpty()) return false;
    return true;
}

/**
 * @brief Включает или отключает виджет и все его поля
 *
 * При включении устанавливает фокус на первое поле.
 *
 * @param enabled true для включения, false для отключения
 */
void OtpInput::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);
    for (int i = 0; i < N; ++i) m_boxes[i]->setEnabled(enabled);
    updateAllStyles();
    if (enabled) m_boxes[0]->setFocus();
}

// ── digit logic ───────────────────────────────────────────────────────────
/**
 * @brief Обработчик изменения цифры в конкретном поле
 *
 * При вводе цифры автоматически переводит фокус на следующее поле.
 * При заполнении всех полей испускает сигнал completed().
 *
 * @param index индекс поля (0–5)
 * @param text содержимое поля после изменения
 */
void OtpInput::onDigitChanged(int index, const QString &text)
{
    updateBoxStyle(index);
    if (text.length() == 1 && index < N - 1) {
        m_boxes[index + 1]->setFocus();
        m_boxes[index + 1]->selectAll();
        updateBoxStyle(index + 1);
    }
    if (isComplete()) {
        m_error = false;
        updateAllStyles();
        emit completed(code());
    }
}

// ── clipboard ─────────────────────────────────────────────────────────────
/**
 * @brief Заполняет поля цифрами из буфера обмена
 *
 * Извлекает только цифры из текста, распределяет по полям
 * и переводит фокус на соответствующее поле.
 *
 * @param text содержимое буфера обмена
 */
void OtpInput::fillFromClipboard(const QString &text)
{
    QString digits;
    for (const QChar &c : text)
        if (c.isDigit()) digits += c;
    if (digits.isEmpty()) return;

    m_error = false;
    for (int i = 0; i < N; ++i) {
        m_boxes[i]->blockSignals(true);
        if (i < digits.length())
            m_boxes[i]->setText(QString(digits[i]));  // QChar → QString явно
        else
            m_boxes[i]->clear();
        m_boxes[i]->blockSignals(false);
        updateBoxStyle(i);
    }

    int focus = qMin(digits.length(), N - 1);
    m_boxes[focus]->setFocus();

    if (isComplete()) emit completed(code());
}

// ── event filter ──────────────────────────────────────────────────────────
/**
 * @brief Фильтр событий для обработки нажатий клавиш в полях ввода
 *
 * Обрабатывает: Escape (возврат), Backspace (удаление/переход),
 * Ctrl+V / Shift+Insert (вставка из буфера), стрелки (навигация),
 * цифры (ввод с автопереходом). Также блокирует контекстное меню.
 *
 * @param obj объект, для которого вызван фильтр
 * @param ev фильтруемое событие
 * @return true если событие обработано и не должно передаваться дальше
 */
bool OtpInput::eventFilter(QObject *obj, QEvent *ev)
{
    int idx = -1;
    for (int i = 0; i < N; ++i)
        if (m_boxes[i] == obj) { idx = i; break; }
    if (idx < 0) return QWidget::eventFilter(obj, ev);

    if (ev->type() == QEvent::FocusIn || ev->type() == QEvent::FocusOut) {
        updateBoxStyle(idx);
        return false;
    }

    if (ev->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(ev);

        if (ke->key() == Qt::Key_Escape) { emit escPressed(); return true; }

        if (ke->key() == Qt::Key_Backspace) {
            if (m_boxes[idx]->text().isEmpty()) {
                if (idx > 0) {
                    m_boxes[idx - 1]->setFocus();
                    m_boxes[idx - 1]->blockSignals(true);
                    m_boxes[idx - 1]->clear();
                    m_boxes[idx - 1]->blockSignals(false);
                    updateBoxStyle(idx - 1);
                }
            } else {
                m_boxes[idx]->blockSignals(true);
                m_boxes[idx]->clear();
                m_boxes[idx]->blockSignals(false);
                updateBoxStyle(idx);
            }
            return true;
        }

        if ((ke->key() == Qt::Key_V        && ke->modifiers() & Qt::ControlModifier) ||
            (ke->key() == Qt::Key_Insert   && ke->modifiers() & Qt::ShiftModifier))
        {
            fillFromClipboard(QApplication::clipboard()->text());
            return true;
        }

        if (ke->key() == Qt::Key_Left  && idx > 0)     { m_boxes[idx-1]->setFocus(); return true; }
        if (ke->key() == Qt::Key_Right && idx < N - 1) { m_boxes[idx+1]->setFocus(); return true; }

        if (ke->text().length() == 1 && ke->text()[0].isDigit()) {
            m_boxes[idx]->blockSignals(true);
            m_boxes[idx]->setText(ke->text());
            m_boxes[idx]->blockSignals(false);
            updateBoxStyle(idx);
            if (idx < N - 1) {
                m_boxes[idx + 1]->setFocus();
                m_boxes[idx + 1]->selectAll();
                updateBoxStyle(idx + 1);
            }
            if (isComplete()) {
                m_error = false;
                updateAllStyles();
                emit completed(code());
            }
            return true;
        }
    }

    if (ev->type() == QEvent::ContextMenu) {
        fillFromClipboard(QApplication::clipboard()->text());
        return true;
    }

    return QWidget::eventFilter(obj, ev);
}
