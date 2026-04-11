#include <QtTest>
#include "calculator.h"
#include <cmath>

/**
 * @file tst_calculator.cpp
 * @brief Модульные тесты для класса Calculator.
 *
 * Проверяют все три ветви функции f(x) и корректность
 * генерации строки данных графика.
 */

/**
 * @class TstCalculator
 * @brief Класс модульных тестов для Calculator.
 */
class TstCalculator : public QObject
{
    Q_OBJECT

private slots:

    /**
     * @brief Тест ветви x < -2: f(x) = |x*a| - 2
     *
     * При x=-3, a=2: |(-3)*2| - 2 = 6 - 2 = 4.0
     */
    void testBranch1_negative()
    {
        double result = Calculator::calculate(-3.0, 2.0, 1.0, 1.0);
        QCOMPARE(result, 4.0);
    }

    /**
     * @brief Тест ветви x < -2 с отрицательным a: |x*a| должен давать положительное значение.
     *
     * При x=-4, a=-1: |(-4)*(-1)| - 2 = 4 - 2 = 2.0
     */
    void testBranch1_negativeA()
    {
        double result = Calculator::calculate(-4.0, -1.0, 1.0, 1.0);
        QCOMPARE(result, 2.0);
    }

    /**
     * @brief Тест ветви -2 <= x < 2: f(x) = b*x^2 + x + 1
     *
     * При x=0, b=1: 1*0 + 0 + 1 = 1.0
     */
    void testBranch2_atZero()
    {
        double result = Calculator::calculate(0.0, 1.0, 1.0, 1.0);
        QCOMPARE(result, 1.0);
    }

    /**
     * @brief Тест ветви -2 <= x < 2: граничное значение x = -2.
     *
     * При x=-2, b=2: 2*4 + (-2) + 1 = 8 - 2 + 1 = 7.0
     */
    void testBranch2_leftBoundary()
    {
        double result = Calculator::calculate(-2.0, 1.0, 2.0, 1.0);
        QCOMPARE(result, 7.0);
    }

    /**
     * @brief Тест ветви x >= 2: f(x) = |x - 2| + c
     *
     * При x=2, c=3: |2-2| + 3 = 0 + 3 = 3.0
     */
    void testBranch3_atBoundary()
    {
        double result = Calculator::calculate(2.0, 1.0, 1.0, 3.0);
        QCOMPARE(result, 3.0);
    }

    /**
     * @brief Тест ветви x >= 2 при x > 2.
     *
     * При x=5, c=1: |5-2| + 1 = 3 + 1 = 4.0
     */
    void testBranch3_positive()
    {
        double result = Calculator::calculate(5.0, 1.0, 1.0, 1.0);
        QCOMPARE(result, 4.0);
    }

    /**
     * @brief Тест generateGraphData: результат начинается с "graph".
     */
    void testGenerateGraphData_prefix()
    {
        QString data = Calculator::generateGraphData(-1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
        QVERIFY(data.startsWith("graph"));
    }

    /**
     * @brief Тест generateGraphData: корректное количество точек.
     *
     * При xMin=-1, xMax=1, step=1 должны получиться точки x=-1, 0, 1 → 3 точки.
     * Строка: "graph||x1;y1||x2;y2||x3;y3" — содержит 3 разделителя "||" после "graph".
     */
    void testGenerateGraphData_pointCount()
    {
        QString data = Calculator::generateGraphData(-1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
        // Убираем первый "graph", считаем оставшиеся части
        QStringList parts = data.split("||");
        // parts[0] = "graph", parts[1..3] = точки
        QCOMPARE(parts.size(), 4); // "graph" + 3 точки
    }

    /**
     * @brief Тест generateGraphData: при некорректном step (<=0) подставляется 0.1.
     *
     * Результат не должен быть пустым и должен начинаться с "graph".
     */
    void testGenerateGraphData_invalidStep()
    {
        QString data = Calculator::generateGraphData(0.0, 0.5, -1.0, 1.0, 1.0, 1.0);
        QVERIFY(!data.isEmpty());
        QVERIFY(data.startsWith("graph"));
    }
};

QTEST_MAIN(TstCalculator)
#include "tst_calculator.moc"
