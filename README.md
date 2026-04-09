# ТМП — Подгруппа 5

**Тема:** Графическое отображение ветвящейся функции в рамках клиент-серверного проекта

**Функция №9:** Комплексная функция с модулями и квадратичным членом

```
        ⎧ |x| − 2      , x < −2
f(x) =  ⎨ x² + x + 1   , −2 ≤ x < 2
        ⎩ |x − 2| + 1   , x ≥ 2
```

## Участники

| ФИО | Роль |
|-----|------|
| Орлов Руслан (Админ) | Сервер, GitHub |
| Карелин Кирилл | Численная задача, Doxygen, Отчёты |
| Серёгина Елизавета | БД, Docker |
| Воробьёва Елизавета | Тестирование, Отчёты |
| Сарафанов Алексей | UI (Qt), Авторизация/Регистрация |

## Структура проекта

```
├── server/          — Консольный TCP-сервер (эхо-сервер)
├── client/          — Оконный GUI-клиент (Qt Widgets)
├── docker/          — Dockerfile для сборки сервера
├── docs/            — Doxyfile для автогенерации документации
├── tests/           — Тесты
└── README.md
```

## Сборка и запуск

### Сервер
```bash
cd server
qmake echoServer.pro
make
./echoServer
```

### Клиент
```bash
cd client
qmake client.pro
make
./client
```

### Docker
```bash
docker build -t timp-group5 -f docker/Dockerfile .
docker run -it -p 33333:33333 timp-group5
```

### Документация
```bash
cd docs
doxygen Doxyfile
# Результат: docs/output/html/index.html
```

### Тесты
```bash
cd tests
g++ -o test_calculator test_calculator.cpp -std=c++11
./test_calculator
```
