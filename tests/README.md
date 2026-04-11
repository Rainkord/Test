# Модульные тесты (Qt Test)

Тесты проверяют класс `Calculator` — вычисление ветвящейся функции и генерацию данных графика.

## Как запустить

```bash
cd tests
qmake tests.pro
make
./tests
```

### Ожидаемый вывод

```
********* Start testing of TstCalculator *********
PASS   : TstCalculator::testBranch1_negative()
PASS   : TstCalculator::testBranch1_negativeA()
PASS   : TstCalculator::testBranch2_atZero()
PASS   : TstCalculator::testBranch2_leftBoundary()
PASS   : TstCalculator::testBranch3_atBoundary()
PASS   : TstCalculator::testBranch3_positive()
PASS   : TstCalculator::testGenerateGraphData_prefix()
PASS   : TstCalculator::testGenerateGraphData_pointCount()
PASS   : TstCalculator::testGenerateGraphData_invalidStep()
Finished testing of TstCalculator
********* Finished testing of TstCalculator *********
```

## Что тестируется

| Тест | Ветвь | Проверка |
|---|---|---|
| `testBranch1_negative` | x < −2 | `\|x·a\| − 2` при a > 0 |
| `testBranch1_negativeA` | x < −2 | `\|x·a\|` с отрицательным a (модуль) |
| `testBranch2_atZero` | −2 ≤ x < 2 | x = 0 |
| `testBranch2_leftBoundary` | −2 ≤ x < 2 | граничное x = −2 |
| `testBranch3_atBoundary` | x ≥ 2 | граничное x = 2 |
| `testBranch3_positive` | x ≥ 2 | x > 2 |
| `testGenerateGraphData_prefix` | — | строка начинается с `"graph"` |
| `testGenerateGraphData_pointCount` | — | корректное число точек |
| `testGenerateGraphData_invalidStep` | — | защита от step ≤ 0 |
