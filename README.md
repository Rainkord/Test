 # ТИМП — Подгруппа 5
---

## Структура проекта

```
├── client/          # Qt-приложение (виджеты, GUI)
├── server/          # Серверная часть (обработка запросов, БД)
├── docker/          # Dockerfile и вспомогательные скрипты
├── tests/           # Модульные тесты (Qt Test)
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

1. Откройте файл `server/email.txt`
2. В поле `email=` укажите адрес Gmail-почты, от которой будут рассылаться письма:
   ```
   email=your_address@gmail.com
   ```
3. В поле `key=` укажите **пароль приложения** (не пароль от аккаунта Google):
   ```
   key=xxxx xxxx xxxx xxxx
   ```

**Как получить пароль приложения Google:**

1. Перейдите на [https://myaccount.google.com/apppasswords](https://myaccount.google.com/apppasswords)
2. Войдите в аккаунт Google, от которого будет работать рассылка
3. В поле «Название приложения» введите любое имя (например, `TIMP Server`)
4. Нажмите **Создать** — Google выдаст 16-символьный пароль вида `xxxx xxxx xxxx xxxx`
5. Скопируйте его в поле `key=` файла `email.txt`

> ⚠️ **Важно:** файл `email.txt` добавлен в `.gitignore` — не коммитьте его в репозиторий, чтобы не раскрыть учётные данные.

**Готовый пример:**
```
# Ключ можно получить: https://myaccount.google.com/apppasswords
email=example@gmail.com
key=abcd efgh ijkl mnop
```

---

## Сборка сервера

```bash
cd server
qmake echoServer.pro
make -j$(nproc)
./echoServer
```

---

## Сборка клиента

```bash
cd client
qmake client.pro
make -j$(nproc)
../build/client
```

---

## Docker

Docker используется для запуска **серверной части** в изолированном окружении (Ubuntu 22.04).
Образ сам собирает проект и запускает `echoServer` на порту `33333`.

### Подготовка

Перед сборкой образа обязательно заполните `server/email.txt` (см. раздел выше) —
`Dockerfile` копирует его внутрь образа.

### Сборка образа

Все команды выполнять из корня репозитория:

```bash
docker build -f docker/Dockerfile -t timp-server .
```

### Запуск контейнера

```bash
docker run -d --name timp-server -p 33333:33333 timp-server
```

```bash
# Посмотреть логи сервера в реальном времени
docker logs -f timp-server

# Остановить контейнер
docker stop timp-server

# Запустить снова
docker start timp-server

# Удалить контейнер полностью
docker rm -f timp-server
```

### Проблема с DNS при сборке

```bash
chmod +x docker/fix-dns.sh
./docker/fix-dns.sh
```

### Публикация образа на Docker Hub

1. Зарегистрируйтесь на [hub.docker.com](https://hub.docker.com) (если нет аккаунта)
2. Войдите в Docker из терминала:
```bash
docker login
```
3. Пересоберите образ с тегом вашего логина:
```bash
docker build -f docker/Dockerfile -t <ваш_логин>/timp-server:latest .
```
4. Запушьте образ:
```bash
docker push <ваш_логин>/timp-server:latest
```
5. После этого образ будет доступен по адресу `hub.docker.com/<ваш_логин>/timp-server`.
Скачать и запустить на любой машႀне:
```bash
docker pull <ваш_логин>/timp-server:latest
docker run -d --name timp-server -p 33333:33333 <ваш_логин>/timp-server:latest
```

### На данный момент актуальная команда для запуска:

```bash
docker pull rainkord/timp-server:latest
docker run -d --name timp-server -p 33333:33333 rainkord/timp-server:latest
```

---

## Модульные тесты (Qt Test)

```bash
cd tests
qmake tests.pro
make -j$(nproc)
./tests
```

---

## Документация (Doxygen)

```bash
# Установка Doxygen (если ещё не установлен)
sudo pacman -S doxygen graphviz
```

```bash
# Генерация документации и открытие в браузере
doxygen docs/Doxyfile && xdg-open docs/output/html/index.html
```

Сгенерированный HTML находится в `docs/output/html/` и закоммичен в репозиторий.
LaTeX, XML и прочие форматы вывода игнорируются через `.gitignore`.

---

## Компоненты клиента

| Файл | Назначение | Ключевые методы / слоты | Qt-инструменты |
|---|---|---|---|
| `authwidget.cpp/h` | Экран авторизации (логин + пароль) | `onLoginClicked()`, `onForgotClicked()` | `QWidget`, `QLineEdit`, `QPushButton`, `QTcpSocket` через `ClientSingleton` |
| `regwidget.cpp/h` | Экран регистрации (3 шага) | `onNextClicked()`, `onRegisterClicked()`, `onBackClicked()` | `QStackedWidget`, `QLineEdit`, `QRegularExpressionValidator` |
| `verifywidget.cpp/h` | Ввод кода подтверждения email (регистрация) | `onConfirmClicked()`, `onResendClicked()` | `QLineEdit`, `QTimer` (обратный отсчёт) |
| `resetwidget.cpp/h` | Восстановление пароля (3 шага: email → код → новый пароль) | `onSendCodeClicked()`, `onVerifyClicked()`, `onResetClicked()` | `QStackedWidget`, `QTimer`, `QLineEdit` |
| `clientsingleton.cpp/h` | TCP-соединение с сервером (Singleton, JSON-протокол) | `instance()`, `sendRequest()`, `readResponse()` | `QTcpSocket`, `QJsonDocument`, `QJsonObject` |
| `mainwindow.cpp/h` | Главное окно — переключение между экранами | `showAuth()`, `showMain()`, `showGraph()` | `QMainWindow`, `QStackedWidget` |
| `graphwidget.cpp/h` | Отрисовка графа / блок-схемы алгоритма | `paintEvent()`, `addNode()`, `addEdge()`, `exportToPng()` | `QPainter`, `QPainterPath`, `QGraphicsScene` / `QGraphicsView` |
| `schemadialog.cpp/h` | Диалог выбора / редактирования схемы | `onAccepted()`, `onRejected()`, `loadSchemas()` | `QDialog`, `QListWidget`, `QDialogButtonBox` |
| `taskdialog.cpp/h` | Диалог создания / редактирования задачи | `onSaveClicked()`, `populate()` | `QDialog`, `QLineEdit`, `QComboBox`, `QDateEdit` |

---

## Компоненты сервера

| Файл | Назначение | Ключевые методы / слоты | Qt-инструменты |
|---|---|---|---|
| `mytcpserver.cpp/h` | TCP-сервер: приём соединений, маршрутизация запросов | `slotNewConnection()`, `slotServerRead()`, `processBuffer()`, `sendResponse()`, `slotClientDisconnected()` | `QTcpServer`, `QTcpSocket`, `QJsonDocument` |
| `database.cpp/h` | Singleton-обёртка над SQLite: CRUD для пользователей, схем, задач | `instance()`, `addUser()`, `getUser()`, `addTask()`, `getTasks()`, `updateTask()`, `deleteTask()` | `QSqlDatabase`, `QSqlQuery` (драйвер `QSQLITE`) |
| `smtpclient.cpp/h` | Отправка писем через Gmail SMTP (SSL, порт 465) | `sendVerificationCode()`, `sendPasswordResetCode()` | `QSslSocket`, `QByteArray::toBase64()`, `QFile` / `QTextStream` (чтение `email.txt`) |
| `calculator.cpp/h` | Вычисление f(x) и генерация точек графика (тестируемый модуль) | `calculate()`, `generateGraphData()` | Чистый C++, `<cmath>` |
| `main.cpp` | Точка входа: инициализация `QCoreApplication` и `MyTcpServer` | — | `QCoreApplication` |

---
