#!/bin/bash
# Запуск КЛИЕНТА для демонстрации в локальной сети.
# Запускать на компьютере, который будет клиентом.
#
# Использование:
#   ./run-client.sh <IP_сервера>        # например: ./run-client.sh 192.168.1.50
#   ./run-client.sh <IP_сервера> <порт> # если порт отличается от 33333
#   ./run-client.sh                     # без аргумента -> подключение к 127.0.0.1 (локально)
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
CLIENT_DIR="$ROOT_DIR/client"
CLIENT_BIN="$ROOT_DIR/build/client"

SERVER_HOST="${1:-127.0.0.1}"
SERVER_PORT="${2:-33333}"

echo "========================================="
echo "  ТИМП Подгруппа 5 — КЛИЕНТ (LAN)"
echo "========================================="
echo "  Сервер: $SERVER_HOST:$SERVER_PORT"
echo "========================================="
echo ""

# Остановить старый клиент
pkill -f "build/client" 2>/dev/null || true
sleep 0.3

# Сборка
echo "[client] Сборка..."
cd "$CLIENT_DIR"
qmake client.pro 2>/dev/null
make -j"$(nproc)" 2>&1 | tail -1
echo "[client] Собрано."
echo ""

# Проверка доступности сервера
echo "[client] Проверка соединения с $SERVER_HOST:$SERVER_PORT ..."
if timeout 3 bash -c "echo > /dev/tcp/$SERVER_HOST/$SERVER_PORT" 2>/dev/null; then
    echo "[client] Сервер доступен."
else
    echo "[client] ВНИМАНИЕ: сервер $SERVER_HOST:$SERVER_PORT недоступен!"
    echo "          Проверьте, что сервер запущен и firewall не блокирует порт."
fi
echo ""

# Запуск клиента с указанием адреса сервера через переменные окружения
echo "[client] Запуск..."
export TIMP_SERVER_HOST="$SERVER_HOST"
export TIMP_SERVER_PORT="$SERVER_PORT"
exec "$CLIENT_BIN"
