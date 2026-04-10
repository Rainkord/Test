# ТМП — Подгруппа 5
---

## Структура проекта

```
├── client/          # Qt-приложение (виджеты, GUI)
├── server/          # Серверная часть (обработка запросов, БД)
├── docker/          # Dockerfile и вспомогательные скрипты
└── README.md
```

---

## Сборка клиента

```bash
cd client
qmake client.pro
make -j$(nproc)
./client
```

## Сборка сервера

```bash
cd server
cmake -B build && cmake --build build -j$(nproc)
./build/server
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

- `-f docker/Dockerfile` — указываем путь к Dockerfile
- `-t timp-server` — даём имя образу
- `.` — контекст сборки (корень репозитория), нужен чтобы Docker видел папку `server/`

### Запуск контейнера

```bash
docker run -d --name timp-server -p 33333:33333 timp-server
```

- `-d` — запуск в фоновом режиме
- `--name timp-server` — имя контейнера
- `-p 33333:33333` — пробрасываем порт наружу: клиент подключается на `localhost:33333`

### Полезные команды

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

Если `docker build` падает с ошибкой разрешения имён (не может скачать пакеты):

```bash
chmod +x docker/fix-dns.sh
./docker/fix-dns.sh
```

После этого повторите `docker build`.

---

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
