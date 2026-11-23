import os
import sys

method = os.environ.get("REQUEST_METHOD", "")
query = os.environ.get("QUERY_STRING", "")
body = sys.stdin.buffer.read()

sys.stdout.buffer.write(b"Content-Type: text/plain\r\n\r\n")
sys.stdout.buffer.write(f"method={method}\nquery={query}\n".encode())
sys.stdout.buffer.write(body)
