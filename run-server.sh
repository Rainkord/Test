#!/bin/bash
# Запуск СЕРВЕРА для демонстрации в локальной сети.
# Запускать на компьютере, который будет сервером.
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVER_DIR="$ROOT_DIR/server"

echo "========================================="
echo "  ТИМП Подгруппа 5 — СЕРВЕР (LAN)"
echo "========================================="
echo ""

# Остановить старый сервер
pkill -f "echoServer" 2>/dev/null || true
sleep 0.5

# Сборка
echo "[server] Сборка..."
cd "$SERVER_DIR"
qmake echoServer.pro 2>/dev/null
make -j"$(nproc)" 2>&1 | tail -1
echo "[server] Собрано."
echo ""

# Показать IP-адреса этого компьютера в локальной сети
echo "-----------------------------------------"
echo "  IP-адреса этого компьютера в сети:"
ip -4 addr show 2>/dev/null | grep -oP '(?<=inet\s)\d+(\.\d+){3}' | grep -v '127.0.0.1' | while read -r ip; do
    echo "    -> $ip"
done
echo ""
echo "  Введите ОДИН из этих адресов на клиенте:"
echo "    ./run-client.sh <этот_IP>"
echo "-----------------------------------------"
echo ""

# Запуск (порт 33333, слушает на всех интерфейсах 0.0.0.0)
echo "[server] Запуск на порту 33333 (Ctrl+C — остановить)..."
echo ""
exec ./echoServer
