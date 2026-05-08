/**
 * @file mainwindow.h
 * @brief Главное окно приложения: навигация между экранами.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class AuthWidget;
class RegWidget;
class VerifyWidget;
class ResetWidget;
class MainAppWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onShowAuth();
    void onShowRegister();
    void onShowVerifyAuth(const QString &login, const QString &codeHash);
    void onAuthSuccess(const QString &login);
    void onShowReset();
    void onResetSuccess();
    void onShowMain(const QString &login);
    void onLogout();

private:
    void setupConnections();

    QStackedWidget *stack;
    AuthWidget     *authWidget;
    RegWidget      *regWidget;
    VerifyWidget   *verifyWidget;
    ResetWidget    *resetWidget;
    MainAppWidget  *mainAppWidget;
};

#endif // MAINWINDOW_H
