import socket
import subprocess
import time
import sys
import os

# Configuration ajustée pour l'exécution depuis le dossier tests/
HOST = "127.0.0.1"
PORT = 8088
BIN = "../httpd"            # Le binaire est dans le dossier parent
CONFIG = "test_config.conf" # On crée la config dans le dossier courant (tests/)

def create_dummy_config():
    # root_dir = .. car index.html est à la racine du projet, pas dans tests/
    with open(CONFIG, "w") as f:
        f.write(f"""[global]
pid_file = /tmp/httpd_test.pid
log = true

[[vhosts]]
server_name = localhost
ip = {HOST}
port = {PORT}
root_dir = ..
""")

def send_request(payload):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            s.sendall(payload.encode())
            data = s.recv(4096)
            return data.decode()
    except Exception as e:
        return str(e)

def run_tests():
    create_dummy_config()
    
    # Vérification que le binaire existe
    if not os.path.exists(BIN):
        print(f"Error: Binary {BIN} not found. Did you run 'make' in root?")
        sys.exit(1)

    # Lancement du serveur
    # On utilise BIN (../httpd)
    proc = subprocess.Popen([BIN, "--server_name", "localhost", "--port", str(PORT), "--ip", HOST, "--root_dir", "..", "--pid_file", "/tmp/test_httpd.pid"])
    
    print("Waiting for server start...")
    time.sleep(1)

    tests_failed = 0

    print("\n--- INTEGRATION TESTS ---")

    # TEST 1: Valid GET
    print("[TEST] Valid GET /index.html ... ", end="")
    res = send_request(f"GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n")
    if "HTTP/1.1 200 OK" in res:
        print("PASS")
    else:
        print(f"FAIL (Got: {res[:50]}...)")
        tests_failed += 1

    # TEST 2: Bad Host (400)
    print("[TEST] Bad Host Header ... ", end="")
    res = send_request(f"GET /index.html HTTP/1.1\r\nHost: hacker.com\r\n\r\n")
    if "400 Bad Request" in res:
        print("PASS")
    else:
        print(f"FAIL (Got: {res[:50]}...)")
        tests_failed += 1

    # TEST 3: Bad Version (505)
    print("[TEST] Bad Version (HTTP/1.0) ... ", end="")
    res = send_request(f"GET /index.html HTTP/1.0\r\nHost: localhost\r\n\r\n")
    if "505 HTTP Version Not Supported" in res:
        print("PASS")
    else:
        print(f"FAIL (Got: {res[:50]}...)")
        tests_failed += 1

    # TEST 4: 404 Not Found
    print("[TEST] 404 Not Found ... ", end="")
    res = send_request(f"GET /introuvable.html HTTP/1.1\r\nHost: localhost\r\n\r\n")
    if "404 Not Found" in res:
        print("PASS")
    else:
        print(f"FAIL (Got: {res[:50]}...)")
        tests_failed += 1

    # Cleanup
    proc.terminate()
    if os.path.exists(CONFIG):
        os.remove(CONFIG)

    sys.exit(tests_failed)

if __name__ == "__main__":
    run_tests()
    