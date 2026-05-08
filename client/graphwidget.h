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
#include <QTableWidget>
#include <QVector>
#include <QPointF>

/**
 * @class GraphWidget
 * @brief Основной рабочий экран: построение графика кусочной функции.
 */
class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);
    ~GraphWidget();

    void setUserLogin(const QString &login);
    void updateGraph();

signals:
    void logout();

private slots:
    void onParamChanged();
    void onExportClicked();
    void onLogoutClicked();

    void onSliderAChanged(int v);
    void onSliderBChanged(int v);
    void onSliderCChanged(int v);
    void onSpinAChanged(double v);
    void onSpinBChanged(double v);
    void onSpinCChanged(double v);

private:
    // UI elements
    QLabel         *userLabel;
    QLabel         *formulaLabel;
    QLabel         *labelA;
    QLabel         *labelB;
    QLabel         *labelC;
    QSlider        *sliderA;
    QSlider        *sliderB;
    QSlider        *sliderC;
    QDoubleSpinBox *spinA;
    QDoubleSpinBox *spinB;
    QDoubleSpinBox *spinC;
    QTableWidget   *table;
    QPushButton    *exportBtn;
    QPushButton    *logoutBtn;
    QWidget        *leftPanel;
    QWidget        *canvas;

    // State
    double currentA;
    double currentB;
    double currentC;
    bool   blockSliderA, blockSliderB, blockSliderC;
    bool   blockSpinA,   blockSpinB,   blockSpinC;

    QVector<QPointF> pointsBranch1;
    QVector<QPointF> pointsBranch2;
    QVector<QPointF> pointsBranch3;
    QString          userLogin;

    void   setupUI();
    void   setupLeftPanel();
    void   updateFormulaLabel();
    void   fillTable(double a, double b, double c);
    double calculate(double x, double a, double b, double c) const;

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // GRAPHWIDGET_H
