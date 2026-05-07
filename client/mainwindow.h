/**
 * @file mainwindow.h
 * @brief Главное окно приложения.
 */

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
 * @class MainWindow
 * @brief Главное окно приложения, управляющее навигацией между экранами.
 *
 * Содержит верхнюю панель с кнопками «Задание» и «Схема»,
 * а также QStackedWidget для переключения между виджетами:
 * авторизации, регистрации, подтверждения входа,
 * восстановления пароля и графика функции.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор. Создаёт UI, подключает сигналы, центрирует окно.
     * @param parent Родительский виджет.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /** @brief Деструктор. */
    ~MainWindow();

private slots:
    /** @brief Переходит на экран регистрации, сбрасывая поля. */
    void onShowRegister();

    /** @brief Переходит на экран авторизации, сбрасывая поля. */
    void onShowAuth();

    /**
     * @brief Переходит на экран подтверждения входа по коду.
     * @param login Логин пользователя, ожидающего верификацию.
     */
    void onShowVerifyAuth(const QString &login);

    /**
     * @brief Вызывается после успешной верификации: открывает график.
     * @param login Логин подтверждённого пользователя.
     */
    void onVerificationSuccess(const QString &login);

    /** @brief Возвращает на экран авторизации, сбрасывая поля. */
    void onBackToAuth();

    /**
     * @brief Вызывается после успешной регистрации: открывает график.
     * @param login Логин зарегистрированного пользователя.
     */
    void onRegistrationSuccess(const QString &login);

    /** @brief Выполняет выход из аккаунта и возврат на экран авторизации. */
    void onLogout();

    /** @brief Переходит на экран восстановления пароля. */
    void onShowReset();

    /** @brief Вызывается после успешного сброса пароля: возврат на авторизацию. */
    void onResetSuccess();

    /** @brief Открывает диалог задания. */
    void onTaskBtnClicked();

    /** @brief Открывает диалог схемы. */
    void onSchemaBtnClicked();

private:
    QWidget        *centralWidget;   ///< Центральный виджет окна.
    QVBoxLayout    *mainVLayout;     ///< Основной вертикальный layout.
    QHBoxLayout    *topBarLayout;    ///< Layout верхней панели.

    QPushButton    *taskBtn;         ///< Кнопка «Задание».
    QPushButton    *schemaBtn;       ///< Кнопка «Схема».
    QLabel         *appTitleLabel;   ///< Заголовок приложения в топ-баре.

    QStackedWidget *stackedWidget;   ///< Стек экранов.

    AuthWidget     *authWidget;      ///< Экран авторизации.
    RegWidget      *regWidget;       ///< Экран регистрации.
    VerifyWidget   *verifyWidget;    ///< Экран подтверждения входа.
    ResetWidget    *resetWidget;     ///< Экран восстановления пароля.
    GraphWidget    *graphWidget;     ///< Экран графика функции.

    /** @brief Создаёт и компонует все элементы UI. */
    void setupUI();

    /** @brief Подключает сигналы дочерних виджетов к слотам MainWindow. */
    void connectSignals();

    static const int IDX_AUTH   = 0; ///< Индекс экрана авторизации.
    static const int IDX_REG    = 1; ///< Индекс экрана регистрации.
    static const int IDX_VERIFY = 2; ///< Индекс экрана верификации.
    static const int IDX_GRAPH  = 3; ///< Индекс экрана графика.
    static const int IDX_RESET  = 4; ///< Индекс экрана восстановления пароля.
};

#endif // MAINWINDOW_H
