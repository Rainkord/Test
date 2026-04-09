#ifndef REGWIDGET_H
#define REGWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class RegWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegWidget(QWidget *parent = nullptr);

signals:
    void registrationSuccess();
    void showAuth();

private slots:
    void onRegisterClicked();

private:
    QLineEdit  *loginEdit;
    QLineEdit  *passwordEdit;
    QLineEdit  *confirmPasswordEdit;
    QLineEdit  *emailEdit;
    QPushButton *registerBtn;
    QPushButton *backBtn;
    QLabel      *statusLabel;
};

#endif // REGWIDGET_H
