#!/bin/bash
# Fixes Docker DNS issues on Arch/CachyOS (and other Linux distros).
# Run once if 'docker build' fails with "Ign:" or "Could not resolve" errors.

set -e

DAEMON_JSON="/etc/docker/daemon.json"

echo "[*] Checking Docker daemon config..."

if [ -f "$DAEMON_JSON" ]; then
    echo "[!] $DAEMON_JSON already exists:"
    cat "$DAEMON_JSON"
    echo
    read -p "Overwrite with DNS fix? [y/N] " confirm
    [[ "$confirm" =~ ^[Yy]$ ]] || { echo "Aborted."; exit 0; }
fi

echo "[*] Creating /etc/docker/ directory if missing..."
sudo mkdir -p /etc/docker

echo "[*] Writing DNS config to $DAEMON_JSON..."
sudo tee "$DAEMON_JSON" > /dev/null <<'EOF'
{
  "dns": ["8.8.8.8", "1.1.1.1"]
}
EOF

echo "[*] Restarting Docker..."
sudo systemctl restart docker

echo "[+] Done! Test with:"
echo "    docker run --rm ubuntu:22.04 nslookup archive.ubuntu.com"
