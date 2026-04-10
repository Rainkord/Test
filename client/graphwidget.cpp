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

// ── GitHub Dark palette ──────────────────────────────────────────────────
#define GH_BG          "#0d1117"
#define GH_PANEL       "#161b22"
#define GH_BORDER      "#30363d"
#define GH_TEXT        "#e6edf3"
#define GH_MUTED       "#8b949e"
#define GH_GREEN       "#238636"
#define GH_GREEN_H     "#2ea043"
#define GH_RED_BTN     "#b91c1c"
#define GH_RED_BTN_H   "#991b1b"
#define GH_INPUT_BG    "#0d1117"
#define GH_BTN_GHOST   "#21262d"
#define GH_BTN_GHOST_H "#30363d"

// Canvas colours
#define CANVAS_BG      QColor(0x0d, 0x11, 0x17)       // #0d1117
#define GRID_COLOR     QColor(0x21, 0x26, 0x2d)       // #21262d
#define AXIS_COLOR     QColor(0xe6, 0xed, 0xf3)       // #e6edf3
#define LABEL_COLOR    QColor(0x8b, 0x94, 0x9e)       // #8b949e
#define LEGEND_BG      QColor(0x16, 0x1b, 0x22, 210)  // #161b22 semi
#define LEGEND_BORDER  QColor(0x30, 0x36, 0x3d)       // #30363d

#define FONT_FAMILY    "Segoe UI"
#define FONT_SIZE_BTN  11
#define FONT_SIZE_UI   10

// ── Constructor ──────────────────────────────────────────────────────────
GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
    , currentA(1.0), currentB(1.0), currentC(1.0)
    , blockSliderA(false), blockSliderB(false), blockSliderC(false)
    , blockSpinA(false),   blockSpinB(false),   blockSpinC(false)
{
    // Глобальный тёмный фон
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_UI));
    setupUI();
    updateGraph();
}

GraphWidget::~GraphWidget() {}

void GraphWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(LEFT_PANEL_WIDTH);
    leftPanel->setObjectName("leftPanel");
    leftPanel->setStyleSheet(QString(
        "QWidget#leftPanel {"
        "  background-color: %1;"
        "  border-right: 1px solid %2;"
        "}"
    ).arg(GH_PANEL).arg(GH_BORDER));
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

    // ── Formula label ──────────────────────────────────────────────────
    formulaLabel = new QLabel(leftPanel);
    formulaLabel->setWordWrap(true);
    formulaLabel->setTextFormat(Qt::RichText);
    formulaLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    formulaLabel->setStyleSheet(QString(
        "QLabel {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "  padding: 6px;"
        "  color: %3;"
        "}"
    ).arg(GH_INPUT_BG).arg(GH_BORDER).arg(GH_TEXT));
    updateFormulaLabel();
    vbox->addWidget(formulaLabel);

    // ── Separator ──────────────────────────────────────────────────────
    auto makeSep = [&]() {
        QFrame *sep = new QFrame(leftPanel);
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
        return sep;
    };
    vbox->addWidget(makeSep());

    // ── Slider rows: a, b, c ───────────────────────────────────────────
    QString sliderStyle = QString(
        "QSlider::groove:horizontal {"
        "  height: 4px; background: %1; border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: %2; border: 2px solid %3;"
        "  width: 14px; height: 14px; margin: -5px 0; border-radius: 7px;"
        "}"
        "QSlider::sub-page:horizontal { background: %4; border-radius: 2px; }"
    ).arg(GH_BORDER).arg(GH_PANEL).arg("#388bfd").arg("#388bfd");

    QString spinStyle = QString(
        "QDoubleSpinBox {"
        "  background: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 4px; padding: 2px 4px; font-size: %4pt;"
        "}"
        "QDoubleSpinBox:focus { border-color: #388bfd; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_SIZE_UI);

    QString lblStyle = QString("QLabel { color: %1; font-size: %2pt; border: none; }")
                       .arg(GH_TEXT).arg(FONT_SIZE_UI);

    auto makeSliderRow = [&](const QString &name, QLabel *&lbl,
                              QSlider *&slider, QDoubleSpinBox *&spin) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(4);

        lbl = new QLabel(name + " =", leftPanel);
        lbl->setFixedWidth(28);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lbl->setStyleSheet(lblStyle);

        slider = new QSlider(Qt::Horizontal, leftPanel);
        slider->setRange(-50, 50);
        slider->setValue(10);
        slider->setTickInterval(10);
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setStyleSheet(sliderStyle);

        spin = new QDoubleSpinBox(leftPanel);
        spin->setRange(-5.0, 5.0);
        spin->setSingleStep(0.1);
        spin->setDecimals(1);
        spin->setValue(1.0);
        spin->setFixedWidth(60);
        spin->setStyleSheet(spinStyle);

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
    connect(spinA, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GraphWidget::onSpinAChanged);
    connect(spinB, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GraphWidget::onSpinBChanged);
    connect(spinC, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GraphWidget::onSpinCChanged);

    vbox->addWidget(makeSep());

    // ── Table ──────────────────────────────────────────────────────────────────
    table = new QTableWidget(21, 2, leftPanel);
    table->setHorizontalHeaderLabels({"x", "F(x)"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setStyleSheet(QString(
        "QTableWidget {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: 4px;"
        "  gridline-color: %3; font-size: %4pt;"
        "}"
        "QHeaderView::section {"
        "  background-color: %5; color: %2;"
        "  border: none; border-bottom: 1px solid %3; padding: 4px;"
        "}"
        "QTableWidget::item { padding: 2px; }"
        "QTableWidget::item:selected { background: #1f3a5f; }"
        "QScrollBar:vertical { background: %1; width: 8px; border: none; }"
        "QScrollBar::handle:vertical { background: %3; border-radius: 4px; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_SIZE_UI).arg(GH_PANEL));
    vbox->addWidget(table, 1);

    // ── User label ─────────────────────────────────────────────────────────
    userLabel = new QLabel("", leftPanel);
    userLabel->setStyleSheet(QString("QLabel { color: %1; font-size: %2pt; border: none; }")
                             .arg(GH_MUTED).arg(FONT_SIZE_UI - 1));
    userLabel->setAlignment(Qt::AlignCenter);
    vbox->addWidget(userLabel);

    // ── Logout button ─────────────────────────────────────────────────────
    logoutBtn = new QPushButton("Выйти из аккаунта", leftPanel);
    logoutBtn->setMinimumHeight(34);
    logoutBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: %1; color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
        "  padding: 6px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_RED_BTN).arg(GH_RED_BTN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN));
    connect(logoutBtn, &QPushButton::clicked, this, &GraphWidget::onLogoutClicked);
    vbox->addWidget(logoutBtn);
}

void GraphWidget::updateFormulaLabel()
{
    QString html = QString(
        "<p style='font-size:9pt; margin:0; line-height:1.6; color:#e6edf3;'>"
        "<b>f(x) =</b><br>"
        "<span style='color:#f85149;'>|x&middot;a| &minus; 2, x &lt; &minus;2</span><br>"
        "<span style='color:#3fb950;'>b&middot;(x&sup2;) + x + 1, &minus;2 &le; x &lt; 2</span><br>"
        "<span style='color:#58a6ff;'>|x &minus; 2| + 1&middot;c, x &ge; 2</span>"
        "</p>"
    );
    formulaLabel->setText(html);
}

// ── Slider / Spin handlers ───────────────────────────────────────────────
void GraphWidget::onSliderAChanged(int v) { if(blockSliderA) return; blockSpinA=true; spinA->setValue(v/10.0); blockSpinA=false; updateGraph(); }
void GraphWidget::onSliderBChanged(int v) { if(blockSliderB) return; blockSpinB=true; spinB->setValue(v/10.0); blockSpinB=false; updateGraph(); }
void GraphWidget::onSliderCChanged(int v) { if(blockSliderC) return; blockSpinC=true; spinC->setValue(v/10.0); blockSpinC=false; updateGraph(); }
void GraphWidget::onSpinAChanged(double v) { if(blockSpinA) return; blockSliderA=true; sliderA->setValue((int)(v*10)); blockSliderA=false; updateGraph(); }
void GraphWidget::onSpinBChanged(double v) { if(blockSpinB) return; blockSliderB=true; sliderB->setValue((int)(v*10)); blockSliderB=false; updateGraph(); }
void GraphWidget::onSpinCChanged(double v) { if(blockSpinC) return; blockSliderC=true; sliderC->setValue((int)(v*10)); blockSliderC=false; updateGraph(); }

void GraphWidget::onLogoutClicked()
{
    ClientSingleton::instance().disconnectFromServer();
    emit logout();
}

// ── Calculation ─────────────────────────────────────────────────────────────
double GraphWidget::calculate(double x, double a, double b, double c) const
{
    if (x < -2.0)        return std::fabs(x * a) - 2.0;
    else if (x < 2.0)   return b * (x * x) + x + 1.0;
    else                 return std::fabs(x - 2.0) + 1.0 * c;
}

void GraphWidget::updateGraph()
{
    double a = spinA->value(), b = spinB->value(), c = spinC->value();
    currentA = a; currentB = b; currentC = c;
    pointsBranch1.clear(); pointsBranch2.clear(); pointsBranch3.clear();

    QString response = ClientSingleton::instance().sendRequest(
        QString("get_graph||-10||10||0.1||%1||%2||%3").arg(a,0,'f',2).arg(b,0,'f',2).arg(c,0,'f',2));

    bool parsedFromServer = false;
    if (!response.isEmpty()) {
        QStringList parts = response.split("||");
        if (parts.size() >= 2 && parts[0] == "graph") {
            for (int i = 1; i < parts.size(); ++i) {
                QStringList xy = parts[i].split(";");
                if (xy.size() == 2) {
                    bool okX = false, okY = false;
                    double px = xy[0].toDouble(&okX), py = xy[1].toDouble(&okY);
                    if (okX && okY) {
                        QPointF pt(px, py);
                        if (px < -2.0) pointsBranch1.append(pt);
                        else if (px < 2.0) pointsBranch2.append(pt);
                        else pointsBranch3.append(pt);
                        parsedFromServer = true;
                    }
                }
            }
        }
    }
    if (!parsedFromServer) {
        for (double x = -10.0; x <= 10.0 + 1e-9; x += 0.1) {
            QPointF pt(x, calculate(x, a, b, c));
            if (x < -2.0) pointsBranch1.append(pt);
            else if (x < 2.0) pointsBranch2.append(pt);
            else pointsBranch3.append(pt);
        }
    }
    fillTable(a, b, c);
    update();
}

void GraphWidget::fillTable(double a, double b, double c)
{
    table->setRowCount(21);
    for (int row = 0, xi = -10; xi <= 10; ++xi, ++row) {
        double y = calculate((double)xi, a, b, c);
        QTableWidgetItem *ix = new QTableWidgetItem(QString::number(xi));
        QTableWidgetItem *iy = new QTableWidgetItem(QString::number(y, 'f', 2));
        ix->setTextAlignment(Qt::AlignCenter);
        iy->setTextAlignment(Qt::AlignCenter);
        // Строки таблицы — тёмный цвет текста
        ix->setForeground(QColor(GH_TEXT));
        iy->setForeground(QColor(GH_TEXT));
        table->setItem(row, 0, ix);
        table->setItem(row, 1, iy);
    }
}

void GraphWidget::setUserLogin(const QString &login)
{
    userLogin = login;
    if (userLabel)
        userLabel->setText("Пользователь: " + login);
}

// ── paintEvent ────────────────────────────────────────────────────────────
void GraphWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int MARGIN     = 60;
    const int leftOffset = LEFT_PANEL_WIDTH;

    int drawX = leftOffset + MARGIN;
    int drawY = MARGIN;
    int drawW = width()  - leftOffset - MARGIN * 2;
    int drawH = height() - MARGIN * 2;
    if (drawW < 50 || drawH < 50) return;

    // Фон платности канваса
    painter.fillRect(leftOffset, 0, width() - leftOffset, height(), CANVAS_BG);
    painter.fillRect(drawX, drawY, drawW, drawH, CANVAS_BG);

    QVector<QPointF> allPts;
    allPts += pointsBranch1; allPts += pointsBranch2; allPts += pointsBranch3;
    if (allPts.isEmpty()) return;

    double xMin = allPts[0].x(), xMax = allPts[0].x();
    double yMin = allPts[0].y(), yMax = allPts[0].y();
    for (const QPointF &pt : allPts) {
        xMin = std::min(xMin, pt.x()); xMax = std::max(xMax, pt.x());
        yMin = std::min(yMin, pt.y()); yMax = std::max(yMax, pt.y());
    }
    double yPad = std::max((yMax - yMin) * 0.05, 0.5);
    yMin -= yPad; yMax += yPad;
    double rangeX = std::max(xMax - xMin, 1e-9);
    double rangeY = std::max(yMax - yMin, 1e-9);
    double scaleX = drawW / rangeX;
    double scaleY = drawH / rangeY;

    auto sX = [&](double x) { return drawX + (int)((x - xMin) * scaleX); };
    auto sY = [&](double y) { return drawY + drawH - (int)((y - yMin) * scaleY); };

    // Рамка
    painter.setPen(QPen(QColor(GH_BORDER), 1));
    painter.drawRect(drawX, drawY, drawW, drawH);

    // Сетка
    painter.setPen(QPen(GRID_COLOR, 1, Qt::SolidLine));
    painter.setFont(QFont(FONT_FAMILY, 8));
    double xStep = 2.0;
    for (double gx = std::ceil(xMin/xStep)*xStep; gx <= xMax+1e-9; gx += xStep) {
        int sx = sX(gx);
        if (sx >= drawX && sx <= drawX+drawW) painter.drawLine(sx, drawY, sx, drawY+drawH);
    }
    double yRange = yMax - yMin;
    double rawStep = yRange / 8.0;
    double mag = std::pow(10.0, std::floor(std::log10(rawStep)));
    double niceSteps[] = {1.0, 2.0, 5.0, 10.0};
    double yStep = mag;
    for (double ns : niceSteps) { if (rawStep <= ns * mag) { yStep = ns * mag; break; } }
    if (yStep < 0.01) yStep = 0.5;
    for (double gy = std::ceil(yMin/yStep)*yStep; gy <= yMax+1e-9; gy += yStep) {
        int sy = sY(gy);
        if (sy >= drawY && sy <= drawY+drawH) painter.drawLine(drawX, sy, drawX+drawW, sy);
    }

    // Оси
    painter.setPen(QPen(AXIS_COLOR, 2));
    if (yMin <= 0.0 && yMax >= 0.0) {
        int sy = sY(0.0);
        painter.drawLine(drawX, sy, drawX+drawW, sy);
        painter.drawLine(drawX+drawW-8, sy-4, drawX+drawW, sy);
        painter.drawLine(drawX+drawW-8, sy+4, drawX+drawW, sy);
    }
    if (xMin <= 0.0 && xMax >= 0.0) {
        int sx = sX(0.0);
        painter.drawLine(sx, drawY, sx, drawY+drawH);
        painter.drawLine(sx-4, drawY+8, sx, drawY);
        painter.drawLine(sx+4, drawY+8, sx, drawY);
    }

    // Метки и подписи осей
    painter.setFont(QFont(FONT_FAMILY, 8));
    painter.setPen(LABEL_COLOR);
    for (double gx = std::ceil(xMin/xStep)*xStep; gx <= xMax+1e-9; gx += xStep) {
        int sx = sX(gx);
        if (sx < drawX || sx > drawX+drawW) continue;
        int sy0 = (yMin <= 0.0 && yMax >= 0.0) ? sY(0.0) : drawY+drawH;
        painter.setPen(AXIS_COLOR);
        painter.drawLine(sx, sy0-3, sx, sy0+3);
        painter.setPen(LABEL_COLOR);
        painter.drawText(QRect(sx-20, sy0+5, 40, 14), Qt::AlignCenter, QString::number(gx,'f',0));
    }
    for (double gy = std::ceil(yMin/yStep)*yStep; gy <= yMax+1e-9; gy += yStep) {
        if (std::fabs(gy) < 1e-9) continue;
        int sy = sY(gy);
        if (sy < drawY || sy > drawY+drawH) continue;
        int sx0 = (xMin <= 0.0 && xMax >= 0.0) ? sX(0.0) : drawX;
        painter.setPen(AXIS_COLOR);
        painter.drawLine(sx0-3, sy, sx0+3, sy);
        painter.setPen(LABEL_COLOR);
        painter.drawText(QRect(sx0-45, sy-7, 40, 14), Qt::AlignRight|Qt::AlignVCenter, QString::number(gy,'f',1));
    }
    painter.setFont(QFont(FONT_FAMILY, 10, QFont::Bold));
    painter.setPen(AXIS_COLOR);
    painter.drawText(drawX+drawW-12, drawY+drawH/2+12, "x");
    painter.drawText(drawX+drawW/2-6, drawY+14, "y");

    // Кривые
    auto drawBranch = [&](const QVector<QPointF> &pts, const QColor &color) {
        if (pts.size() < 2) return;
        painter.setPen(QPen(color, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        for (int i = 1; i < pts.size(); ++i) {
            int x1=sX(pts[i-1].x()), y1=sY(pts[i-1].y());
            int x2=sX(pts[i].x()),   y2=sY(pts[i].y());
            if ((x1<drawX&&x2<drawX)||(x1>drawX+drawW&&x2>drawX+drawW)) continue;
            if ((y1<drawY&&y2<drawY)||(y1>drawY+drawH&&y2>drawY+drawH)) continue;
            painter.drawLine(x1,y1,x2,y2);
        }
    };
    drawBranch(pointsBranch1, QColor("#f85149"));  // GitHub red
    drawBranch(pointsBranch2, QColor("#3fb950"));  // GitHub green
    drawBranch(pointsBranch3, QColor("#58a6ff"));  // GitHub blue

    // Легенда
    {
        int lx = drawX + 10, ly = drawY + 10;
        int lw = 250, lh = 64;
        painter.fillRect(lx-4, ly-4, lw+8, lh+8, LEGEND_BG);
        painter.setPen(QPen(LEGEND_BORDER, 1));
        painter.drawRect(lx-4, ly-4, lw+8, lh+8);
        painter.setFont(QFont(FONT_FAMILY, 9));
        struct { QColor color; QString text; } legs[] = {
            { QColor("#f85149"), "|x\u00B7a| \u2212 2,  x < \u22122" },
            { QColor("#3fb950"), "b\u00B7(x\u00B2) + x + 1,  \u22122 \u2264 x < 2" },
            { QColor("#58a6ff"), "|x \u2212 2| + 1\u00B7c,  x \u2265 2" }
        };
        for (int i = 0; i < 3; ++i) {
            int lineY = ly + i*20 + 10;
            painter.setPen(QPen(legs[i].color, 2.5));
            painter.drawLine(lx, lineY, lx+25, lineY);
            painter.setPen(legs[i].color);
            painter.drawText(lx+30, lineY+4, legs[i].text);
        }
    }
}
