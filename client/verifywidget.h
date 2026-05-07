#ifndef VERIFYWIDGET_H
#define VERIFYWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

class VerifyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VerifyWidget(QWidget *parent = nullptr);
    ~VerifyWidget();

    void setLogin(const QString &login);

signals:
    void verificationSuccess(const QString &login);
    void backToAuth();

private slots:
    void onVerifyClicked();
    void onBackClicked();
    void onLockTimerFired();
    void onVerifyResponseReceived(const QString &response);

private:
    QLabel      *promptLabel;
    QLabel      *infoLabel;
    QLineEdit   *codeEdit;
    QPushButton *verifyBtn;
    QLabel      *statusLabel;
    QPushButton *backBtn;

    int     lockLevel;
    QTimer  *lockTimer;
    bool    isLocked;
    QString m_login;

    void setupUI();
};

#endif // VERIFYWIDGET_H
