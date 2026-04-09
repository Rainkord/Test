#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "authwidget.h"
#include "regwidget.h"
#include "graphwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void showGraph(const QString &login);
    void showAuth();
    void showRegister();
    void handleLogout();

private:
    QStackedWidget *stackedWidget;
    AuthWidget     *authWidget;
    RegWidget      *regWidget;
    GraphWidget    *graphWidget;
};

#endif // MAINWINDOW_H
