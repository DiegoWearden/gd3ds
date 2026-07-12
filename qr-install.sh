#!/usr/bin/env bash
# Build the game and serve the .cia over the local network with a QR code
# for FBI's Remote Install -> Scan QR Code. Ctrl+C stops the server.
#
# Usage: ./qr-install.sh [--no-build]   (PORT=xxxx to override the port)
set -euo pipefail
cd "$(dirname "$0")"

PORT="${PORT:-8951}"
CIA="GeometryDash3DS.cia"

if [[ "${1:-}" != "--no-build" ]]; then
    make
fi

if [[ ! -f "output/$CIA" ]]; then
    echo "error: output/$CIA not found" >&2
    exit 1
fi

IP=$(ip route get 1.1.1.1 2>/dev/null | grep -oP 'src \K[0-9.]+' | head -1)
if [[ -z "$IP" ]]; then
    IP=$(hostname -I | awk '{print $1}')
fi
if [[ -z "$IP" ]]; then
    echo "error: could not determine LAN IP" >&2
    exit 1
fi

URL="http://$IP:$PORT/$CIA"

echo
echo "  On the 3DS: FBI -> Remote Install -> Scan QR Code"
echo "  URL: $URL"
echo
qrencode -t ANSIUTF8 "$URL"
echo
echo "  Serving output/ on port $PORT — Ctrl+C to stop when the install finishes."
echo
python3 -m http.server "$PORT" --directory output --bind 0.0.0.0
