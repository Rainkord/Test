#ifndef OTPINPUT_H
#define OTPINPUT_H

#include <QWidget>

class QLineEdit;

/**
 * @brief Виджет ввода одноразового кода (OTP) из 6 цифр
 *
 * Представляет собой шесть отдельных полей ввода для каждой цифры кода.
 * Поддерживает автопереход между полями, вставку из буфера обмена,
 * навигацию стрелками и обработку Backspace.
 */
class OtpInput : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета OTP-ввода
     * @param parent родительский виджет
     */
    explicit OtpInput(QWidget *parent = nullptr);

    /**
     * @brief Возвращает текущий введённый шестизначный код
     * @return строка из 6 цифр
     */
    QString code()      const;

    /// Очищает все поля ввода и сбрасывает флаг ошибки
    void    clear();

    /**
     * @brief Проверяет, заполнены ли все 6 полей
     * @return true если все поля содержат цифру
     */
    bool    isComplete() const;

    /**
     * @brief Включает или отключает виджет
     * @param enabled true для включения, false для отключения
     */
    void    setEnabled(bool enabled);

    /**
     * @brief Устанавливает режим ошибки (красная подсветка полей)
     * @param error true для включения режима ошибки
     */
    void    setError(bool error);

signals:
    /// Испускается при заполнении всех 6 полей
    void completed(const QString &code);

    /// Испускается при нажатии клавиши Escape
    void escPressed();

private slots:
    /**
     * @brief Обработчик изменения цифры в конкретном поле
     * @param index индекс поля (0–5)
     * @param text содержимое поля после изменения
     */
    void onDigitChanged(int index, const QString &text);

private:
    /// Инициализация пользовательского интерфейса (создание 6 полей)
    void setupUI();

    /**
     * @brief Обновляет стиль одного поля в зависимости от его состояния
     * @param index индекс поля (0–5)
     */
    void updateBoxStyle(int index);

    /// Обновляет стили всех 6 полей
    void updateAllStyles();

    /**
     * @brief Фильтр событий для обработки нажатий клавиш в полях
     * @param obj объект, для которого вызван фильтр
     * @param ev фильтруемое событие
     * @return true если событие обработано и не должно передаваться дальше
     */
    bool eventFilter(QObject *obj, QEvent *ev) override;

    /**
     * @brief Заполняет поля цифрами из буфера обмена
     * @param text содержимое буфера обмена
     */
    void fillFromClipboard(const QString &text);

    static const int N = 6;          ///< Количество полей OTP-кода
    QLineEdit *m_boxes[N];           ///< Массив полей ввода цифр
    bool       m_error = false;      ///< Флаг режима ошибки
};

#endif // OTPINPUT_H
