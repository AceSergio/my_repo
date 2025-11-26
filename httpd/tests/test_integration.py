import pytest
import os
import time
import signal
import subprocess

# Chemins dynamiques basés sur l'emplacement du fichier de test
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(BASE_DIR, "../httpd")
PID_FILE = os.path.abspath("/tmp/httpd_daemon_test.pid")
LOG_FILE = os.path.abspath("daemon_test.log")

BASE_CMD = [
    BIN,
    "--server_name", "daemon_test",
    "--port", "8089",
    "--ip", "127.0.0.1",
    "--root_dir", os.path.join(BASE_DIR, ".."), # Root relatif au dossier tests
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

@pytest.fixture(autouse=True)
def cleanup():
    """Fixture qui s'exécute avant et après chaque test pour nettoyer."""
    # Avant le test : on s'assure que rien ne traîne
    if os.path.exists(PID_FILE):
        pid = get_pid_from_file()
        if pid and check_pid_running(pid):
            os.kill(pid, signal.SIGKILL)
        os.remove(PID_FILE)
    
    yield # Le test s'exécute ici
    
    # Après le test : nettoyage final
    if os.path.exists(PID_FILE):
        pid = get_pid_from_file()
        if pid and check_pid_running(pid):
            os.kill(pid, signal.SIGKILL)
        os.remove(PID_FILE)
    if os.path.exists(LOG_FILE):
        os.remove(LOG_FILE)

def test_daemon_lifecycle():
    """Teste le cycle complet : Start -> Restart -> Stop"""
    
    assert os.path.exists(BIN), f"Le binaire {BIN} n'existe pas. Lancez 'make' d'abord."

    # --- ETAPE 1 : DEMARRAGE ---
    cmd_start = BASE_CMD + ["--daemon", "start"]
    subprocess.check_call(cmd_start, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    time.sleep(0.5)
    
    assert os.path.exists(PID_FILE), "Le fichier PID doit exister après le démarrage"
    pid_start = get_pid_from_file()
    assert pid_start is not None, "Le fichier PID ne doit pas être vide"
    assert check_pid_running(pid_start), f"Le processus {pid_start} devrait tourner"

    # --- ETAPE 2 : REDEMARRAGE ---
    cmd_restart = BASE_CMD + ["--daemon", "restart"]
    subprocess.check_call(cmd_restart, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    time.sleep(1.5)
    
    pid_restart = get_pid_from_file()
    assert pid_restart is not None, "Le fichier PID doit exister après restart"
    assert pid_restart != pid_start, f"Le PID aurait dû changer (Ancien: {pid_start}, Nouveau: {pid_restart})"
    assert check_pid_running(pid_restart), "Le nouveau processus devrait tourner"
    assert not check_pid_running(pid_start), "L'ancien processus devrait être mort"

    # --- ETAPE 3 : ARRET ---
    cmd_stop = BASE_CMD + ["--daemon", "stop"]
    subprocess.check_call(cmd_stop, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    time.sleep(0.5)
    
    assert not os.path.exists(PID_FILE), "Le fichier PID ne devrait plus exister après stop"
    assert not check_pid_running(pid_restart), "Le processus devrait être arrêté"
    