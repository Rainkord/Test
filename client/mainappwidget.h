#ifndef MAINAPPWIDGET_H
#define MAINAPPWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;
class QStackedWidget;

/**
 * @class MainAppWidget
 * @brief Главный экран приложения после успешного входа.
 *
 * Отображает логин пользователя и предоставляет кнопку выхода.
 * При нажатии «Выйти» испускает сигнал logout().
 */
class MainAppWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainAppWidget(QWidget *parent = nullptr);
    ~MainAppWidget();

    /** @brief Устанавливает логин текущего пользователя в заголовок. */
    void setLogin(const QString &login);

signals:
    /** @brief Испускается при нажатии кнопки «Выйти». */
    void logout();

private slots:
    void onLogoutClicked();

private:
    void setupUI();

    QLabel      *welcomeLabel;
    QPushButton *logoutBtn;
    QStackedWidget *contentStack;
};

#endif // MAINAPPWIDGET_H
