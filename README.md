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
## Настройка server/email.txt

Файл `server/email.txt` хранит учётные данные почты, от имени которой сервер отправляет письма с кодами подтверждения (регистрация, сброс пароля).

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
cd ./server
cmake -B build && cmake --build build -j$(nproc)
./build/server
```

---

## Сборка клиента

```bash
cd ./client
qmake client.pro
make -j$(nproc)
../build/client
```

---

## Docker

Docker используется для запуска **серверной части** в изолированном окружении (Ubuntu 22.04).
Образ сам собирает проект и запускает `echoServer` на порту `33333`.

### Подготовка

Перед сборкой образа обязательно заполните `server/email.txt` (см. раздел ниже) —
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
   Скачать и запустить на любой машине:
   ```bash
   docker pull <ваш_логин>/timp-server:latest
   docker run -d --name timp-server -p 33333:33333 <ваш_логин>/timp-server:latest
   ```

---

## Модульные тесты

Тесты находятся в папке `tests/` и проверяют класс `Calculator` — вычисление ветвящейся функции 	9 и генерацию данных для графика.

В `tests/` есть два файла:
- `tst_calculator.cpp` — тесты на фреймворке **Qt Test** (11 тест-кейсов)
- `test_calculator.cpp` — автономные тесты на чистом C++11, без зависимости от Qt

### Способ 1: Qt Test (основной)

```bash
cd tests
qmake tests.pro
make
./tests
```

Ожидаемый вывод:
```
********* Start testing of TstCalculator *********
PASS   : TstCalculator::testBranch1_negative()
PASS   : TstCalculator::testBranch2_atZero()
PASS   : TstCalculator::testBranch3_atBoundary()
...
Totals: 11 passed, 0 failed, 0 skipped
********* Finished testing of TstCalculator *********
```

### Способ 2: чистый C++ (без Qt)

```bash
cd tests
g++ -o test_calculator test_calculator.cpp -std=c++11
./test_calculator
```

Ожидаемый вывод:
```
[OK] f(-5) = 3 (ожидалось 3)
[OK] f(0)  = 1 (ожидалось 1)
...
=== Все тесты пройдены! ===
```

> ℹ️ Способ 1 — основной, т.к. проект на Qt. Способ 2 — альтернативный, запускается на любой машине без установленного Qt.

---

## Документация (Doxygen)

```bash
### Установка Doxygen
sudo pacman -S doxygen graphviz
```

```bash
doxygen docs/Doxyfile && xdg-open docs/output/html/index.html
```

---

## Компоненты клиента

| Файл | Назначение |
|---|---|
| `authwidget.cpp/h` | Экран авторизации |
| `regwidget.cpp/h` | Экран регистрации + подтверждение email |
| `resetwidget.cpp/h` | Восстановление пароля (3 шага) |
| `verifywidget.cpp/h` | Подтверждение кода при входе (2FA) |
| `clientsingleton.cpp/h` | TCP-соединение с сервером (Singleton) |
| `mainwindow.cpp/h` | Главное окно, переключение экранов |
| `graphwidget.cpp/h` | Граф / блок-схема (основная функция приложения) |

---
