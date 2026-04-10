#include "schemadialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QPen>
#include <QFont>
#include <QPolygon>

// ══════════════════════════════════════════════════════════════════════
// FlowchartWidget
// ══════════════════════════════════════════════════════════════════════

FlowchartWidget::FlowchartWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(700, 1000);
    setStyleSheet("background: white;");
}

// ── Drawing helpers ─────────────────────────────────────────────────

void FlowchartWidget::drawRoundedBlock(QPainter &p, int cx, int cy, int w, int h,
                                        const QString &text, const QColor &fill)
{
    QRect rect(cx - w/2, cy - h/2, w, h);
    QPainterPath path;
    path.addRoundedRect(rect, 10, 10);
    p.fillPath(path, fill);
    p.setPen(QPen(Qt::black, 2));
    p.drawPath(path);
    p.setPen(Qt::black);
    QFont f = p.font();
    f.setPointSize(10);
    p.setFont(f);
    p.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, text);
}

void FlowchartWidget::drawDiamond(QPainter &p, int cx, int cy, int w, int h,
                                   const QString &text, const QColor &fill)
{
    QPolygon diamond;
    diamond << QPoint(cx, cy - h/2)
            << QPoint(cx + w/2, cy)
            << QPoint(cx, cy + h/2)
            << QPoint(cx - w/2, cy);

    QPainterPath path;
    path.addPolygon(diamond);
    path.closeSubpath();
    p.fillPath(path, fill);
    p.setPen(QPen(Qt::black, 2));
    p.drawPath(path);
    p.setPen(Qt::black);
    QFont f = p.font();
    f.setPointSize(9);
    p.setFont(f);
    QRect textRect(cx - w/2 + 15, cy - h/2 + 10, w - 30, h - 20);
    p.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
}

void FlowchartWidget::drawArrowDown(QPainter &p, int cx, int y1, int y2)
{
    p.setPen(QPen(Qt::black, 2));
    p.drawLine(cx, y1, cx, y2);
    QPolygon arrow;
    arrow << QPoint(cx, y2) << QPoint(cx - 6, y2 - 10) << QPoint(cx + 6, y2 - 10);
    p.setBrush(Qt::black);
    p.drawPolygon(arrow);
    p.setBrush(Qt::NoBrush);
}

void FlowchartWidget::drawArrowRight(QPainter &p, int x1, int x2, int y)
{
    p.setPen(QPen(Qt::black, 2));
    p.drawLine(x1, y, x2, y);
    QPolygon arrow;
    arrow << QPoint(x2, y) << QPoint(x2 - 10, y - 6) << QPoint(x2 - 10, y + 6);
    p.setBrush(Qt::black);
    p.drawPolygon(arrow);
    p.setBrush(Qt::NoBrush);
}

void FlowchartWidget::drawArrowLine(QPainter &p, int x1, int y1, int x2, int y2)
{
    p.setPen(QPen(Qt::black, 2));
    p.drawLine(x1, y1, x2, y2);
}

void FlowchartWidget::drawText(QPainter &p, int cx, int cy, int w, int h, const QString &text)
{
    QRect rect(cx - w/2, cy - h/2, w, h);
    QFont f = p.font();
    f.setPointSize(9);
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::black);
    p.drawText(rect, Qt::AlignCenter, text);
    f.setBold(false);
    p.setFont(f);
}

// ── Paint the flowchart ──────────────────────────────────────────────

void FlowchartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor greenFill(200, 240, 200);
    QColor blueFill(200, 220, 255);
    QColor yellowFill(255, 255, 200);
    QColor outputFill(200, 240, 230);

    // Layout constants
    int centerX = 220;   // main vertical flow
    int rightX  = 520;   // right blocks ("Da" branches)
    int bw = 200;        // block width
    int bh = 45;         // block height
    int dw = 240;        // diamond width
    int dh = 80;         // diamond height
    int gap = 30;        // vertical gap

    // Right-side routing: lines go to the right of rightX to avoid overlapping blocks
    int routeX = rightX + bw/2 + 25;  // vertical routing rail, right of all right blocks

    int y = 40;

    // ── 1. Начало ─────────────────────────────────────────
    drawRoundedBlock(p, centerX, y, 140, 40, "Начало", greenFill);
    int y1 = y + 20;
    y += 40 + gap;
    drawArrowDown(p, centerX, y1, y - bh/2);

    // ── 2. Ввод x, a, b, c ────────────────────────────────────
    drawRoundedBlock(p, centerX, y, bw, bh, "Ввод x, a, b, c", blueFill);
    y1 = y + bh/2;
    y += bh + gap;
    drawArrowDown(p, centerX, y1, y - dh/2);

    // ── 3. x < -2 ? ──────────────────────────────────────────────────
    int diamondY1 = y;   // center Y of first diamond
    drawDiamond(p, centerX, y, dw, dh, "x < -2 ?", yellowFill);

    // "Da" label + arrow right to branch block
    drawArrowRight(p, centerX + dw/2, rightX - bw/2, y);
    drawText(p, centerX + dw/2 + 25, y - 14, 40, 20, "Да");

    // Branch 1 block
    int rb1CenterY = diamondY1;  // same row as diamond
    drawRoundedBlock(p, rightX, rb1CenterY, bw, bh, "f = |x\u00B7a| \u2212 2", QColor(255, 200, 200));
    int rb1Bottom = rb1CenterY + bh/2;

    // "Net" arrow down from diamond 1
    y1 = y + dh/2;
    y += dh + gap;
    drawArrowDown(p, centerX, y1, y - dh/2);
    drawText(p, centerX + 16, y1 + 10, 40, 20, "Нет");

    // ── 4. -2 ≤ x < 2 ? ───────────────────────────────────────────────
    int diamondY2 = y;   // center Y of second diamond
    drawDiamond(p, centerX, y, dw, dh, "-2 \u2264 x < 2 ?", yellowFill);

    // "Da" label + arrow right to branch block
    drawArrowRight(p, centerX + dw/2, rightX - bw/2, y);
    drawText(p, centerX + dw/2 + 25, y - 14, 40, 20, "Да");

    // Branch 2 block
    int rb2CenterY = diamondY2;
    drawRoundedBlock(p, rightX, rb2CenterY, bw, bh, "f = b\u00B7(x\u00B2) + x + 1", QColor(200, 255, 200));
    int rb2Bottom = rb2CenterY + bh/2;

    // "Net" arrow down from diamond 2
    y1 = y + dh/2;
    y += dh + gap;
    drawArrowDown(p, centerX, y1, y - bh/2);
    drawText(p, centerX + 16, y1 + 10, 40, 20, "Нет");

    // ── 5. f = |x-2| + 1·c (center, "Net" from diamond 2) ──────────
    int block3CenterY = y;
    drawRoundedBlock(p, centerX, block3CenterY, bw, bh, "f = |x \u2212 2| + 1\u00B7c", QColor(200, 200, 255));
    int block3Bottom = block3CenterY + bh/2;

    // ── Merge point: all three branches join here ─────────────────────
    // Place merge well below block 3
    int mergeY = block3Bottom + gap + 20;

    // Center path: straight down from block 3 to merge
    drawArrowLine(p, centerX, block3Bottom, centerX, mergeY);

    // Right routing rail: go from rb1Bottom down the right side to mergeY,
    // then left across to centerX. This path stays OUTSIDE the diagram area.
    //   rb1Bottom -> routeX (horizontal right)
    //   routeX, rb1Bottom -> routeX, mergeY  (vertical down)
    //   routeX, mergeY -> centerX, mergeY    (horizontal left)
    // Branch 1 right side
    p.setPen(QPen(Qt::black, 2));
    p.drawLine(rightX + bw/2, rb1Bottom, routeX, rb1Bottom);    // right from block1
    p.drawLine(routeX, rb1Bottom, routeX, mergeY);               // down along rail

    // Branch 2 right side
    p.drawLine(rightX + bw/2, rb2Bottom, routeX, rb2Bottom);    // right from block2
    // (shares the same rail routeX, no need to redraw vertical — but we draw from rb2Bottom)
    // The vertical rail from routeX,rb1Bottom already goes to mergeY,
    // but rb2Bottom is below rb1Bottom, so draw from rb2Bottom down to mergeY:
    p.drawLine(routeX, rb2Bottom, routeX, mergeY);               // down to merge

    // Horizontal from routeX to centerX at mergeY, with arrowhead pointing left
    p.drawLine(routeX, mergeY, centerX, mergeY);
    QPolygon arrowLeft;
    arrowLeft << QPoint(centerX, mergeY)
              << QPoint(centerX + 10, mergeY - 6)
              << QPoint(centerX + 10, mergeY + 6);
    p.setBrush(Qt::black);
    p.drawPolygon(arrowLeft);
    p.setBrush(Qt::NoBrush);

    y = mergeY + 10;

    // ── 6. Вывод f(x) ───────────────────────────────────────────────
    drawArrowDown(p, centerX, mergeY, y + bh/2 - 5);
    y += bh/2;
    drawRoundedBlock(p, centerX, y, bw, bh, "Вывод f(x)", outputFill);
    y1 = y + bh/2;
    y += bh + gap;
    drawArrowDown(p, centerX, y1, y - 20);

    // ── 7. Конец ──────────────────────────────────────────────────────
    drawRoundedBlock(p, centerX, y, 140, 40, "Конец", greenFill);
}

// ══════════════════════════════════════════════════════════════════════
// SchemaDialog
// ══════════════════════════════════════════════════════════════════════

SchemaDialog::SchemaDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

SchemaDialog::~SchemaDialog() {}

void SchemaDialog::setupUI()
{
    setWindowTitle("Блок-схема вычислительного процесса");
    resize(750, 750);
    setMinimumSize(500, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    canvas = new FlowchartWidget();

    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(canvas);
    scrollArea->setWidgetResizable(false);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setStyleSheet("QScrollArea { border: 1px solid #ccc; }");
    mainLayout->addWidget(scrollArea, 1);

    closeBtn = new QPushButton("Закрыть", this);
    closeBtn->setMinimumHeight(36);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; "
        "border-radius: 5px; font-size: 12pt; padding: 4px 16px; }"
        "QPushButton:hover { background-color: #2980b9; }"
    );
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    btnRow->addWidget(closeBtn);
    btnRow->addStretch(1);
    mainLayout->addLayout(btnRow);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
