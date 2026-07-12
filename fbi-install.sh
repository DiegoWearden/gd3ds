#!/usr/bin/env bash
# Build the game and push the .cia to the 3DS through FBI's
# Remote Install -> Receive URLs over the network.
#
# On the 3DS, open FBI -> Remote Install -> Receive URLs over the network,
# then run this script. It serves output/ over HTTP, sends the .cia URL to
# FBI, waits for the download to finish, and exits by itself.
#
# Usage: ./fbi-install.sh [--no-build]
#   DS_IP=x.x.x.x  3DS address   (default 192.168.1.83)
#   DS_PORT=n      FBI port      (default 5000)
#   PORT=n         HTTP port     (default 8951)
set -euo pipefail
cd "$(dirname "$0")"

DS_IP="${DS_IP:-192.168.1.83}"
DS_PORT="${DS_PORT:-5000}"
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

exec python3 - "$IP" "$PORT" "$DS_IP" "$DS_PORT" "$CIA" <<'EOF'
import functools
import http.server
import os
import socket
import struct
import sys
import threading

host_ip, port, ds_ip, ds_port, cia = (
    sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4]), sys.argv[5])
url = f"http://{host_ip}:{port}/{cia}"
cia_mb = os.path.getsize(os.path.join("output", cia)) / 1e6

done = threading.Event()

class Handler(http.server.SimpleHTTPRequestHandler):
    def copyfile(self, source, outputfile):
        super().copyfile(source, outputfile)
        # Full response body written without error -> download finished
        if self.command == "GET" and self.path.lstrip("/") == cia:
            done.set()

    def log_message(self, fmt, *args):
        print("  3DS:", fmt % args)

server = http.server.ThreadingHTTPServer(
    ("0.0.0.0", port), functools.partial(Handler, directory="output"))
threading.Thread(target=server.serve_forever, daemon=True).start()

print(f"  Sending {url}")
print(f"  to FBI at {ds_ip}:{ds_port} ...")
try:
    with socket.create_connection((ds_ip, ds_port), timeout=5) as s:
        payload = url.encode()
        s.sendall(struct.pack(">I", len(payload)) + payload)
except OSError as e:
    print(f"  error: could not reach FBI at {ds_ip}:{ds_port} ({e})")
    print("  On the 3DS: FBI -> Remote Install -> Receive URLs over the network")
    sys.exit(1)

print(f"  Confirm the install on the 3DS. Serving {cia} ({cia_mb:.1f} MB)...")
done.wait()
print("  Download complete - FBI is installing. Done.")
server.shutdown()
EOF
