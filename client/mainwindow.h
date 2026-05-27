#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "authwidget.h"
#include "regwidget.h"
#include "verifywidget.h"
#include "resetwidget.h"
#include "graphwidget.h"

/**
 * @brief Главное окно приложения "График функции"
 * 
 * Управляет навигацией между экранами: авторизация, регистрация,
 * верификация, восстановление пароля и просмотр графика.
 * Использует QStackedWidget для переключения между виджетами.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор главного окна
     * 
     * @param parent Родительский виджет
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор главного окна
     */
    ~MainWindow();

private slots:
    /// @brief Слот для переключения на экран регистрации
    void onShowRegister();

    /// @brief Слот для переключения на экран авторизации
    void onShowAuth();

    /// @brief Слот для переключения на экран верификации при входе
    /// @param login Логин пользователя
    /// @param codeHash Хэш кода подтверждения
    void onShowVerifyAuth(const QString &login, const QString &codeHash);

    /// @brief Слот, вызываемый при успешной верификации
    /// @param login Логин пользователя
    void onVerificationSuccess(const QString &login);

    /// @brief Слот для возврата на экран авторизации
    void onBackToAuth();

    /// @brief Слот, вызываемый при успешной регистрации
    /// @param login Логин зарегистрированного пользователя
    void onRegistrationSuccess(const QString &login);

    /// @brief Слот для выхода из аккаунта
    void onLogout();

    /// @brief Слот для переключения на экран восстановления пароля
    void onShowReset();

    /// @brief Слот, вызываемый при успешном сбросе пароля
    void onResetSuccess();

    /// @brief Слот для открытия диалога задания
    void onTaskBtnClicked();

    /// @brief Слот для открытия диалога схемы
    void onSchemaBtnClicked();

private:
    /// Центральный виджет окна
    QWidget        *centralWidget;

    /// Основной вертикальный контейнер
    QVBoxLayout    *mainVLayout;

    /// Горизонтальный контейнер верхней панели
    QHBoxLayout    *topBarLayout;

    /// Кнопка "Задание" в верхней панели
    QPushButton    *taskBtn;

    /// Кнопка "Схема" в верхней панели
    QPushButton    *schemaBtn;

    /// Метка с заголовком приложения
    QLabel         *appTitleLabel;

    /// Стековый виджет для переключения между экранами
    QStackedWidget *stackedWidget;

    /// Виджет авторизации
    AuthWidget     *authWidget;

    /// Виджет регистрации
    RegWidget      *regWidget;

    /// Виджет верификации
    VerifyWidget   *verifyWidget;

    /// Виджет восстановления пароля
    ResetWidget    *resetWidget;

    /// Виджет графика
    GraphWidget    *graphWidget;

    /**
     * @brief Настройка пользовательского интерфейса
     * 
     * Создаёт все виджеты, кнопки и компоновки, настраивает стили.
     */
    void setupUI();

    /**
     * @brief Подключение сигналов к слотам
     * 
     * Устанавливает все соединения между виджетами и слотами навигации.
     */
    void connectSignals();

    /// Индекс экрана авторизации в стековом виджете
    static const int IDX_AUTH   = 0;

    /// Индекс экрана регистрации в стековом виджете
    static const int IDX_REG    = 1;

    /// Индекс экрана верификации в стековом виджете
    static const int IDX_VERIFY = 2;

    /// Индекс экрана графика в стековом виджете
    static const int IDX_GRAPH  = 3;

    /// Индекс экрана восстановления пароля в стековом виджете
    static const int IDX_RESET  = 4;
};

#endif // MAINWINDOW_H
