#ifndef AUTHWIDGET_H
#define AUTHWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class AuthWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AuthWidget(QWidget *parent = nullptr);

signals:
    void loginSuccess(QString login);
    void showRegister();

private slots:
    void onLoginClicked();

private:
    QLineEdit  *loginEdit;
    QLineEdit  *passwordEdit;
    QPushButton *loginBtn;
    QPushButton *registerBtn;
    QLabel      *statusLabel;
};

#endif // AUTHWIDGET_H
