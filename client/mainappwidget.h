#ifndef MAINAPPWIDGET_H
#define MAINAPPWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;
class QStackedWidget;

/**
 * @brief Основной виджет приложения после успешного входа
 *
 * Содержит верхнюю панель с приветствием и кнопкой выхода,
 * а также стековый виджет для размещения основного контента.
 */
class MainAppWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор главного виджета приложения
     * @param parent родительский виджет
     */
    explicit MainAppWidget(QWidget *parent = nullptr);

    /// Деструктор виджета
    ~MainAppWidget();

    /**
     * @brief Устанавливает логин пользователя для отображения в приветствии
     * @param login логин пользователя
     */
    void setLogin(const QString &login);

signals:
    /// Испускается при нажатии кнопки «Выйти»
    void logout();

private slots:
    /// Обработчик нажатия кнопки «Выйти»
    void onLogoutClicked();

private:
    /// Инициализация пользовательского интерфейса
    void setupUI();

    QLabel         *welcomeLabel;   ///< Метка приветствия с логином
    QPushButton    *logoutBtn;      ///< Кнопка «Выйти»
    QStackedWidget *contentStack;   ///< Стековый виджет для контента
};

#endif // MAINAPPWIDGET_H
