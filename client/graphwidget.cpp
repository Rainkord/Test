#include "graphwidget.h"
#include "clientsingleton.h"
#include "taskdialog.h"
#include "schemadialog.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QFrame>
#include <QStyle>
#include <QApplication>
#include <cmath>
#include <algorithm>
#include <limits>

// ─────────────────────────────────────────────
GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
    , currentA(1.0)
    , currentB(1.0)
    , currentC(1.0)
    , blockSliderA(false)
    , blockSliderB(false)
    , blockSliderC(false)
    , blockSpinA(false)
    , blockSpinB(false)
    , blockSpinC(false)
{
    setupUI();
    updateGraph();
}

GraphWidget::~GraphWidget() {}

// ─────────────────────────────────────────────
void GraphWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(LEFT_PANEL_WIDTH);
    leftPanel->setObjectName("leftPanel");
    leftPanel->setStyleSheet(
        "QWidget#leftPanel { background-color: #f5f5f5; border-right: 1px solid #cccccc; }"
    );
    setupLeftPanel();

    mainLayout->addWidget(leftPanel);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
    setMinimumSize(1000, 700);
}

void GraphWidget::setupLeftPanel()
{
    QVBoxLayout *vbox = new QVBoxLayout(leftPanel);
    vbox->setContentsMargins(8, 8, 8, 8);
    vbox->setSpacing(6);

    // ── Formula label ────────────────────────────────────────────
    formulaLabel = new QLabel(leftPanel);
    formulaLabel->setWordWrap(true);
    formulaLabel->setTextFormat(Qt::RichText);
    formulaLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    formulaLabel->setStyleSheet("QLabel { background: white; border: 1px solid #ccc; padding: 4px; }");
    updateFormulaLabel();
    vbox->addWidget(formulaLabel);

    // ── Separator ────────────────────────────────────────────
    QFrame *sep1 = new QFrame(leftPanel);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    vbox->addWidget(sep1);

    // ── Slider rows: a, b, c ─────────────────────────────────────
    auto makeSliderRow = [&](const QString &name, QLabel *&lbl,
                              QSlider *&slider, QDoubleSpinBox *&spin) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(4);

        lbl = new QLabel(name + " =", leftPanel);
        lbl->setFixedWidth(28);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        slider = new QSlider(Qt::Horizontal, leftPanel);
        slider->setRange(-50, 50);
        slider->setValue(10);
        slider->setTickInterval(10);
        slider->setTickPosition(QSlider::TicksBelow);

        spin = new QDoubleSpinBox(leftPanel);
        spin->setRange(-5.0, 5.0);
        spin->setSingleStep(0.1);
        spin->setDecimals(1);
        spin->setValue(1.0);
        spin->setFixedWidth(60);

        row->addWidget(lbl);
        row->addWidget(slider, 1);
        row->addWidget(spin);
        vbox->addLayout(row);
    };

    makeSliderRow("a", labelA, sliderA, spinA);
    makeSliderRow("b", labelB, sliderB, spinB);
    makeSliderRow("c", labelC, sliderC, spinC);

    connect(sliderA, &QSlider::valueChanged, this, &GraphWidget::onSliderAChanged);
    connect(sliderB, &QSlider::valueChanged, this, &GraphWidget::onSliderBChanged);
    connect(sliderC, &QSlider::valueChanged, this, &GraphWidget::onSliderCChanged);
    connect(spinA, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GraphWidget::onSpinAChanged);
    connect(spinB, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GraphWidget::onSpinBChanged);
    connect(spinC, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GraphWidget::onSpinCChanged);

    // ── Separator ────────────────────────────────────────────
    QFrame *sep2 = new QFrame(leftPanel);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    vbox->addWidget(sep2);

    // ── Table (-10 to 19, step 1, 30 rows, no fixed height) ───────────
    table = new QTableWidget(30, 2, leftPanel);
    table->setHorizontalHeaderLabels({"x", "F(x)"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setStyleSheet("QTableWidget { font-size: 11px; }");
    // No setFixedHeight — table expands to fill available space
    vbox->addWidget(table, 1);

    // ── User label ───────────────────────────────────────────
    userLabel = new QLabel("", leftPanel);
    userLabel->setStyleSheet("QLabel { color: #666; font-size: 10px; }");
    userLabel->setAlignment(Qt::AlignCenter);
    vbox->addWidget(userLabel);

    // ── Logout button ─────────────────────────────────────────
    logoutBtn = new QPushButton("Выйти из аккаунта", leftPanel);
    logoutBtn->setStyleSheet(
        "QPushButton { background-color: #e74c3c; color: white; "
        "border-radius: 4px; padding: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #c0392b; }"
    );
    logoutBtn->setMinimumHeight(34);
    vbox->addWidget(logoutBtn);

    connect(logoutBtn, &QPushButton::clicked, this, &GraphWidget::onLogoutClicked);
}

void GraphWidget::updateFormulaLabel()
{
    QString html = QString(
        "<p style='font-size:10pt; margin:0; line-height:1.5;'>"
        "<b>f(x) =</b><br>"
        "<span style='color:red;'>|x&middot;a| &minus; 2, x &lt; &minus;2</span><br>"
        "<span style='color:green;'>b&middot;(x&sup2;) + x + 1, &minus;2 &le; x &lt; 2</span><br>"
        "<span style='color:blue;'>|x &minus; 2| + 1&middot;c, x &ge; 2</span>"
        "</p>"
    );
    formulaLabel->setText(html);
}

// ─────────────────────────────────────────────
void GraphWidget::onSliderAChanged(int value)
{
    if (blockSliderA) return;
    blockSpinA = true;
    spinA->setValue(value / 10.0);
    blockSpinA = false;
    updateGraph();
}

void GraphWidget::onSliderBChanged(int value)
{
    if (blockSliderB) return;
    blockSpinB = true;
    spinB->setValue(value / 10.0);
    blockSpinB = false;
    updateGraph();
}

void GraphWidget::onSliderCChanged(int value)
{
    if (blockSliderC) return;
    blockSpinC = true;
    spinC->setValue(value / 10.0);
    blockSpinC = false;
    updateGraph();
}

void GraphWidget::onSpinAChanged(double value)
{
    if (blockSpinA) return;
    blockSliderA = true;
    sliderA->setValue(static_cast<int>(value * 10.0));
    blockSliderA = false;
    updateGraph();
}

void GraphWidget::onSpinBChanged(double value)
{
    if (blockSpinB) return;
    blockSliderB = true;
    sliderB->setValue(static_cast<int>(value * 10.0));
    blockSliderB = false;
    updateGraph();
}

void GraphWidget::onSpinCChanged(double value)
{
    if (blockSpinC) return;
    blockSliderC = true;
    sliderC->setValue(static_cast<int>(value * 10.0));
    blockSliderC = false;
    updateGraph();
}

void GraphWidget::onLogoutClicked()
{
    ClientSingleton::instance().disconnectFromServer();
    emit logout();
}

// ─────────────────────────────────────────────
double GraphWidget::calculate(double x, double a, double b, double c) const
{
    if (x < -2.0)
        return std::fabs(x * a) - 2.0;
    else if (x >= -2.0 && x < 2.0)
        return b * (x * x) + x + 1.0;
    else
        return std::fabs(x - 2.0) + 1.0 * c;
}

void GraphWidget::updateGraph()
{
    double a = spinA->value();
    double b = spinB->value();
    double c = spinC->value();
    currentA = a;
    currentB = b;
    currentC = c;

    pointsBranch1.clear();
    pointsBranch2.clear();
    pointsBranch3.clear();

    QString request = QString("get_graph||-10||10||0.1||%1||%2||%3")
                          .arg(a, 0, 'f', 2)
                          .arg(b, 0, 'f', 2)
                          .arg(c, 0, 'f', 2);

    QString response = ClientSingleton::instance().sendRequest(request);

    bool parsedFromServer = false;
    if (!response.isEmpty()) {
        QStringList parts = response.split("||");
        if (parts.size() >= 2 && parts[0] == "graph") {
            for (int i = 1; i < parts.size(); ++i) {
                QStringList xy = parts[i].split(";");
                if (xy.size() == 2) {
                    bool okX = false, okY = false;
                    double px = xy[0].toDouble(&okX);
                    double py = xy[1].toDouble(&okY);
                    if (okX && okY) {
                        QPointF pt(px, py);
                        if (px < -2.0)
                            pointsBranch1.append(pt);
                        else if (px < 2.0)
                            pointsBranch2.append(pt);
                        else
                            pointsBranch3.append(pt);
                        parsedFromServer = true;
                    }
                }
            }
        }
    }

    if (!parsedFromServer) {
        const double xStart = -10.0;
        const double xEnd   =  10.0;
        const double step   =  0.1;
        for (double x = xStart; x <= xEnd + 1e-9; x += step) {
            double y = calculate(x, a, b, c);
            QPointF pt(x, y);
            if (x < -2.0)
                pointsBranch1.append(pt);
            else if (x < 2.0)
                pointsBranch2.append(pt);
            else
                pointsBranch3.append(pt);
        }
    }

    fillTable(a, b, c);
    update();
}

void GraphWidget::fillTable(double a, double b, double c)
{
    // x from -10 to 19 inclusive, step 1 => 30 rows
    table->setRowCount(30);
    int row = 0;
    for (int xi = -10; xi <= 19 && row < 30; ++xi, ++row) {
        double x = static_cast<double>(xi);
        double y = calculate(x, a, b, c);
        QTableWidgetItem *itemX = new QTableWidgetItem(QString::number(x, 'f', 0));
        QTableWidgetItem *itemY = new QTableWidgetItem(QString::number(y, 'f', 2));
        itemX->setTextAlignment(Qt::AlignCenter);
        itemY->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 0, itemX);
        table->setItem(row, 1, itemY);
    }
}

void GraphWidget::setUserLogin(const QString &login)
{
    userLogin = login;
    if (userLabel) {
        userLabel->setText("Пользователь: " + login);
    }
}

// ─────────────────────────────────────────────
void GraphWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int MARGIN = 60;
    const int leftOffset = LEFT_PANEL_WIDTH;

    int totalW = width();
    int totalH = height();

    int drawX = leftOffset + MARGIN;
    int drawY = MARGIN;
    int drawW = totalW - leftOffset - MARGIN * 2;
    int drawH = totalH - MARGIN * 2;

    if (drawW < 50 || drawH < 50) return;

    painter.fillRect(leftOffset, 0, totalW - leftOffset, totalH, QColor("#ffffff"));
    painter.fillRect(drawX, drawY, drawW, drawH, QColor("#ffffff"));

    QVector<QPointF> allPts;
    allPts.append(pointsBranch1);
    allPts.append(pointsBranch2);
    allPts.append(pointsBranch3);

    if (allPts.isEmpty()) return;

    double xMin = allPts[0].x(), xMax = allPts[0].x();
    double yMin = allPts[0].y(), yMax = allPts[0].y();
    for (const QPointF &pt : allPts) {
        if (pt.x() < xMin) xMin = pt.x();
        if (pt.x() > xMax) xMax = pt.x();
        if (pt.y() < yMin) yMin = pt.y();
        if (pt.y() > yMax) yMax = pt.y();
    }

    double yPad = (yMax - yMin) * 0.05;
    if (yPad < 0.5) yPad = 0.5;
    yMin -= yPad;
    yMax += yPad;

    double rangeX = xMax - xMin;
    double rangeY = yMax - yMin;
    if (rangeX < 1e-9) rangeX = 1.0;
    if (rangeY < 1e-9) rangeY = 1.0;

    double scaleX = static_cast<double>(drawW) / rangeX;
    double scaleY = static_cast<double>(drawH) / rangeY;

    auto toScreenX = [&](double x) -> int {
        return drawX + static_cast<int>((x - xMin) * scaleX);
    };
    auto toScreenY = [&](double y) -> int {
        return drawY + drawH - static_cast<int>((y - yMin) * scaleY);
    };

    painter.setPen(QPen(QColor("#aaaaaa"), 1));
    painter.drawRect(drawX, drawY, drawW, drawH);

    {
        QPen gridPen(QColor("#e0e0e0"), 1, Qt::DashLine);
        painter.setPen(gridPen);
        painter.setFont(QFont("Arial", 8));

        double xStep = 2.0;
        for (double gx = std::ceil(xMin / xStep) * xStep; gx <= xMax + 1e-9; gx += xStep) {
            int sx = toScreenX(gx);
            if (sx >= drawX && sx <= drawX + drawW)
                painter.drawLine(sx, drawY, sx, drawY + drawH);
        }
        double yRange = yMax - yMin;
        double rawStep = yRange / 8.0;
        double mag = std::pow(10.0, std::floor(std::log10(rawStep)));
        double niceSteps[] = {1.0, 2.0, 5.0, 10.0};
        double yStep = mag;
        for (double ns : niceSteps) {
            if (rawStep <= ns * mag) { yStep = ns * mag; break; }
        }
        if (yStep < 0.01) yStep = 0.5;
        for (double gy = std::ceil(yMin / yStep) * yStep; gy <= yMax + 1e-9; gy += yStep) {
            int sy = toScreenY(gy);
            if (sy >= drawY && sy <= drawY + drawH)
                painter.drawLine(drawX, sy, drawX + drawW, sy);
        }
    }

    {
        QPen axisPen(QColor("#000000"), 2);
        painter.setPen(axisPen);

        if (yMin <= 0.0 && yMax >= 0.0) {
            int sy = toScreenY(0.0);
            painter.drawLine(drawX, sy, drawX + drawW, sy);
            painter.drawLine(drawX + drawW - 8, sy - 4, drawX + drawW, sy);
            painter.drawLine(drawX + drawW - 8, sy + 4, drawX + drawW, sy);
        }
        if (xMin <= 0.0 && xMax >= 0.0) {
            int sx = toScreenX(0.0);
            painter.drawLine(sx, drawY, sx, drawY + drawH);
            painter.drawLine(sx - 4, drawY + 8, sx, drawY);
            painter.drawLine(sx + 4, drawY + 8, sx, drawY);
        }

        painter.setFont(QFont("Arial", 8));
        painter.setPen(QColor("#333333"));

        double xStep = 2.0;
        for (double gx = std::ceil(xMin / xStep) * xStep; gx <= xMax + 1e-9; gx += xStep) {
            int sx = toScreenX(gx);
            if (sx < drawX || sx > drawX + drawW) continue;
            int sy0 = (yMin <= 0.0 && yMax >= 0.0) ? toScreenY(0.0) : drawY + drawH;
            painter.drawLine(sx, sy0 - 3, sx, sy0 + 3);
            QString lbl = QString::number(gx, 'f', 0);
            painter.drawText(QRect(sx - 20, sy0 + 5, 40, 14), Qt::AlignCenter, lbl);
        }

        double yRange = yMax - yMin;
        double rawStep = yRange / 8.0;
        double mag = std::pow(10.0, std::floor(std::log10(rawStep)));
        double niceSteps[] = {1.0, 2.0, 5.0, 10.0};
        double yStep = mag;
        for (double ns : niceSteps) {
            if (rawStep <= ns * mag) { yStep = ns * mag; break; }
        }
        if (yStep < 0.01) yStep = 0.5;
        for (double gy = std::ceil(yMin / yStep) * yStep; gy <= yMax + 1e-9; gy += yStep) {
            if (std::fabs(gy) < 1e-9) continue;
            int sy = toScreenY(gy);
            if (sy < drawY || sy > drawY + drawH) continue;
            int sx0 = (xMin <= 0.0 && xMax >= 0.0) ? toScreenX(0.0) : drawX;
            painter.drawLine(sx0 - 3, sy, sx0 + 3, sy);
            QString lbl = QString::number(gy, 'f', 1);
            painter.drawText(QRect(sx0 - 45, sy - 7, 40, 14), Qt::AlignRight | Qt::AlignVCenter, lbl);
        }

        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.setPen(QColor("#000000"));
        painter.drawText(drawX + drawW - 12, drawY + drawH / 2 + 12, "x");
        painter.drawText(drawX + drawW / 2 - 6, drawY + 14, "y");
    }

    auto drawBranchLines = [&](const QVector<QPointF> &pts, const QColor &color) {
        if (pts.size() < 2) return;
        QPen pen(color, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        for (int i = 1; i < pts.size(); ++i) {
            int x1 = toScreenX(pts[i-1].x());
            int y1 = toScreenY(pts[i-1].y());
            int x2 = toScreenX(pts[i].x());
            int y2 = toScreenY(pts[i].y());
            if ((x1 < drawX && x2 < drawX) || (x1 > drawX+drawW && x2 > drawX+drawW)) continue;
            if ((y1 < drawY && y2 < drawY) || (y1 > drawY+drawH && y2 > drawY+drawH)) continue;
            painter.drawLine(x1, y1, x2, y2);
        }
    };

    drawBranchLines(pointsBranch1, QColor(Qt::red));
    drawBranchLines(pointsBranch2, QColor(Qt::darkGreen));
    drawBranchLines(pointsBranch3, QColor(Qt::blue));

    // ── Legend (all proper Unicode symbols) ───────────────────────
    {
        int lx = drawX + 10;
        int ly = drawY + 10;
        int lw = 230;
        int lh = 64;

        painter.fillRect(lx - 4, ly - 4, lw + 8, lh + 8, QColor(255, 255, 255, 200));
        painter.setPen(QColor("#aaaaaa"));
        painter.drawRect(lx - 4, ly - 4, lw + 8, lh + 8);

        painter.setFont(QFont("Arial", 9));

        // Use only proper Unicode — no Latin-1 byte escapes
        struct { QColor color; QString text; } legends[] = {
            { Qt::red,       "|x\u00B7a| \u2212 2,  x < \u22122" },
            { Qt::darkGreen, "b\u00B7(x\u00B2) + x + 1,  \u22122 \u2264 x < 2" },
            { Qt::blue,      "|x \u2212 2| + 1\u00B7c,  x \u2265 2" }
        };

        for (int i = 0; i < 3; ++i) {
            int lineY = ly + i * 20 + 10;
            painter.setPen(QPen(legends[i].color, 2.5));
            painter.drawLine(lx, lineY, lx + 25, lineY);
            painter.setPen(legends[i].color);
            painter.drawText(lx + 30, lineY + 4, legends[i].text);
        }
    }
}
