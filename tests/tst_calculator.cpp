#include <QtTest>
#include "calculator.h"
#include <cmath>

class TstCalculator : public QObject
{
    Q_OBJECT

private slots:

    void testBranch1_negative()
    {
        double result = Calculator::calculate(-3.0, 2.0, 1.0, 1.0);
        QCOMPARE(result, 4.0);
    }

    void testBranch1_negativeA()
    {
        double result = Calculator::calculate(-4.0, -1.0, 1.0, 1.0);
        QCOMPARE(result, 2.0);
    }

    void testBranch2_atZero()
    {
        double result = Calculator::calculate(0.0, 1.0, 1.0, 1.0);
        QCOMPARE(result, 1.0);
    }

    void testBranch2_leftBoundary()
    {
        double result = Calculator::calculate(-2.0, 1.0, 2.0, 1.0);
        QCOMPARE(result, 7.0);
    }

    void testBranch3_atBoundary()
    {
        double result = Calculator::calculate(2.0, 1.0, 1.0, 3.0);
        QCOMPARE(result, 3.0);
    }

    void testBranch3_positive()
    {
        double result = Calculator::calculate(5.0, 1.0, 1.0, 1.0);
        QCOMPARE(result, 4.0);
    }

    void testGenerateGraphData_prefix()
    {
        QString data = Calculator::generateGraphData(-1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
        QVERIFY(data.startsWith("graph"));
    }

    void testGenerateGraphData_pointCount()
    {
        QString data = Calculator::generateGraphData(-1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
        // Убираем первый "graph", считаем оставшиеся части
        QStringList parts = data.split("||");
        // parts[0] = "graph", parts[1..3] = точки
        QCOMPARE(parts.size(), 4); // "graph" + 3 точки
    }

    void testGenerateGraphData_invalidStep()
    {
        QString data = Calculator::generateGraphData(0.0, 0.5, -1.0, 1.0, 1.0, 1.0);
        QVERIFY(!data.isEmpty());
        QVERIFY(data.startsWith("graph"));
    }
};

QTEST_MAIN(TstCalculator)
#include "tst_calculator.moc"
