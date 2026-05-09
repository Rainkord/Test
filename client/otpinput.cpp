/**
 * @file otpinput.cpp
 */

#include "otpinput.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QFocusEvent>

// ── стили (совпадают с GH dark palette остальных виджетов) ────────────────
#define GH_BG       "#0d1117"
#define GH_BORDER   "#30363d"
#define GH_TEXT     "#e6edf3"
#define GH_BLUE     "#388bfd"
#define FONT_FAMILY "Segoe UI"

static QString boxStyle()
{
    return QString(
        "QLineEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1.5px solid %3;"
        "  border-radius: 8px;"
        "  font-family: '%4';"
        "  font-size: 20pt;"
        "  font-weight: bold;"
        "}"
        "QLineEdit:focus { border-color: %5; }"
        "QLineEdit:disabled { color: #555; border-color: #21262d; }"
    ).arg(GH_BG, GH_TEXT, GH_BORDER, FONT_FAMILY, GH_BLUE);
}

// ── OtpInput ──────────────────────────────────────────────────────────────
OtpInput::OtpInput(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void OtpInput::setupUI()
{
    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(8);

    for (int i = 0; i < N; ++i) {
        m_boxes[i] = new QLineEdit(this);
        m_boxes[i]->setMaxLength(1);
        m_boxes[i]->setFixedSize(48, 56);
        m_boxes[i]->setAlignment(Qt::AlignCenter);
        m_boxes[i]->setStyleSheet(boxStyle());
        m_boxes[i]->installEventFilter(this);

        // Принимать только цифры
        connect(m_boxes[i], &QLineEdit::textChanged, this, [this, i](const QString &t) {
            // Фильтруем нецифровые символы
            if (!t.isEmpty() && !t[0].isDigit()) {
                m_boxes[i]->clear();
                return;
            }
            onDigitChanged(i, t);
        });

        row->addWidget(m_boxes[i]);
    }
}

QString OtpInput::code() const
{
    QString s;
    for (int i = 0; i < N; ++i)
        s += m_boxes[i]->text();
    return s;
}

void OtpInput::clear()
{
    for (int i = 0; i < N; ++i)
        m_boxes[i]->clear();
    if (isEnabled())
        m_boxes[0]->setFocus();
}

bool OtpInput::isComplete() const
{
    for (int i = 0; i < N; ++i)
        if (m_boxes[i]->text().isEmpty()) return false;
    return true;
}

void OtpInput::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);
    for (int i = 0; i < N; ++i)
        m_boxes[i]->setEnabled(enabled);
    if (enabled)
        m_boxes[0]->setFocus();
}

void OtpInput::onDigitChanged(int index, const QString &text)
{
    if (text.length() == 1 && index < N - 1) {
        m_boxes[index + 1]->setFocus();
        m_boxes[index + 1]->selectAll();
    }

    if (isComplete())
        emit completed(code());
}

void OtpInput::fillFromClipboard(const QString &text)
{
    // Убираем всё, кроме цифр
    QString digits;
    for (const QChar &c : text)
        if (c.isDigit()) digits += c;

    if (digits.isEmpty()) return;

    for (int i = 0; i < N && i < digits.length(); ++i) {
        m_boxes[i]->setText(digits[i]);
    }
    // Фокус на следующий незаполненный или на последний
    int focus = qMin(digits.length(), N - 1);
    m_boxes[focus]->setFocus();

    if (isComplete())
        emit completed(code());
}

bool OtpInput::eventFilter(QObject *obj, QEvent *ev)
{
    // Определяем индекс бокса
    int idx = -1;
    for (int i = 0; i < N; ++i)
        if (m_boxes[i] == obj) { idx = i; break; }
    if (idx < 0) return QWidget::eventFilter(obj, ev);

    if (ev->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(ev);

        // ESC → сигнал назад
        if (ke->key() == Qt::Key_Escape) {
            emit escPressed();
            return true;
        }

        // Backspace: очищаем текущий, если пуст — переходим назад
        if (ke->key() == Qt::Key_Backspace) {
            if (m_boxes[idx]->text().isEmpty() && idx > 0) {
                m_boxes[idx - 1]->setFocus();
                m_boxes[idx - 1]->clear();
            } else {
                m_boxes[idx]->clear();
            }
            return true;
        }

        // Ctrl+V или Shift+Insert → вставка
        if ((ke->key() == Qt::Key_V &&
             ke->modifiers() & Qt::ControlModifier) ||
            (ke->key() == Qt::Key_Insert &&
             ke->modifiers() & Qt::ShiftModifier)) {
            fillFromClipboard(QApplication::clipboard()->text());
            return true;
        }

        // Стрелки: ← / →
        if (ke->key() == Qt::Key_Left && idx > 0) {
            m_boxes[idx - 1]->setFocus();
            return true;
        }
        if (ke->key() == Qt::Key_Right && idx < N - 1) {
            m_boxes[idx + 1]->setFocus();
            return true;
        }
    }

    // Перехватываем contextMenu-вставку через paste
    if (ev->type() == QEvent::InputMethod) {
        return QWidget::eventFilter(obj, ev);
    }

    return QWidget::eventFilter(obj, ev);
}
