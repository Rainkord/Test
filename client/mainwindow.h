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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onShowRegister();
    void onShowAuth();
    void onShowVerifyAuth(const QString &login);
    void onVerificationSuccess(const QString &login);
    void onBackToAuth();
    void onRegistrationSuccess();
    void onLogout();
    void onShowReset();           // NEW
    void onResetSuccess();        // NEW

    void onTaskBtnClicked();
    void onSchemaBtnClicked();

private:
    QWidget         *centralWidget;
    QVBoxLayout     *mainVLayout;
    QHBoxLayout     *topBarLayout;

    QPushButton     *taskBtn;
    QPushButton     *schemaBtn;
    QLabel          *appTitleLabel;

    QStackedWidget  *stackedWidget;

    AuthWidget      *authWidget;
    RegWidget       *regWidget;
    VerifyWidget    *verifyWidget;
    ResetWidget     *resetWidget;     // NEW
    GraphWidget     *graphWidget;

    void setupUI();
    void connectSignals();

    static const int IDX_AUTH   = 0;
    static const int IDX_REG    = 1;
    static const int IDX_VERIFY = 2;
    static const int IDX_GRAPH  = 3;
    static const int IDX_RESET  = 4;  // NEW
};

#endif // MAINWINDOW_H
