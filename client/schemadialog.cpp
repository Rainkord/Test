#include "schemadialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QFont>
#include <QPolygon>

#define FC_BG          QColor(0x0d, 0x11, 0x17)
#define FC_BORDER      QColor(0x30, 0x36, 0x3d)
#define FC_TEXT        QColor(0xe6, 0xed, 0xf3)
#define FC_ARROW       QColor(0x8b, 0x94, 0x9e)
#define FC_LABEL       QColor(0xd2, 0x99, 0x22)

#define FC_START_FILL  QColor(0x0f, 0x51, 0x32)
#define FC_INPUT_FILL  QColor(0x0d, 0x41, 0x9d)
#define FC_COND_FILL   QColor(0x5d, 0x43, 0x00)
#define FC_BR1_FILL    QColor(0x6e, 0x1a, 0x1a)
#define FC_BR2_FILL    QColor(0x0e, 0x4d, 0x55)
#define FC_BR3_FILL    QColor(0x3d, 0x1f, 0x6e)
#define FC_OUT_FILL    QColor(0x0d, 0x38, 0x75)

#define FC_START_BDR   QColor(0x23, 0x86, 0x36)
#define FC_INPUT_BDR   QColor(0x38, 0x8b, 0xfd)
#define FC_COND_BDR    QColor(0xd2, 0x99, 0x22)
#define FC_BR1_BDR     QColor(0xf8, 0x51, 0x49)
#define FC_BR2_BDR     QColor(0x39, 0xc5, 0xcf)
#define FC_BR3_BDR     QColor(0xa3, 0x71, 0xf7)
#define FC_OUT_BDR     QColor(0x58, 0xa6, 0xff)

#define FONT_FAMILY    "Segoe UI"

// Виртуальные размеры схемы (до масштабирования)
#define SCHEMA_W  700
#define SCHEMA_H  735
#define SCALE     (1.0 / 1.5)

// ──────────────────────────────────────────────────────────────────────────
FlowchartWidget::FlowchartWidget(QWidget *parent)
    : QWidget(parent)
{
    // Размер канваса = виртуальные размеры схемы (отображается в 1.5 раза меньше через scale)
    setFixedSize(SCHEMA_W, SCHEMA_H);
}

void FlowchartWidget::drawRoundedBlock(QPainter &p, int cx, int cy, int w, int h,
                                        const QString &text,
                                        const QColor &fill, const QColor &border)
{
    QRect rect(cx - w/2, cy - h/2, w, h);
    QPainterPath path;
    path.addRoundedRect(rect, 10, 10);
    p.fillPath(path, fill);
    p.setPen(QPen(border, 1.5));
    p.drawPath(path);
    p.setPen(FC_TEXT);
    QFont f(FONT_FAMILY, 10);
    p.setFont(f);
    p.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, text);
}

void FlowchartWidget::drawDiamond(QPainter &p, int cx, int cy, int w, int h,
                                   const QString &text,
                                   const QColor &fill, const QColor &border)
{
    QPolygon diamond;
    diamond << QPoint(cx,       cy - h/2)
            << QPoint(cx + w/2, cy)
            << QPoint(cx,       cy + h/2)
            << QPoint(cx - w/2, cy);
    QPainterPath path;
    path.addPolygon(diamond);
    path.closeSubpath();
    p.fillPath(path, fill);
    p.setPen(QPen(border, 1.5));
    p.drawPath(path);
    p.setPen(FC_TEXT);
    QFont f(FONT_FAMILY, 9);
    p.setFont(f);
    QRect textRect(cx - w/2 + 15, cy - h/2 + 10, w - 30, h - 20);
    p.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
}

void FlowchartWidget::drawArrowDown(QPainter &p, int cx, int y1, int y2)
{
    p.setPen(QPen(FC_ARROW, 2));
    p.drawLine(cx, y1, cx, y2);
    QPolygon arr;
    arr << QPoint(cx, y2) << QPoint(cx-6, y2-10) << QPoint(cx+6, y2-10);
    p.setBrush(FC_ARROW);
    p.drawPolygon(arr);
    p.setBrush(Qt::NoBrush);
}

void FlowchartWidget::drawArrowRight(QPainter &p, int x1, int x2, int y)
{
    p.setPen(QPen(FC_ARROW, 2));
    p.drawLine(x1, y, x2, y);
    QPolygon arr;
    arr << QPoint(x2, y) << QPoint(x2-10, y-6) << QPoint(x2-10, y+6);
    p.setBrush(FC_ARROW);
    p.drawPolygon(arr);
    p.setBrush(Qt::NoBrush);
}

void FlowchartWidget::drawArrowLine(QPainter &p, int x1, int y1, int x2, int y2)
{
    p.setPen(QPen(FC_ARROW, 2));
    p.drawLine(x1, y1, x2, y2);
}

void FlowchartWidget::drawText(QPainter &p, int cx, int cy, int w, int h, const QString &text)
{
    QRect rect(cx - w/2, cy - h/2, w, h);
    QFont f(FONT_FAMILY, 9, QFont::Bold);
    p.setFont(f);
    p.setPen(FC_LABEL);
    p.drawText(rect, Qt::AlignCenter, text);
}

void FlowchartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), FC_BG);

    // Схема рисуется в виртуальных координатах 700x735,
    // затем масштабируется до 1/1.5 ≈ 467x490
    // и центрируется через translate
    const double sc = SCALE;
    const double scaledW = SCHEMA_W * sc;  // ~467
    const double scaledH = SCHEMA_H * sc;  // ~490
    const double offX = (width()  - scaledW) / 2.0;
    const double offY = (height() - scaledH) / 2.0;

    p.save();
    p.translate(offX, offY);
    p.scale(sc, sc);

    // Все координаты ниже — в виртуальном пространстве 700x735
    int bw = 200, bh = 40;
    int dw = 240, dh = 70;
    int gap = 18;

    // центр основного потока: полуширина схемы влево = dw/2=120, вправо = 300+bw/2+25=425
    // centerX = 120 + (700-545)/2 = 120+77 = 197 ≈ 200
    int centerX = 200;
    int rightX  = centerX + 300;
    int routeX  = rightX + bw/2 + 25;

    // старт сверху: высота схемы ~683, (735-683)/2 = 26
    int y = 26;

    // 1. Начало
    drawRoundedBlock(p, centerX, y, 140, 36, "Начало", FC_START_FILL, FC_START_BDR);
    int y1 = y + 18; y += 36 + gap;
    drawArrowDown(p, centerX, y1, y - bh/2);

    // 2. Ввод
    drawRoundedBlock(p, centerX, y, bw, bh, "Ввод x, a, b, c", FC_INPUT_FILL, FC_INPUT_BDR);
    y1 = y + bh/2; y += bh + gap;
    drawArrowDown(p, centerX, y1, y - dh/2);

    // 3. x < -2 ?
    int diamondY1 = y;
    drawDiamond(p, centerX, y, dw, dh, "x < -2 ?", FC_COND_FILL, FC_COND_BDR);
    drawArrowRight(p, centerX + dw/2, rightX - bw/2, y);
    drawText(p, centerX + dw/2 + 25, y - 12, 40, 20, "Да");
    drawRoundedBlock(p, rightX, diamondY1, bw, bh, "f = |x·a| − 2", FC_BR1_FILL, FC_BR1_BDR);
    int rb1Bottom = diamondY1 + bh/2;

    y1 = y + dh/2; y += dh + gap;
    drawArrowDown(p, centerX, y1, y - dh/2);
    drawText(p, centerX + 16, y1 + 8, 40, 20, "Нет");

    // 4. -2 ≤ x < 2 ?
    int diamondY2 = y;
    drawDiamond(p, centerX, y, dw, dh, "-2 ≤ x < 2 ?", FC_COND_FILL, FC_COND_BDR);
    drawArrowRight(p, centerX + dw/2, rightX - bw/2, y);
    drawText(p, centerX + dw/2 + 25, y - 12, 40, 20, "Да");
    drawRoundedBlock(p, rightX, diamondY2, bw, bh, "f = b·(x²) + x + 1", FC_BR2_FILL, FC_BR2_BDR);
    int rb2Bottom = diamondY2 + bh/2;

    y1 = y + dh/2; y += dh + gap;
    drawArrowDown(p, centerX, y1, y - bh/2);
    drawText(p, centerX + 16, y1 + 8, 40, 20, "Нет");

    // 5. Ветвь 3
    int block3CenterY = y;
    drawRoundedBlock(p, centerX, block3CenterY, bw, bh, "f = |x − 2| + 1·c", FC_BR3_FILL, FC_BR3_BDR);
    int block3Bottom = block3CenterY + bh/2;

    int mergeY = block3Bottom + gap + 15;
    drawArrowLine(p, centerX, block3Bottom, centerX, mergeY);

    p.setPen(QPen(FC_ARROW, 2));
    p.drawLine(rightX + bw/2, rb1Bottom, routeX, rb1Bottom);
    p.drawLine(routeX, rb1Bottom, routeX, mergeY);
    p.drawLine(rightX + bw/2, rb2Bottom, routeX, rb2Bottom);
    p.drawLine(routeX, rb2Bottom, routeX, mergeY);
    p.drawLine(routeX, mergeY, centerX, mergeY);
    QPolygon arrowLeft;
    arrowLeft << QPoint(centerX, mergeY)
              << QPoint(centerX+10, mergeY-6)
              << QPoint(centerX+10, mergeY+6);
    p.setBrush(FC_ARROW);
    p.drawPolygon(arrowLeft);
    p.setBrush(Qt::NoBrush);

    y = mergeY + 8;

    // 6. Вывод
    drawArrowDown(p, centerX, mergeY, y + bh/2 - 4);
    y += bh/2;
    drawRoundedBlock(p, centerX, y, bw, bh, "Вывод f(x)", FC_OUT_FILL, FC_OUT_BDR);
    y1 = y + bh/2; y += bh + gap;
    drawArrowDown(p, centerX, y1, y - 18);

    // 7. Конец
    drawRoundedBlock(p, centerX, y, 140, 36, "Конец", FC_START_FILL, FC_START_BDR);

    p.restore();
}

// ──────────────────────────────────────────────────────────────────────────
SchemaDialog::SchemaDialog(QWidget *parent)
    : QDialog(parent)
{
    setStyleSheet(QString(
        "QDialog { background-color: #0d1117; color: #e6edf3; font-family: '%1'; }"
    ).arg(FONT_FAMILY));
    setupUI();
}

SchemaDialog::~SchemaDialog() {}

void SchemaDialog::setupUI()
{
    setWindowTitle("Блок-схема вычислительного процесса");

    // Канвас 700x735 + отступы 16*2 + кнопка 44 = ~800x800
    resize(800, 800);
    setFixedSize(800, 800);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(8);

    canvas = new FlowchartWidget(this);
    mainLayout->addWidget(canvas, 1, Qt::AlignHCenter | Qt::AlignVCenter);

    closeBtn = new QPushButton("Закрыть", this);
    closeBtn->setMinimumHeight(36);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #388bfd; color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
        "  font-size: 11pt; padding: 4px 20px;"
        "}"
        "QPushButton:hover { background-color: #58a6ff; }"
    );
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    btnRow->addWidget(closeBtn);
    btnRow->addStretch(1);
    mainLayout->addLayout(btnRow);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
