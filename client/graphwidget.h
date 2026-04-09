#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QPointF>

class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);
    void setUserLogin(const QString &login);

signals:
    void logout();

private slots:
    void onPlotClicked();
    void onGetTaskClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel        *userLabel;
    QLabel        *taskLabel;
    QDoubleSpinBox *xMinSpin;
    QDoubleSpinBox *xMaxSpin;
    QDoubleSpinBox *stepSpin;
    QPushButton   *plotBtn;
    QPushButton   *logoutBtn;
    QPushButton   *taskBtn;

    QVector<QPointF> points;
};

#endif // GRAPHWIDGET_H
