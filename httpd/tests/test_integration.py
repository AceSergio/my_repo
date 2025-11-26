import socket
import subprocess
import time
import os
import pytest

HOST = "127.0.0.1"
PORT = 8088
BIN = "../httpd"
CONFIG = "test_config_pytest.conf"

@pytest.fixture(scope="module")
def server():
    with open("index.html", "w") as f:
        f.write("Welcome to HTTPd")

    with open(CONFIG, "w") as f:
        f.write(f"""[global]
pid_file = /tmp/httpd_pytest.pid
log = true

[[vhosts]]
server_name = localhost
ip = {HOST}
port = {PORT}
root_dir = ..
""")

    if not os.path.exists(BIN):
        pytest.fail(f"Binary {BIN} not found. Did you run 'make'?")

    proc = subprocess.Popen([
        BIN, 
        "--server_name", "localhost", 
        "--port", str(PORT), 
        "--ip", HOST, 
        "--root_dir", ".", 
        "--pid_file", "/tmp/httpd_pytest.pid"
    ])
    
    time.sleep(1)
    
    if proc.poll() is not None:
        pytest.fail("Server failed to start.")

    yield proc

    proc.terminate()
    proc.wait()
    if os.path.exists(CONFIG): os.remove(CONFIG)
    if os.path.exists("index.html"): os.remove("index.html")
    if os.path.exists("/tmp/httpd_pytest.pid"):
        try: os.remove("/tmp/httpd_pytest.pid")
        except OSError: pass

def send_request(payload):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(2)
            s.connect((HOST, PORT))
            s.sendall(payload.encode())
            
            response = b""
            while True:
                try:
                    chunk = s.recv(4096)
                    if not chunk:
                        break
                    response += chunk
                except socket.timeout:
                    break
            
            return response.decode(errors='ignore')
    except Exception as e:
        return str(e)



def test_valid_get(server):
    res = send_request("GET /index.html HTTP/1.1\nHost: localhost\n\n")
    assert "HTTP/1.1 200 OK" in res
    assert "Welcome to HTTPd" in res

def test_head_request(server):
    """HEAD doit renvoyer la taille (Content-Length) mais PAS de corps."""
    res = send_request("HEAD /index.html HTTP/1.1\nHost: localhost\n\n")
    assert "HTTP/1.1 200 OK" in res
    assert "Content-Length: 16" in res 
    assert "Welcome to HTTPd" not in res 

def test_directory_index(server):
    res = send_request("GET / HTTP/1.1\nHost: localhost\n\n")
    assert "HTTP/1.1 200 OK" in res
    assert "Welcome to HTTPd" in res


def test_404_not_found(server):
    res = send_request("GET /does_not_exist.html HTTP/1.1\nHost: localhost\n\n")
    assert "HTTP/1.1 404 Not Found" in res

def test_method_not_allowed(server):
    """POST/DELETE/PUT ne sont pas supportés -> 405."""
    res = send_request("POST /index.html HTTP/1.1\nHost: localhost\n\n")
    assert "HTTP/1.1 405 Method Not Allowed" in res

def test_bad_version(server):
    res = send_request("GET /index.html HTTP/1.0\nHost: localhost\n\n")
    assert "505 HTTP Version Not Supported" in res


def test_bad_host_value(server):
    res = send_request("GET /index.html HTTP/1.1\nHost: evil.com\n\n")
    assert "400 Bad Request" in res

def test_missing_host(server):
    res = send_request("GET /index.html HTTP/1.1\n\n")
    assert "400 Bad Request" in res

def test_host_case_insensitive(server):
    res = send_request("GET /index.html HTTP/1.1\nhOsT: localhost\n\n")
    assert "HTTP/1.1 200 OK" in res

def test_host_with_spaces(server):
    res = send_request("GET /index.html HTTP/1.1\nHost:   localhost  \n\n")
    assert "HTTP/1.1 200 OK" in res


def test_path_traversal(server):
    res = send_request("GET /../Makefile HTTP/1.1\nHost: localhost\n\n")
    assert "HTTP/1.1" in res

def test_malformed_request_line(server):
    res = send_request("NOT_A_HTTP_REQUEST\n\n")
    assert "400 Bad Request" in res
