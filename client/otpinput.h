/**
 * @file otpinput.h
 * @brief Виджет ввода одноразового кода: 6 отдельных боксов, как в Supercell.
 *
 * Особенности:
 * - Автоматический переход к следующему боксу при вводе цифры.
 * - Backspace возвращает фокус на предыдущий бокс.
 * - Вставка из буфера обмена (Ctrl+V / ПКМ→Вставить) заполняет все боксы.
 * - Сигнал completed(QString) испускается, когда введены все 6 цифр.
 * - setEnabled(bool) блокирует/разблокирует весь виджет.
 * - ESC → сигнал escPressed().
 */

#ifndef OTPINPUT_H
#define OTPINPUT_H

#include <QWidget>

class QLineEdit;

class OtpInput : public QWidget
{
    Q_OBJECT

public:
    explicit OtpInput(QWidget *parent = nullptr);

    /** @brief Возвращает введённый код (0–6 символов, только цифры). */
    QString code() const;

    /** @brief Очищает все боксы и переводит фокус на первый. */
    void clear();

    /** @brief Возвращает true, если все 6 боксов заполнены. */
    bool isComplete() const;

    /** @brief Блокирует / разблокирует весь виджет. */
    void setEnabled(bool enabled);

signals:
    /** @brief Испускается когда все 6 цифр введены. @param code Шестизначный код. */
    void completed(const QString &code);

    /** @brief Испускается при нажатии ESC в любом из боксов. */
    void escPressed();

private slots:
    void onDigitChanged(int index, const QString &text);

private:
    void setupUI();
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void fillFromClipboard(const QString &text);

    static const int N = 6;
    QLineEdit *m_boxes[N];
};

#endif // OTPINPUT_H
