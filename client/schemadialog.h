/**
 * @file schemadialog.h
 * @brief Диалоговое окно со схемой алгоритма вычисления функции.
 */

#ifndef SCHEMADIALOG_H
#define SCHEMADIALOG_H

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>

/**
 * @class FlowchartWidget
 * @brief Виджет для отрисовки блок-схемы алгоритма с помощью QPainter.
 */
class FlowchartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FlowchartWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawRoundedBlock(QPainter &p, int cx, int cy, int w, int h,
                          const QString &text,
                          const QColor &fill, const QColor &border);
    void drawDiamond(QPainter &p, int cx, int cy, int w, int h,
                     const QString &text,
                     const QColor &fill, const QColor &border);
    void drawArrowDown(QPainter &p, int cx, int y1, int y2);
    void drawArrowRight(QPainter &p, int x1, int x2, int y);
    void drawArrowLine(QPainter &p, int x1, int y1, int x2, int y2);
    void drawText(QPainter &p, int cx, int cy, int w, int h, const QString &text);
};

/**
 * @class SchemaDialog
 * @brief Модальный диалог, отображающий блок-схему вычислительного процесса.
 */
class SchemaDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор. Создаёт и компонует содержимое диалога.
     * @param parent Родительский виджет.
     */
    explicit SchemaDialog(QWidget *parent = nullptr);

    /** @brief Деструктор. */
    ~SchemaDialog();

private:
    FlowchartWidget *canvas;   ///< Виджет с отрисованной блок-схемой.
    QPushButton     *closeBtn; ///< Кнопка «Закрыть».

    /** @brief Инициализирует и компонует элементы интерфейса. */
    void setupUI();
};

#endif // SCHEMADIALOG_H
