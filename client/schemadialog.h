#ifndef SCHEMADIALOG_H
#define SCHEMADIALOG_H

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>

/**
 * @brief Виджет отрисовки блок-схемы вычислительного процесса
 *
 * Визуально отображает алгоритм вычисления кусочной функции
 * в виде блок-схемы с блоками начала/конца, ввода, условий,
 * вычислений и вывода результата.
 */
class FlowchartWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор виджета блок-схемы
     * @param parent родительский виджет
     */
    explicit FlowchartWidget(QWidget *parent = nullptr);

protected:
    /**
     * @brief Обработчик отрисовки блок-схемы
     * @param event событие отрисовки
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief Рисует прямоугольный блок со скруглёнными углами
     * @param p объект рисовальщика
     * @param cx центр блока по X
     * @param cy центр блока по Y
     * @param w ширина блока
     * @param h высота блока
     * @param text текст внутри блока
     * @param fill цвет заливки
     * @param border цвет обводки
     */
    void drawRoundedBlock(QPainter &p, int cx, int cy, int w, int h,
                          const QString &text,
                          const QColor &fill, const QColor &border);

    /**
     * @brief Рисует ромбовидный блок условия
     * @param p объект рисовальщика
     * @param cx центр блока по X
     * @param cy центр блока по Y
     * @param w ширина блока
     * @param h высота блока
     * @param text текст условия внутри ромба
     * @param fill цвет заливки
     * @param border цвет обводки
     */
    void drawDiamond(QPainter &p, int cx, int cy, int w, int h,
                     const QString &text,
                     const QColor &fill, const QColor &border);

    /**
     * @brief Рисует стрелку вниз
     * @param p объект рисовальщика
     * @param cx координата X начала и конца стрелки
     * @param y1 координата Y начала стрелки
     * @param y2 координата Y конца стрелки
     */
    void drawArrowDown(QPainter &p, int cx, int y1, int y2);

    /**
     * @brief Рисует стрелку вправо
     * @param p объект рисовальщика
     * @param x1 координата X начала стрелки
     * @param x2 координата X конца стрелки
     * @param y координата Y линии стрелки
     */
    void drawArrowRight(QPainter &p, int x1, int x2, int y);

    /**
     * @brief Рисует линию-стрелку между двумя точками
     * @param p объект рисовальщика
     * @param x1 координата X начала
     * @param y1 координата Y начала
     * @param x2 координата X конца
     * @param y2 координата Y конца
     */
    void drawArrowLine(QPainter &p, int x1, int y1, int x2, int y2);

    /**
     * @brief Рисует текстовую подпись (например, «Да»/«Нет»)
     * @param p объект рисовальщика
     * @param cx центр текста по X
     * @param cy центр текста по Y
     * @param w ширина области текста
     * @param h высота области текста
     * @param text выводимый текст
     */
    void drawText(QPainter &p, int cx, int cy, int w, int h, const QString &text);
};

/**
 * @brief Диалоговое окно с блок-схемой вычислительного процесса
 *
 * Отображает блок-схему алгоритма вычисления кусочной функции.
 * Содержит виджет FlowchartWidget и кнопку закрытия.
 */
class SchemaDialog : public QDialog
{
    Q_OBJECT

public:

    /**
     * @brief Конструктор диалога блок-схемы
     * @param parent родительский виджет
     */
    explicit SchemaDialog(QWidget *parent = nullptr);

    /// Деструктор диалога
    ~SchemaDialog();

private:
    FlowchartWidget *canvas;    ///< Виджет блок-схемы
    QPushButton     *closeBtn;  ///< Кнопка «Закрыть»

    /// Инициализация пользовательского интерфейса диалога
    void setupUI();
};

#endif // SCHEMADIALOG_H
