import os
import time
import signal
import subprocess
import sys

BIN = "../httpd"
PID_FILE = os.path.abspath("/tmp/httpd_daemon_test.pid")
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

def check_pid_running(pid):
    """Vérifie si un processus avec ce PID existe."""
    try:
        os.kill(pid, 0)
    except OSError:
        return False
    return True

def get_pid_from_file():
    """Lit le PID depuis le fichier."""
    if not os.path.exists(PID_FILE):
        return None
    try:
        with open(PID_FILE, "r") as f:
            return int(f.read().strip())
    except ValueError:
        return None

def run_daemon_test():
    if os.path.exists(PID_FILE):
        pid = get_pid_from_file()
        if pid and check_pid_running(pid):
            os.kill(pid, signal.SIGKILL)
        if os.path.exists(PID_FILE):
            os.remove(PID_FILE)

    print("\n--- DAEMON INTEGRATION TESTS ---")
    
    if not os.path.exists(BIN):
        print(f"Error: Binary {BIN} not found. Run 'make' in root first.")
        return 1

    print("[TEST] Daemon Start ... ", end="")
    
    cmd_start = BASE_CMD + ["--daemon", "start"]
    try:
        subprocess.check_call(cmd_start, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        print("FAIL (Command returned error)")
        return 1

    time.sleep(0.5)

    if not os.path.exists(PID_FILE):
        print("FAIL (PID file missing)")
        return 1
    
    pid_start = get_pid_from_file()
    if not pid_start or not check_pid_running(pid_start):
        print(f"FAIL (Process {pid_start} not running)")
        return 1
        
    print(f"PASS (PID: {pid_start})")

    print("[TEST] Daemon Restart ... ", end="")
    
    cmd_restart = BASE_CMD + ["--daemon", "restart"]
    try:
        subprocess.check_call(cmd_restart, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        print("FAIL (Command error)")
        return 1
        
    time.sleep(1.5)
    pid_restart = get_pid_from_file()
    
    if not pid_restart:
        print("FAIL (PID file missing after restart)")
        return 1
        
    if pid_restart == pid_start:
        print(f"FAIL (PID did not change: {pid_start})")
        return 1
        
    if not check_pid_running(pid_restart):
        print(f"FAIL (New process {pid_restart} not running)")
        return 1
        
    if check_pid_running(pid_start):
        print(f"FAIL (Old process {pid_start} still alive)")
        return 1
        
    print(f"PASS (Old: {pid_start} -> New: {pid_restart})")

    print("[TEST] Daemon Stop ... ", end="")
    
    cmd_stop = BASE_CMD + ["--daemon", "stop"]
    try:
        subprocess.check_call(cmd_stop, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        print("FAIL (Command error)")
        return 1
        
    time.sleep(0.5)
    
    if os.path.exists(PID_FILE):
        print("FAIL (PID file still exists)")
        return 1
        
    if check_pid_running(pid_restart):
        print(f"FAIL (Process {pid_restart} still running)")
        return 1
        
    print("PASS")
    
    if os.path.exists(LOG_FILE):
        os.remove(LOG_FILE)

    return 0

if __name__ == "__main__":
    sys.exit(run_daemon_test())
