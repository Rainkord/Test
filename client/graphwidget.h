#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QVector>
#include <QPointF>

/**
 * @brief Виджет графического отображения кусочной функции
 *
 * Позволяет визуализировать функцию f(x) с тремя ветвями на графике,
 * управлять параметрами a, b, c через слайдеры и спинбоксы.
 * Получает данные от сервера или вычисляет локально при недоступности сервера.
 * Отображает таблицу значений и легенду.
 */
class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета графика
     * @param parent родительский виджет
     */
    explicit GraphWidget(QWidget *parent = nullptr);

    /// Деструктор виджета
    ~GraphWidget();

    /**
     * @brief Устанавливает логин текущего пользователя для отображения
     * @param login логин пользователя
     */
    void setUserLogin(const QString &login);

    /// Пересчитывает точки графика и таблицу, перерисовывает виджет
    void updateGraph();

signals:
    /// Испускается при нажатии кнопки «Выйти из аккаунта»
    void logout();

private slots:
    /// Обработчик нажатия кнопки «Выйти из аккаунта»
    void onLogoutClicked();

    /**
     * @brief Обработчик изменения слайдера параметра a
     * @param v целочисленное значение слайдера
     */
    void onSliderAChanged(int v);

    /**
     * @brief Обработчик изменения слайдера параметра b
     * @param v целочисленное значение слайдера
     */
    void onSliderBChanged(int v);

    /**
     * @brief Обработчик изменения слайдера параметра c
     * @param v целочисленное значение слайдера
     */
    void onSliderCChanged(int v);

    /**
     * @brief Обработчик изменения спинбокса параметра a
     * @param v вещественное значение параметра
     */
    void onSpinAChanged(double v);

    /**
     * @brief Обработчик изменения спинбокса параметра b
     * @param v вещественное значение параметра
     */
    void onSpinBChanged(double v);

    /**
     * @brief Обработчик изменения спинбокса параметра c
     * @param v вещественное значение параметра
     */
    void onSpinCChanged(double v);

private:
    // UI elements
    QLabel         *userLabel;       ///< Метка с логином пользователя
    QLabel         *formulaLabel;    ///< Метка с формулой функции
    QLabel         *labelA;          ///< Подпись параметра a
    QLabel         *labelB;          ///< Подпись параметра b
    QLabel         *labelC;          ///< Подпись параметра c
    QSlider        *sliderA;         ///< Слайдер параметра a
    QSlider        *sliderB;         ///< Слайдер параметра b
    QSlider        *sliderC;         ///< Слайдер параметра c
    QDoubleSpinBox *spinA;           ///< Спинбокс параметра a
    QDoubleSpinBox *spinB;           ///< Спинбокс параметра b
    QDoubleSpinBox *spinC;           ///< Спинбокс параметра c
    QTableWidget   *table;           ///< Таблица значений x и F(x)
    QPushButton    *exportBtn;       ///< Кнопка экспорта (не используется)
    QPushButton    *logoutBtn;       ///< Кнопка «Выйти из аккаунта»
    QWidget        *leftPanel;       ///< Левая панель управления
    QWidget        *canvas;          ///< Область отрисовки графика

    // State
    double currentA;                 ///< Текущее значение параметра a
    double currentB;                 ///< Текущее значение параметра b
    double currentC;                 ///< Текущее значение параметра c
    bool   blockSliderA;             ///< Флаг блокировки обработчика слайдера a
    bool   blockSliderB;             ///< Флаг блокировки обработчика слайдера b
    bool   blockSliderC;             ///< Флаг блокировки обработчика слайдера c
    bool   blockSpinA;               ///< Флаг блокировки обработчика спинбокса a
    bool   blockSpinB;               ///< Флаг блокировки обработчика спинбокса b
    bool   blockSpinC;               ///< Флаг блокировки обработчика спинбокса c

    QVector<QPointF> pointsBranch1;  ///< Точки первой ветви (x < -2)
    QVector<QPointF> pointsBranch2;  ///< Точки второй ветви (-2 <= x < 2)
    QVector<QPointF> pointsBranch3;  ///< Точки третьей ветви (x >= 2)
    QString          userLogin;      ///< Логин текущего пользователя

    /// Создание и компоновка основного интерфейса
    void   setupUI();

    /// Создание левой панели с элементами управления
    void   setupLeftPanel();

    /// Обновление метки с формулой функции
    void   updateFormulaLabel();

    /**
     * @brief Заполняет таблицу значениями функции для целых x от -10 до 10
     * @param a значение параметра a
     * @param b значение параметра b
     * @param c значение параметра c
     */
    void   fillTable(double a, double b, double c);

    /**
     * @brief Вычисляет значение кусочной функции
     * @param x аргумент функции
     * @param a параметр a
     * @param b параметр b
     * @param c параметр c
     * @return значение функции f(x)
     */
    double calculate(double x, double a, double b, double c) const;

protected:
    /**
     * @brief Обработчик отрисовки графика
     * @param event событие отрисовки
     */
    void paintEvent(QPaintEvent *event) override;
};

#endif // GRAPHWIDGET_H
