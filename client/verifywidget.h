#ifndef VERIFYWIDGET_H
#define VERIFYWIDGET_H

#include <QWidget>
#include <QKeyEvent>

class OtpInput;
class QPushButton;
class QLabel;
class QTimer;

/**
 * @brief Виджет двухфакторной верификации по OTP-коду
 *
 * Экран ввода одноразового кода подтверждения, отправленного пользователю
 * при входе в аккаунт. Поддерживает блокировку при превышении лимита попыток.
 */
class VerifyWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета верификации
     * @param parent родительский виджет
     */
    explicit VerifyWidget(QWidget *parent = nullptr);

    /// Деструктор виджета
    ~VerifyWidget();

    /**
     * @brief Устанавливает логин и хэш кода для верификации
     * @param login логин пользователя
     * @param codeHash SHA-256 хэш ожидаемого OTP-кода
     */
    void setLogin(const QString &login, const QString &codeHash);

    /// Очищает все поля и сбрасывает состояние виджета
    void clearFields();

signals:
    /// Испускается при успешной верификации OTP-кода
    void verificationSuccess(const QString &login);

    /// Испускается при возврате на экран авторизации
    void backToAuth();

protected:
    /**
     * @brief Обработчик нажатия клавиш
     * @param e событие нажатия клавиши
     */
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    /// Обработчик нажатия кнопки «Подтвердить»
    void onVerifyClicked();

    /// Обработчик нажатия кнопки «Назад»
    void onBackClicked();

    /**
     * @brief Обработчик завершения ввода OTP-кода
     * @param code введённый шестизначный код
     */
    void onCodeCompleted(const QString &code);

    /// Обработчик срабатывания таймера блокировки
    void onLockTimerFired();

    /**
     * @brief Обработчик ответа сервера на запрос верификации
     * @param response строка ответа от сервера
     */
    void onVerifyResponseReceived(const QString &response);

private:
    /// Инициализация пользовательского интерфейса
    void setupUI();

    /**
     * @brief Блокирует ввод на указанное время
     * @param seconds количество секунд блокировки
     * @param message сообщение, отображаемое при блокировке
     */
    void applyLock(int seconds, const QString &message);

    /// Обновляет состояние кнопки «Подтвердить» (активна/неактивна)
    void updateVerifyBtn();

    OtpInput    *otpInput;       ///< Виджет ввода OTP-кода
    QPushButton *verifyBtn;      ///< Кнопка подтверждения
    QPushButton *backBtn;        ///< Кнопка возврата
    QLabel      *statusLabel;    ///< Метка статуса (ошибка/успех)
    QLabel      *hintLabel;      ///< Метка-подсказка
    QTimer      *lockTimer;      ///< Таймер блокировки при превышении попыток

    bool    isLocked;            ///< Флаг: ввод заблокирован
    QString m_login;             ///< Логин текущего пользователя
    QString m_codeHash;          ///< Хэш ожидаемого OTP-кода
    int     m_attemptsLeft = 3;  ///< Оставшееся количество попыток ввода
};

#endif // VERIFYWIDGET_H
