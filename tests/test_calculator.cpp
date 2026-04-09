#include <iostream>
#include <cmath>
#include <cassert>

// Копия функции calculate из calculator.cpp (для автономного тестирования)
double calculate(double x)
{
    if (x < -2.0) {
        return std::fabs(x) - 2.0;
    } else if (x >= -2.0 && x < 2.0) {
        return x * x + x + 1.0;
    } else {
        return std::fabs(x - 2.0) + 1.0;
    }
}

bool approxEqual(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) < eps;
}

int main()
{
    std::cout << "=== Тестирование функции №9 ===" << std::endl;
    std::cout << "f(x) = |x|-2 при x<-2, x^2+x+1 при -2<=x<2, |x-2|+1 при x>=2" << std::endl;
    std::cout << std::endl;

    // Тест 1: x < -2 (левая ветвь: |x| - 2)
    // f(-5) = |-5| - 2 = 5 - 2 = 3
    assert(approxEqual(calculate(-5.0), 3.0));
    std::cout << "[OK] f(-5) = " << calculate(-5.0) << " (ожидалось 3)" << std::endl;

    // f(-3) = |-3| - 2 = 3 - 2 = 1
    assert(approxEqual(calculate(-3.0), 1.0));
    std::cout << "[OK] f(-3) = " << calculate(-3.0) << " (ожидалось 1)" << std::endl;

    // f(-2.5) = |-2.5| - 2 = 2.5 - 2 = 0.5
    assert(approxEqual(calculate(-2.5), 0.5));
    std::cout << "[OK] f(-2.5) = " << calculate(-2.5) << " (ожидалось 0.5)" << std::endl;

    // Тест 2: -2 <= x < 2 (средняя ветвь: x^2 + x + 1)
    // f(-2) = 4 + (-2) + 1 = 3
    assert(approxEqual(calculate(-2.0), 3.0));
    std::cout << "[OK] f(-2) = " << calculate(-2.0) << " (ожидалось 3)" << std::endl;

    // f(0) = 0 + 0 + 1 = 1
    assert(approxEqual(calculate(0.0), 1.0));
    std::cout << "[OK] f(0) = " << calculate(0.0) << " (ожидалось 1)" << std::endl;

    // f(1) = 1 + 1 + 1 = 3
    assert(approxEqual(calculate(1.0), 3.0));
    std::cout << "[OK] f(1) = " << calculate(1.0) << " (ожидалось 3)" << std::endl;

    // f(-1) = 1 + (-1) + 1 = 1
    assert(approxEqual(calculate(-1.0), 1.0));
    std::cout << "[OK] f(-1) = " << calculate(-1.0) << " (ожидалось 1)" << std::endl;

    // Тест 3: x >= 2 (правая ветвь: |x-2| + 1)
    // f(2) = |2-2| + 1 = 0 + 1 = 1
    assert(approxEqual(calculate(2.0), 1.0));
    std::cout << "[OK] f(2) = " << calculate(2.0) << " (ожидалось 1)" << std::endl;

    // f(5) = |5-2| + 1 = 3 + 1 = 4
    assert(approxEqual(calculate(5.0), 4.0));
    std::cout << "[OK] f(5) = " << calculate(5.0) << " (ожидалось 4)" << std::endl;

    // f(10) = |10-2| + 1 = 8 + 1 = 9
    assert(approxEqual(calculate(10.0), 9.0));
    std::cout << "[OK] f(10) = " << calculate(10.0) << " (ожидалось 9)" << std::endl;

    // Тест 4: граничные точки (непрерывность)
    // Граница x = -2:  левая ветвь f(-2-eps) -> |-2|-2 = 0, средняя f(-2) = 3
    // (функция НЕ непрерывна в x=-2, это нормально для ветвящейся функции)
    std::cout << std::endl;
    std::cout << "Граничные значения:" << std::endl;
    std::cout << "  f(-2.001) = " << calculate(-2.001) << " (левая ветвь)" << std::endl;
    std::cout << "  f(-2.000) = " << calculate(-2.0)   << " (средняя ветвь)" << std::endl;
    std::cout << "  f(1.999)  = " << calculate(1.999)  << " (средняя ветвь)" << std::endl;
    std::cout << "  f(2.000)  = " << calculate(2.0)    << " (правая ветвь)" << std::endl;

    std::cout << std::endl;
    std::cout << "=== Все тесты пройдены! ===" << std::endl;

    return 0;
}
