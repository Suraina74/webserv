from pathlib import Path
import sys

message = Path("message.data").read_bytes()
sys.stdout.buffer.write(b"Content-Type: text/plain\r\n\r\n" + message)
