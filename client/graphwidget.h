/**
 * @file graphwidget.h
 * @brief Виджет построения и отображения графика функции.
 */

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QVector>
#include <QPointF>

/**
 * @class GraphWidget
 * @brief Основной рабочий экран: построение графика кусочной функции.
 *
 * Позволяет пользователю задавать параметры a, b, c, диапазон
 * отображения и шаг дискретизации. Отрисовывает график с помощью
 * кастомного paintEvent, поддерживает экспорт в PNG и выход из аккаунта.
 */
class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit GraphWidget(QWidget *parent = nullptr);

    /** @brief Деструктор. */
    ~GraphWidget();

    /**
     * @brief Сохраняет логин текущего пользователя для отображения.
     * @param login Логин пользователя.
     */
    void setUserLogin(const QString &login);

    /** @brief Пересчитывает точки и перерисовывает график. */
    void updateGraph();

signals:
    /** @brief Испускается при нажатии кнопки «Выйти». */
    void logout();

private slots:
    /** @brief Обновляет график при изменении любого параметра. */
    void onParamChanged();

    /** @brief Сохраняет текущий график в PNG-файл. */
    void onExportClicked();

    /** @brief Выполняет выход из аккаунта. */
    void onLogoutClicked();

private:
    QLabel        *userLabel;       ///< Метка с именем текущего пользователя.
    QDoubleSpinBox *spinA;          ///< Спинбокс параметра a.
    QDoubleSpinBox *spinB;          ///< Спинбокс параметра b.
    QDoubleSpinBox *spinC;          ///< Спинбокс параметра c.
    QDoubleSpinBox *spinXMin;       ///< Спинбокс минимума оси X.
    QDoubleSpinBox *spinXMax;       ///< Спинбокс максимума оси X.
    QDoubleSpinBox *spinStep;       ///< Спинбокс шага дискретизации.
    QCheckBox     *chkGrid;         ///< Чекбокс отображения сетки.
    QPushButton   *exportBtn;       ///< Кнопка экспорта в PNG.
    QPushButton   *logoutBtn;       ///< Кнопка выхода из аккаунта.
    QWidget       *canvas;          ///< Область отрисовки графика.

    QVector<QPointF> m_points;      ///< Вычисленные точки графика.
    QString          m_login;       ///< Логин текущего пользователя.

    /** @brief Инициализирует и компонует элементы интерфейса. */
    void setupUI();

    /**
     * @brief Вычисляет значение кусочной функции в точке x.
     * @param x Аргумент функции.
     * @param a Параметр a.
     * @param b Параметр b.
     * @param c Параметр c.
     * @return Значение функции f(x).
     */
    double evalFunction(double x, double a, double b, double c) const;
};

#endif // GRAPHWIDGET_H
