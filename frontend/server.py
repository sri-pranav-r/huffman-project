"""
server.py
---------
A very small web server for the Huffman project. It does NO compression
itself. It only:

  1. serves index.html (the drop-in-file page)
  2. receives an uploaded file, saves it to a temp folder
  3. runs the C program  ../backend/huff  on it
  4. sends the result file (and the program's printed output) back

Run:   python3 server.py        then open http://localhost:8000
Uses only the Python standard library.
"""

import base64
import json
import os
import subprocess
import tempfile
from http.server import BaseHTTPRequestHandler, HTTPServer

HERE = os.path.dirname(os.path.abspath(__file__))
BACKEND_DIR = os.path.join(HERE, "..", "backend")
HUFF = os.path.join(BACKEND_DIR, "huff")
INDEX = os.path.join(HERE, "index.html")
PORT = 8000


def build_backend_if_needed():
    """Compile the C program with 'make' if the binary is missing."""
    if not os.path.exists(HUFF):
        print("huff binary not found, running make in backend/ ...")
        subprocess.run(["make"], cwd=BACKEND_DIR, check=True)


def output_name(input_name, mode):
    """Decide the result file name: add .huf when compressing, remove it when decompressing."""
    if mode == "compress":
        return input_name + ".huf"
    if input_name.lower().endswith(".huf"):
        return input_name[:-4]
    return input_name + ".out"


class Handler(BaseHTTPRequestHandler):

    def send_json(self, status, payload):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        """Serve the single HTML page."""
        if self.path not in ("/", "/index.html"):
            self.send_error(404)
            return
        with open(INDEX, "rb") as f:
            body = f.read()
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        """Receive a file, run huff on it, return the result as JSON."""
        mode = self.path.strip("/")
        if mode not in ("compress", "decompress"):
            self.send_json(404, {"ok": False, "error": "unknown action"})
            return

        length = int(self.headers.get("Content-Length", 0))
        data = self.rfile.read(length)
        # keep only the base name so a path can never escape the temp folder
        in_name = os.path.basename(self.headers.get("X-Filename", "file")) or "file"
        out_name = output_name(in_name, mode)

        with tempfile.TemporaryDirectory() as tmp:
            in_path = os.path.join(tmp, "input")
            out_path = os.path.join(tmp, "output")
            with open(in_path, "wb") as f:
                f.write(data)

            result = subprocess.run([HUFF, mode, in_path, out_path],
                                    capture_output=True, text=True)

            if result.returncode != 0 or not os.path.exists(out_path):
                # show the user's file name, not our temporary path
                message = result.stdout.strip().replace(in_path, in_name) or "huff failed"
                self.send_json(400, {"ok": False, "error": message})
                return

            with open(out_path, "rb") as f:
                out_data = f.read()

        self.send_json(200, {
            "ok": True,
            "mode": mode,
            "inputName": in_name,
            "outputName": out_name,
            "inputSize": len(data),
            "outputSize": len(out_data),
            "log": result.stdout,
            "data": base64.b64encode(out_data).decode("ascii"),
        })

    def log_message(self, fmt, *args):
        print("%s %s" % (self.command, self.path))


if __name__ == "__main__":
    build_backend_if_needed()
    print("Huffman frontend running at http://localhost:%d" % PORT)
    HTTPServer(("127.0.0.1", PORT), Handler).serve_forever()
