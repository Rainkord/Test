#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QVector>
#include <QPointF>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>

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

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onSliderAChanged(int value);
    void onSliderBChanged(int value);
    void onSliderCChanged(int value);
    void onSpinAChanged(double value);
    void onSpinBChanged(double value);
    void onSpinCChanged(double value);
    void onLogoutClicked();

private:
    QLabel         *formulaLabel;

    QLabel         *labelA;
    QSlider        *sliderA;
    QDoubleSpinBox *spinA;

    QLabel         *labelB;
    QSlider        *sliderB;
    QDoubleSpinBox *spinB;

    QLabel         *labelC;
    QSlider        *sliderC;
    QDoubleSpinBox *spinC;

    QTableWidget   *table;
    QPushButton    *logoutBtn;
    QLabel         *userLabel;

    QVector<QPointF> pointsBranch1;
    QVector<QPointF> pointsBranch2;
    QVector<QPointF> pointsBranch3;

    double currentA;
    double currentB;
    double currentC;
    QString userLogin;

    QWidget *leftPanel;
    // Увеличена с 320 до 420
    static const int LEFT_PANEL_WIDTH = 420;

    void setupUI();
    void setupLeftPanel();
    void updateFormulaLabel();
    void fillTable(double a, double b, double c);
    double calculate(double x, double a, double b, double c) const;
    void drawGrid(QPainter &painter, int drawX, int drawY, int drawW, int drawH,
                  double xMin, double xMax, double yMin, double yMax,
                  double scaleX, double scaleY);
    void drawAxes(QPainter &painter, int drawX, int drawY, int drawW, int drawH,
                  double xMin, double xMax, double yMin, double yMax,
                  double scaleX, double scaleY);
    void drawBranch(QPainter &painter, const QVector<QPointF> &pts,
                    int drawX, int drawY, int drawH,
                    double xMin, double yMin,
                    double scaleX, double scaleY,
                    const QColor &color);
    void drawLegend(QPainter &painter, int drawX, int drawY);

    bool blockSliderA;
    bool blockSliderB;
    bool blockSliderC;
    bool blockSpinA;
    bool blockSpinB;
    bool blockSpinC;
};

#endif // GRAPHWIDGET_H
