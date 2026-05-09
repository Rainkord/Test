/**
 * @file otpinput.h
 * @brief Виджет OTP-ввода — 6 боксов в стиле Supercell.
 *
 * - Авто-переход к следующему боксу при вводе цифры.
 * - При вводе в заполненный бокс — замена символа и переход дальше.
 * - Backspace: очищает текущий; если пустой — возвращается назад.
 * - Ctrl+V / Shift+Insert / ПКМ→Вставить — заполняет все боксы из буфера.
 * - Стрелки ← / → перемещают фокус.
 * - ESC → сигнал escPressed().
 * - Визуальные состояния: пустой / в фокусе / заполнен / ошибка / заблокирован.
 * - setError(bool) окрашивает рамки в красный.
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

    QString code()      const;
    void    clear();
    bool    isComplete() const;
    void    setEnabled(bool enabled);
    void    setError(bool error);   ///< включает/выключает красное состояние

signals:
    void completed(const QString &code);
    void escPressed();

private slots:
    void onDigitChanged(int index, const QString &text);

private:
    void setupUI();
    void updateBoxStyle(int index);
    void updateAllStyles();
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void fillFromClipboard(const QString &text);

    static const int N = 6;
    QLineEdit *m_boxes[N];
    bool       m_error = false;
};

#endif // OTPINPUT_H
