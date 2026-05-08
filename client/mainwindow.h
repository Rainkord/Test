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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onShowRegister();
    void onShowAuth();

    /** @brief Переходит на экран подтверждения: login + SHA-256 хэш кода. */
    void onShowVerifyAuth(const QString &login, const QString &codeHash);

    void onVerificationSuccess(const QString &login);
    void onBackToAuth();
    void onRegistrationSuccess(const QString &login);
    void onLogout();
    void onShowReset();
    void onResetSuccess();

    void onTaskBtnClicked();
    void onSchemaBtnClicked();

private:
    QWidget        *centralWidget;
    QVBoxLayout    *mainVLayout;
    QHBoxLayout    *topBarLayout;

    QPushButton    *taskBtn;
    QPushButton    *schemaBtn;
    QLabel         *appTitleLabel;

    QStackedWidget *stackedWidget;

    AuthWidget     *authWidget;
    RegWidget      *regWidget;
    VerifyWidget   *verifyWidget;
    ResetWidget    *resetWidget;
    GraphWidget    *graphWidget;

    void setupUI();
    void connectSignals();

    static const int IDX_AUTH   = 0;
    static const int IDX_REG    = 1;
    static const int IDX_VERIFY = 2;
    static const int IDX_GRAPH  = 3;
    static const int IDX_RESET  = 4;
};

#endif // MAINWINDOW_H
