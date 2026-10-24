"""Serve the embedded graph from USB serial, or explicitly simulated data."""
import argparse
import json
import math
import threading
import time
from collections import deque
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
START = time.monotonic()
SAMPLES = deque(maxlen=80)
LOCK = threading.Lock()
DEMO = False


def serial_reader(port):
    import serial

    sequence = 0
    while True:
        try:
            with serial.Serial(port, 115200, timeout=1) as device:
                while True:
                    line = device.readline().decode("ascii", errors="ignore").strip()
                    try:
                        raw = int(line)
                    except ValueError:
                        continue
                    if not -8388608 <= raw <= 8388607:
                        continue
                    sequence += 1
                    with LOCK:
                        SAMPLES.append([sequence, int((time.monotonic() - START) * 1000), raw])
        except (serial.SerialException, OSError) as exc:
            print(f"Serial unavailable: {exc}; retrying in 2 s", flush=True)
            time.sleep(2)


def readings():
    now = int((time.monotonic() - START) * 1000)
    if DEMO:
        seq = now // 100
        samples = [[i, i * 100, round(125000 + 12000 * math.sin(i / 40) + 80 * math.sin(i * 2.7))]
                   for i in range(max(1, seq - 79), seq + 1)]
    else:
        with LOCK:
            samples = list(SAMPLES)
    return {"uptime": now, "ok": bool(samples) and now - samples[-1][1] < 2000,
            "demo": DEMO, "samples": samples}


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/api/readings":
            body = json.dumps(readings()).encode()
            content_type = "application/json"
        elif self.path == "/":
            source = (ROOT / "dashboard.h").read_text()
            body = source.split('R"HTML(', 1)[1].split(')HTML";', 1)[0].encode()
            content_type = "text/html; charset=utf-8"
        else:
            self.send_error(404)
            return
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, *_):
        pass


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--port", help="USB serial port, e.g. /dev/cu.usbserial-10")
    mode.add_argument("--demo", action="store_true", help="explicitly simulated data")
    parser.add_argument("--http-port", type=int, default=8000)
    args = parser.parse_args()
    DEMO = args.demo
    if not DEMO:
        try:
            import serial
        except ImportError:
            parser.error("Install pyserial: python3 -m pip install -r requirements.txt")
    server = ThreadingHTTPServer(("127.0.0.1", args.http_port), Handler)
    if not DEMO:
        threading.Thread(target=serial_reader, args=(args.port,), daemon=True).start()
    print(f"{'SIMULATED' if DEMO else 'USB LIVE'} graph: http://127.0.0.1:{args.http_port}", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
