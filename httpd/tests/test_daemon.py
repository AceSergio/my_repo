import os
import time
import signal
import subprocess
import pytest

BIN = "../httpd"
PID_FILE = os.path.abspath("/tmp/httpd_daemon_pytest.pid")
LOG_FILE = os.path.abspath("daemon_test.log")

BASE_CMD = [
    BIN,
    "--server_name", "daemon_test",
    "--port", "8089",
    "--ip", "127.0.0.1",
    "--root_dir", "..",
    "--pid_file", PID_FILE,
    "--log_file", LOG_FILE,
    "--log", "true"
]

def get_pid():
    if not os.path.exists(PID_FILE):
        return None
    try:
        with open(PID_FILE, "r") as f:
            return int(f.read().strip())
    except ValueError:
        return None

def is_running(pid):
    if pid is None: return False
    try:
        os.kill(pid, 0)
        return True
    except OSError:
        return False

@pytest.fixture(autouse=True)
def cleanup_daemon():
    """Nettoie avant et après chaque test."""
    # Setup : on s'assure que rien ne tourne
    pid = get_pid()
    if pid and is_running(pid):
        os.kill(pid, signal.SIGKILL)
    if os.path.exists(PID_FILE):
        os.remove(PID_FILE)
    
    yield # Exécution du test
    
    # Teardown : on nettoie ce qui reste
    pid = get_pid()
    if pid and is_running(pid):
        os.kill(pid, signal.SIGKILL)
    if os.path.exists(PID_FILE):
        os.remove(PID_FILE)
    if os.path.exists(LOG_FILE):
        os.remove(LOG_FILE)

def test_daemon_start():
    cmd = BASE_CMD + ["--daemon", "start"]
    subprocess.check_call(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    time.sleep(0.5)
    
    assert os.path.exists(PID_FILE), "PID file should exist"
    pid = get_pid()
    assert is_running(pid), f"Daemon process {pid} should be running"

def test_daemon_restart():
    # 1. Start initial
    cmd_start = BASE_CMD + ["--daemon", "start"]
    subprocess.check_call(cmd_start, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(0.5)
    pid_1 = get_pid()
    assert is_running(pid_1)

    # 2. Restart
    cmd_restart = BASE_CMD + ["--daemon", "restart"]
    subprocess.check_call(cmd_restart, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(1.5) # Restart délai + fork

    # 3. Verifications
    pid_2 = get_pid()
    assert pid_2 is not None
    assert pid_2 != pid_1, "PID should have changed after restart"
    assert is_running(pid_2), "New daemon should be running"
    assert not is_running(pid_1), "Old daemon should be dead"

def test_daemon_stop():
    # 1. Start
    cmd_start = BASE_CMD + ["--daemon", "start"]
    subprocess.check_call(cmd_start, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(0.5)
    pid = get_pid()
    assert is_running(pid)

    # 2. Stop
    cmd_stop = BASE_CMD + ["--daemon", "stop"]
    subprocess.check_call(cmd_stop, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(0.5)

    # 3. Verifications
    assert not os.path.exists(PID_FILE), "PID file should be removed"
    assert not is_running(pid), "Daemon process should be stopped"