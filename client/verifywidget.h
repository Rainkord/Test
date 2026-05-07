/**
 * @file verifywidget.h
 * @brief Виджет подтверждения входа по одноразовому коду.
 */

#ifndef VERIFYWIDGET_H
#define VERIFYWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

/**
 * @class VerifyWidget
 * @brief Экран двухфакторной верификации при входе в систему.
 *
 * После ввода корректных логина и пароля пользователь направляется
 * на данный экран для ввода одноразового кода, отправленного на email.
 * Поддерживает прогрессивную блокировку при превышении числа попыток.
 */
class VerifyWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit VerifyWidget(QWidget *parent = nullptr);

    /** @brief Деструктор. */
    ~VerifyWidget();

    /**
     * @brief Устанавливает логин пользователя, проходящего верификацию.
     * @param login Логин пользователя.
     */
    void setLogin(const QString &login);

signals:
    /**
     * @brief Испускается при успешной верификации.
     * @param login Логин подтверждённого пользователя.
     */
    void verificationSuccess(const QString &login);

    /** @brief Испускается при нажатии кнопки «Назад». */
    void backToAuth();

private slots:
    /** @brief Отправляет введённый код на сервер. */
    void onVerifyClicked();

    /** @brief Возвращает пользователя на экран авторизации. */
    void onBackClicked();

    /** @brief Вызывается по истечении таймера блокировки. */
    void onLockTimerFired();

    /**
     * @brief Обрабатывает ответ сервера на запрос верификации.
     * @param response Строка ответа от сервера.
     */
    void onVerifyResponseReceived(const QString &response);

private:
    QLabel      *promptLabel;  ///< Метка с заголовком экрана.
    QLabel      *infoLabel;    ///< Информационное сообщение (на какой email отправлен код).
    QLineEdit   *codeEdit;     ///< Поле ввода кода.
    QPushButton *verifyBtn;    ///< Кнопка «Подтвердить».
    QLabel      *statusLabel;  ///< Метка статуса/ошибки.
    QPushButton *backBtn;      ///< Кнопка «Назад».

    int     lockLevel;          ///< Уровень блокировки.
    QTimer  *lockTimer;         ///< Таймер снятия блокировки.
    bool    isLocked;           ///< Флаг активной блокировки.
    bool    m_waitingForVerify; ///< Флаг ожидания ответа сервера.
    QString m_login;            ///< Логин текущего пользователя.

    /** @brief Инициализирует и компонует элементы интерфейса. */
    void setupUI();
};

#endif // VERIFYWIDGET_H
