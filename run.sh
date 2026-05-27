#!/bin/bash
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVER_DIR="$ROOT_DIR/server"
CLIENT_DIR="$ROOT_DIR/client"
SERVER_BIN="$SERVER_DIR/echoServer"
CLIENT_BIN="$ROOT_DIR/build/client"
SERVER_PID=""

cleanup() {
    echo ""
    echo "[run] Остановка сервера..."
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null
        wait "$SERVER_PID" 2>/dev/null
    fi
    echo "[run] Готово."
    exit 0
}

trap cleanup SIGINT SIGTERM EXIT

echo "========================================="
echo "  ТИМП Подгруппа 5 — Сборка и запуск"
echo "========================================="
echo ""

# Убить старые экземпляры
echo "[run] Поиск и остановка старых процессов..."
pkill -f "echoServer" 2>/dev/null || true
pkill -f "$CLIENT_BIN" 2>/dev/null || true
sleep 0.5

# Сборка сервера
echo "[run] Сборка сервера..."
cd "$SERVER_DIR"
qmake echoServer.pro 2>/dev/null
make -j$(nproc) 2>&1 | tail -1
echo "[run] Сервер собран."

# Запуск сервера
echo "[run] Запуск сервера на порту 33333..."
cd "$SERVER_DIR"
./echoServer &
SERVER_PID=$!
sleep 1

# Проверка что сервер запустился
if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "[run] ОШИБКА: Сервер не запустился!"
    exit 1
fi
echo "[run] Сервер запущен (PID: $SERVER_PID)."

# Сборка клиента
echo "[run] Сборка клиента..."
cd "$CLIENT_DIR"
qmake client.pro 2>/dev/null
make -j$(nproc) 2>&1 | tail -1
echo "[run] Клиент собран."

# Запуск клиента
echo "[run] Запуск клиента..."
"$CLIENT_BIN" &
CLIENT_PID=$!
sleep 0.5

if ! kill -0 "$CLIENT_PID" 2>/dev/null; then
    echo "[run] ОШИБКА: Клиент не запустился!"
    cleanup
    exit 1
fi

echo "[run] Клиент запущен (PID: $CLIENT_PID)."
echo ""
echo "========================================="
echo "  Сервер и клиент работают."
echo "  Ctrl+C для остановки обоих."
echo "========================================="
echo ""

wait "$CLIENT_PID" 2>/dev/null || true
