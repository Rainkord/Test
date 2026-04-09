#include "graphwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <cmath>
#include <algorithm>

static const int MARGIN = 60; // px padding around the graph area

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // ---- Top controls bar ----
    QHBoxLayout *ctrlLayout = new QHBoxLayout();
    ctrlLayout->setSpacing(10);

    userLabel = new QLabel("Пользователь: —", this);
    ctrlLayout->addWidget(userLabel);

    ctrlLayout->addStretch();

    ctrlLayout->addWidget(new QLabel("X мин:", this));
    xMinSpin = new QDoubleSpinBox(this);
    xMinSpin->setRange(-1000.0, 1000.0);
    xMinSpin->setValue(-5.0);
    xMinSpin->setDecimals(2);
    xMinSpin->setMinimumWidth(80);
    ctrlLayout->addWidget(xMinSpin);

    ctrlLayout->addWidget(new QLabel("X макс:", this));
    xMaxSpin = new QDoubleSpinBox(this);
    xMaxSpin->setRange(-1000.0, 1000.0);
    xMaxSpin->setValue(5.0);
    xMaxSpin->setDecimals(2);
    xMaxSpin->setMinimumWidth(80);
    ctrlLayout->addWidget(xMaxSpin);

    ctrlLayout->addWidget(new QLabel("Шаг:", this));
    stepSpin = new QDoubleSpinBox(this);
    stepSpin->setRange(0.001, 100.0);
    stepSpin->setValue(0.1);
    stepSpin->setDecimals(3);
    stepSpin->setMinimumWidth(70);
    ctrlLayout->addWidget(stepSpin);

    plotBtn = new QPushButton("Построить график", this);
    ctrlLayout->addWidget(plotBtn);

    taskBtn = new QPushButton("Задание", this);
    ctrlLayout->addWidget(taskBtn);

    logoutBtn = new QPushButton("Выход", this);
    ctrlLayout->addWidget(logoutBtn);

    mainLayout->addLayout(ctrlLayout);

    // Task label (shows task text or graph info)
    taskLabel = new QLabel(this);
    taskLabel->setWordWrap(true);
    taskLabel->setAlignment(Qt::AlignCenter);
    taskLabel->setStyleSheet("color: #333; font-style: italic;");
    mainLayout->addWidget(taskLabel);

    // The widget itself acts as the drawing canvas — stretch fills remaining space
    mainLayout->addStretch(1);

    // Connections
    connect(plotBtn,   &QPushButton::clicked, this, &GraphWidget::onPlotClicked);
    connect(taskBtn,   &QPushButton::clicked, this, &GraphWidget::onGetTaskClicked);
    connect(logoutBtn, &QPushButton::clicked, this, &GraphWidget::logout);
}

void GraphWidget::setUserLogin(const QString &login)
{
    userLabel->setText("Пользователь: " + login);
}

void GraphWidget::onPlotClicked()
{
    taskLabel->setText("Загрузка данных...");
    taskLabel->repaint();

    double xMin = xMinSpin->value();
    double xMax = xMaxSpin->value();
    double step = stepSpin->value();

    if (xMin >= xMax) {
        taskLabel->setText("Ошибка: X мин должен быть меньше X макс.");
        return;
    }

    ClientSingleton &client = ClientSingleton::instance();
    if (!client.isConnected()) {
        taskLabel->setText("Ошибка: нет подключения к серверу.");
        return;
    }

    QString request = QString("get_graph||%1||%2||%3")
                        .arg(xMin, 0, 'f', 6)
                        .arg(xMax, 0, 'f', 6)
                        .arg(step, 0, 'f', 6);

    QString response = client.sendRequest(request);

    if (response.isEmpty()) {
        taskLabel->setText("Ошибка: нет ответа от сервера.");
        return;
    }

    // Protocol: "graph||x1;y1||x2;y2||..."
    QStringList parts = response.trimmed().split("||");
    if (parts.isEmpty() || parts[0] != "graph") {
        taskLabel->setText("Ошибка: неверный формат ответа сервера.");
        return;
    }

    points.clear();
    for (int i = 1; i < parts.size(); ++i) {
        QStringList xy = parts[i].split(";");
        if (xy.size() == 2) {
            bool okX = false, okY = false;
            double x = xy[0].toDouble(&okX);
            double y = xy[1].toDouble(&okY);
            if (okX && okY)
                points.append(QPointF(x, y));
        }
    }

    taskLabel->setText(QString("Построено %1 точек.").arg(points.size()));
    update(); // trigger repaint
}

void GraphWidget::onGetTaskClicked()
{
    ClientSingleton &client = ClientSingleton::instance();
    if (!client.isConnected()) {
        taskLabel->setText("Ошибка: нет подключения к серверу.");
        return;
    }

    QString response = client.sendRequest("get_task");
    taskLabel->setText(response.trimmed());
}

// -----------------------------------------------------------------------
// paintEvent — draws coordinate system and graph
// -----------------------------------------------------------------------
void GraphWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Drawing rectangle (leave margin for labels)
    QRect widgetRect = rect();

    // Reserve space for top controls (~80px) and task label (~30px)
    int topOffset = 90;
    QRect drawRect(
        MARGIN,
        topOffset + MARGIN / 2,
        widgetRect.width() - 2 * MARGIN,
        widgetRect.height() - topOffset - MARGIN
    );

    if (drawRect.width() < 10 || drawRect.height() < 10)
        return;

    // Background
    painter.fillRect(drawRect, QColor(250, 250, 255));

    // --- Determine data range ---
    double xMin = -5.0, xMax = 5.0, yMin = -3.0, yMax = 10.0;

    if (!points.isEmpty()) {
        xMin = points[0].x(); xMax = xMin;
        yMin = points[0].y(); yMax = yMin;
        for (const QPointF &p : points) {
            xMin = std::min(xMin, p.x());
            xMax = std::max(xMax, p.x());
            yMin = std::min(yMin, p.y());
            yMax = std::max(yMax, p.y());
        }
        // Add some padding to the data range
        double xPad = (xMax - xMin) * 0.05;
        double yPad = (yMax - yMin) * 0.05;
        if (xPad == 0) xPad = 0.5;
        if (yPad == 0) yPad = 0.5;
        xMin -= xPad; xMax += xPad;
        yMin -= yPad; yMax += yPad;
    }

    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    if (xRange == 0) xRange = 1;
    if (yRange == 0) yRange = 1;

    // Helpers: convert data coords -> widget coords
    auto toWidgetX = [&](double x) -> double {
        return drawRect.left() + (x - xMin) / xRange * drawRect.width();
    };
    auto toWidgetY = [&](double y) -> double {
        return drawRect.bottom() - (y - yMin) / yRange * drawRect.height();
    };

    // --- Grid lines ---
    painter.setPen(QPen(QColor(210, 210, 210), 1, Qt::DashLine));

    // Determine nice grid step
    auto niceStep = [](double range) -> double {
        double raw = range / 8.0;
        double mag = std::pow(10.0, std::floor(std::log10(raw)));
        double norm = raw / mag;
        double step;
        if      (norm < 1.5) step = 1;
        else if (norm < 3.5) step = 2;
        else if (norm < 7.5) step = 5;
        else                 step = 10;
        return step * mag;
    };

    double xStep = niceStep(xRange);
    double yStep = niceStep(yRange);

    double xStart = std::ceil(xMin / xStep) * xStep;
    double yStart = std::ceil(yMin / yStep) * yStep;

    for (double x = xStart; x <= xMax + xStep * 0.01; x += xStep) {
        int wx = static_cast<int>(toWidgetX(x));
        painter.drawLine(wx, drawRect.top(), wx, drawRect.bottom());
    }
    for (double y = yStart; y <= yMax + yStep * 0.01; y += yStep) {
        int wy = static_cast<int>(toWidgetY(y));
        painter.drawLine(drawRect.left(), wy, drawRect.right(), wy);
    }

    // --- Border ---
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(drawRect);

    // --- Axis labels ---
    QFont smallFont = painter.font();
    smallFont.setPointSize(8);
    painter.setFont(smallFont);
    painter.setPen(Qt::black);

    for (double x = xStart; x <= xMax + xStep * 0.01; x += xStep) {
        int wx = static_cast<int>(toWidgetX(x));
        // Tick
        painter.drawLine(wx, drawRect.bottom(), wx, drawRect.bottom() + 4);
        QString label = QString::number(x, 'g', 4);
        QRect textRect(wx - 25, drawRect.bottom() + 5, 50, 18);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }
    for (double y = yStart; y <= yMax + yStep * 0.01; y += yStep) {
        int wy = static_cast<int>(toWidgetY(y));
        painter.drawLine(drawRect.left() - 4, wy, drawRect.left(), wy);
        QString label = QString::number(y, 'g', 4);
        QRect textRect(drawRect.left() - MARGIN + 2, wy - 9, MARGIN - 8, 18);
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, label);
    }

    // --- Coordinate axes (if visible in range) ---
    painter.setPen(QPen(Qt::black, 2));

    // X axis
    if (yMin <= 0 && 0 <= yMax) {
        int wy0 = static_cast<int>(toWidgetY(0));
        painter.drawLine(drawRect.left(), wy0, drawRect.right(), wy0);
        // Arrow
        painter.drawLine(drawRect.right(), wy0,
                         drawRect.right() - 8, wy0 - 4);
        painter.drawLine(drawRect.right(), wy0,
                         drawRect.right() - 8, wy0 + 4);
        // Label
        painter.drawText(drawRect.right() + 4, wy0 + 5, "X");
    }
    // Y axis
    if (xMin <= 0 && 0 <= xMax) {
        int wx0 = static_cast<int>(toWidgetX(0));
        painter.drawLine(wx0, drawRect.bottom(), wx0, drawRect.top());
        // Arrow
        painter.drawLine(wx0, drawRect.top(),
                         wx0 - 4, drawRect.top() + 8);
        painter.drawLine(wx0, drawRect.top(),
                         wx0 + 4, drawRect.top() + 8);
        // Label
        painter.drawText(wx0 + 4, drawRect.top() - 4, "Y");
    }

    // --- Function title ---
    QFont titleFont = painter.font();
    titleFont.setBold(true);
    titleFont.setPointSize(9);
    painter.setFont(titleFont);
    painter.setPen(QColor(60, 60, 60));
    painter.drawText(drawRect.left(), drawRect.top() - 6,
                     "f(x) = { |x|−2, x<−2 } { x²+x+1, −2≤x<2 } { |x−2|+1, x≥2 }");

    // --- Graph line ---
    if (points.size() >= 2) {
        painter.setPen(QPen(QColor(0, 80, 220), 2));

        for (int i = 1; i < points.size(); ++i) {
            double x1 = points[i - 1].x(), y1 = points[i - 1].y();
            double x2 = points[i].x(),     y2 = points[i].y();
            int wx1 = static_cast<int>(toWidgetX(x1));
            int wy1 = static_cast<int>(toWidgetY(y1));
            int wx2 = static_cast<int>(toWidgetX(x2));
            int wy2 = static_cast<int>(toWidgetY(y2));
            painter.drawLine(wx1, wy1, wx2, wy2);
        }
    } else if (points.isEmpty()) {
        // Hint
        QFont hintFont = painter.font();
        hintFont.setItalic(true);
        hintFont.setPointSize(11);
        painter.setFont(hintFont);
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(drawRect, Qt::AlignCenter,
                         "Нажмите «Построить график» для отображения функции");
    }
}
