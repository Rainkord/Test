#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>

class Calculator
{
public:
    // Function #9:
    //   f(x) = |x| - 2          for x < -2
    //   f(x) = x^2 + x + 1      for -2 <= x < 2
    //   f(x) = |x - 2| + 1      for x >= 2
    static double calculate(double x);

    // Returns "graph||x1;y1||x2;y2||..."
    static QString generateGraphData(double xMin, double xMax, double step);

private:
    Calculator() = delete;
};

#endif // CALCULATOR_H
