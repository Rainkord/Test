# ТИМП — Подгруппа 5

Клиент-серверное приложение для графического отображения ветвящейся (кусочной) функции.
Реализовано в рамках дисциплины «Технологии Интернет и мультимедиа» (251/372).

---

## Технологии

- **Язык:** C++17
- **Фреймворк:** Qt 6 (Widgets, Network, SQL)
- **БД:** SQLite (через `QSqlDatabase`, драйвер `QSQLITE`)
- **Протокол:** TCP, порт `33333`, текстовый протокол с разделителем `||`
- **Почта:** SMTP через Gmail (SSL, порт 465)
- **Контейнеризация:** Docker (Ubuntu 22.04)
- **Тестирование:** Qt Test
- **Документация:** Doxygen (русский язык)

---

## Структура проекта

```
├── client/          # GUI-клиент (Qt Widgets)
├── server/          # TCP-сервер + БД + SMTP
├── docker/          # Dockerfile и скрипты
├── tests/           # Модульные тесты (Qt Test)
├── docs/            # Doxygen-конфиг и сгенерированная документация
│   ├── Doxyfile
│   ├── output/html/ # HTML-документация
│   └── документы/   # UML-диаграммы, тест-кейсы
├── .gitignore
└── README.md
```

---

## Настройка server/email.txt

Файл `server/email.txt` хранит учётные данные почты, от имени которой сервер отправляет письма с кодами подтверждения (регистрация, сброс пароля).

```bash
nano ./server/email.txt
```

**Формат файла:**
```
# Ключ можно получить: https://myaccount.google.com/apppasswords
email=
key=
```

**Заполнение:**

1. В поле `email=` укажите адрес Gmail-почты:
   ```
   email=your_address@gmail.com
   ```
2. В поле `key=` укажите **пароль приложения** (не пароль от аккаунта Google):
   ```
   key=xxxx xxxx xxxx xxxx
   ```

**Как получить пароль приложения Google:**

1. Перейдите на [https://myaccount.google.com/apppasswords](https://myaccount.google.com/apppasswords)
2. Войдите в аккаунт Google
3. В поле «Название приложения» введите любое имя (например, `TIMP Server`)
4. Нажмите **Создать** — Google выдаст 16-символьный пароль вида `xxxx xxxx xxxx xxxx`
5. Скопируйте его в поле `key=` файла `email.txt`

> ⚠️ Файл `email.txt` добавлен в `.gitignore` — не коммитьте его в репозиторий.

---

## Сборка и запуск

### Сервер

```bash
cd server
qmake echoServer.pro
make -j$(nproc)
./echoServer
```

### Клиент

```bash
cd client
qmake client.pro
make -j$(nproc)
../build/client
```

### Модульные тесты

```bash
cd tests
qmake tests.pro
make -j$(nproc)
./tests
```

---

## Docker

Docker используется для запуска серверной части в изолированном окружении (Ubuntu 22.04).
Образ собирает проект и запускает `echoServer` на порту `33333`.

### Подготовка

Перед сборкой образа заполните `server/email.txt` (см. раздел выше) — `Dockerfile` копирует его внутрь образа.

### Сборка и запуск

```bash
# Сборка образа (из корня репозитория)
docker build -f docker/Dockerfile -t timp-server .

# Запуск контейнера
docker run -d --name timp-server -p 33333:33333 timp-server

# Логи в реальном времени
docker logs -f timp-server

# Остановка / перезапуск / удаление
docker stop timp-server
docker start timp-server
docker rm -f timp-server
```

### Проблема с DNS при сборке

```bash
chmod +x docker/fix-dns.sh
./docker/fix-dns.sh
```

### Актуальная команда для запуска (Docker Hub)

```bash
docker pull rainkord/timp-server:latest
docker run -d --name timp-server -p 33333:33333 rainkord/timp-server:latest
```

### Публикация на Docker Hub

```bash
docker login
docker build -f docker/Dockerfile -t <ваш_логин>/timp-server:latest .
docker push <ваш_логин>/timp-server:latest
```

---

## Протокол обмена данными

Клиент и сервер общаются по TCP на порту `33333`. Сообщения разделены `||`.

**Формат запроса:** `команда||параметр1||параметр2||...`

| Команда | Параметры | Описание |
|---------|-----------|----------|
| `AUTH` | логин, пароль | Авторизация |
| `REG` | логин, пароль, email | Регистрация |
| `RESET` | email | Запрос сброса пароля |
| `RESET_CODE` | email, код | Подтверждение кода сброса |
| `RESET_NEW` | email, новый_пароль | Установка нового пароля |
| `CALC` | выражение | Вычисление значения функции |
| `GRAPH` | выражение, x_min, x_max, шаг | Генерация данных графика |

**Ответ сервера:** `OK||данные` или `ERROR||сообщение`

---

## Компоненты клиента

| Файл | Назначение | Ключевые методы / слоты | Qt-инструменты |
|------|------------|-------------------------|----------------|
| `authwidget.cpp/h` | Экран авторизации (логин + пароль, защита от перебора) | `onLoginClicked()`, `onForgotClicked()`, `onRegisterClicked()` | `QWidget`, `QLineEdit`, `QPushButton`, `QTimer` |
| `regwidget.cpp/h` | Экран регистрации (3 шага) | `onNextClicked()`, `onRegisterClicked()`, `showStep()` | `QStackedWidget`, `QLineEdit`, `QRegularExpressionValidator` |
| `verifywidget.cpp/h` | Ввод кода подтверждения email | `onConfirmClicked()`, `onResendClicked()` | `QLineEdit`, `QTimer` (обратный отсчёт) |
| `resetwidget.cpp/h` | Восстановление пароля (email → код → новый пароль) | `onSendCodeClicked()`, `onVerifyClicked()`, `onResetClicked()` | `QStackedWidget`, `QTimer`, `QLineEdit` |
| `otpinput.cpp/h` | Ввод одноразового кода (6 цифр) | `codeChanged`, `setCodeLength()` | `QLineEdit` |
| `clientsingleton.cpp/h` | TCP-соединение с сервером (Singleton) | `instance()`, `sendRequestAsync()`, `responseReceived` (signal) | `QTcpSocket` |
| `mainwindow.cpp/h` | Главное окно — переключение между экранами | `onShowAuth()`, `onShowRegister()`, `onLogout()`, `onTaskBtnClicked()` | `QMainWindow`, `QStackedWidget` |
| `graphwidget.cpp/h` | Построение и отрисовка графика функции | `evalFunction()`, `onBuildClicked()`, `onExportClicked()`, `paintEvent()` | `QPainter`, `QPainterPath` |
| `mainappwidget.cpp/h` | Контейнер основного интерфейса после входа | конструктор, навигация | `QWidget`, `QStackedWidget` |
| `schemadialog.cpp/h` | Диалог отображения блок-схемы алгоритма | конструктор, `schemaLabel` | `QDialog`, `QLabel`, `QScrollArea` |
| `taskdialog.cpp/h` | Диалог просмотра задания (вариант 9) | конструктор | `QDialog`, `QLabel` (Rich Text) |

---

## Компоненты сервера

| Файл | Назначение | Ключевые методы / слоты | Qt-инструменты |
|------|------------|-------------------------|----------------|
| `mytcpserver.cpp/h` | TCP-сервер: приём соединений, маршрутизация запросов | `slotNewConnection()`, `slotServerRead()`, `slotClientDisconnected()` | `QTcpServer`, `QTcpSocket` |
| `database.cpp/h` | Singleton-обёртка над SQLite: CRUD пользователей | `instance()`, `addUser()`, `checkUser()`, `userExists()`, `emailExists()`, `getUserEmail()`, `updatePasswordByEmail()` | `QSqlDatabase`, `QSqlQuery` |
| `smtpclient.cpp/h` | Отправка писем через Gmail SMTP (SSL, порт 465) | `sendVerificationCode()`, `sendPasswordResetCode()` | `QSslSocket`, `QByteArray::toBase64()` |
| `functionsforserver.cpp/h` | Диспетчер команд протокола `\|\|` | `processMessage()` | Чистый C++ + Qt |
| `calculator.cpp/h` | Вычисление f(x) и генерация точек графика | `calculate()`, `generateGraphData()` | `<cmath>` |
| `logger.h` | Логирование событий сервера | `log()`, `info()`, `warn()`, `error()` | Чистый C++ |
| `main.cpp` | Точка входа: инициализация `QCoreApplication` и `MyTcpServer` | — | `QCoreApplication` |

---

## Документация (Doxygen)

```bash
# Установка (Arch Linux)
sudo pacman -S doxygen graphviz

# Генерация документации
doxygen docs/Doxyfile

# Открытие в браузере
xdg-open docs/output/html/index.html
```

Сгенерированный HTML находится в `docs/output/html/` и закоммичен в репозиторий.
LaTeX, XML и прочие форматы вывода игнорируются через `.gitignore`.

### Дополнительные документы

- `docs/документы/uml_class_diagram.puml` — UML-диаграмма классов (PlantUML)
- `docs/документы/usecase_diagram.puml` — Диаграмма вариантов использования
- `docs/документы/test_cases.md` — Тест-кейсы
- `docs/документы/strategy_testing.md` — Стратегия тестирования

---

## График функции (Вариант 9)

Кусочная функция:

```
f(x) = { x² + 1,          при x < -2
        { 3·sin(x) + 2,    при -2 ≤ x ≤ 1
        { e^(0.5·x) - 1,   при x > 1
```

Клиент отправляет запрос `GRAPH||выражение||x_min||x_max||шаг`, сервер вычисляет точки и возвращает координаты для построения графика через `QPainter`.
