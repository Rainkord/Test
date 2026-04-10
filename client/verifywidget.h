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
    void verificationSuccess(QString login);
    void backToAuth();

private slots:
    void onVerifyClicked();
    void onBackClicked();
    void onLockTimerFired();
    void onVerifyResponseReceived(const QString &response);

private:
    QLabel      *infoLabel;
    QLineEdit   *codeEdit;
    QPushButton *verifyBtn;
    QLabel      *statusLabel;
    QLabel      *attemptsLabel;
    QPushButton *backBtn;

    int    failedAttempts;
    int    lockLevel;
    QTimer *lockTimer;
    bool   isLocked;
    bool   m_waitingForVerify;
    QString login;

    void setupUI();
    void applyLock(int minutes, const QString &message);
};

#endif // VERIFYWIDGET_H
