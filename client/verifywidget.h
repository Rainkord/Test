#ifndef VERIFYWIDGET_H
#define VERIFYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

class VerifyWidget : public QWidget
{
    Q_OBJECT

public:
    enum Mode { AuthMode, RegMode };

    explicit VerifyWidget(QWidget *parent = nullptr);
    ~VerifyWidget();

    void setLogin(const QString &login);
    void setMode(Mode m);

signals:
    void verificationSuccess(QString login);
    void backToAuth();

private slots:
    void onVerifyClicked();
    void onBackClicked();
    void onLockTimerFired();

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
    QString login;
    Mode   mode;

    void setupUI();
    void applyLock(int minutes, const QString &message);
};

#endif // VERIFYWIDGET_H
