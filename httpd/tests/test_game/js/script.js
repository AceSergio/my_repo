document.addEventListener('DOMContentLoaded', () => {
    const inventoryStatus = document.getElementById('inventory-status');
    function updateInventoryDisplay() {
        const hasKey = localStorage.getItem('hasKey') === 'true';
        if (inventoryStatus) {
            inventoryStatus.textContent = hasKey ? "Service Key [ACQUIRED]" : "Service Key [MISSING]";
            inventoryStatus.style.color = hasKey ? '#2ecc71' : '#e74c3c';
        }
    }

    document.querySelectorAll('.choice-button').forEach(link => {
        link.addEventListener('click', (e) => {
            e.preventDefault();

            const soundFile = link.getAttribute('data-sound');
            const targetUrl = link.href;

            if (soundFile) {
                const audio = new Audio(`assets/sounds/${soundFile}`);

                audio.play().then(() => {
                    setTimeout(() => {
                        window.location.href = targetUrl;
                    }, 100);
                }).catch(error => {
                    console.error("Audio playback blocked, navigating immediately:", error);
                    window.location.href = targetUrl;
                });
            } else {
                window.location.href = targetUrl;
            }
        });
    });

    if (document.body.classList.contains('index-page') || document.location.pathname.endsWith('index.html') || document.location.pathname.endsWith('/')) {
        const inspectButton = document.getElementById('inspect-button');
        const messageArea = document.getElementById('message-area');

        if (inspectButton) {
            inspectButton.addEventListener('click', () => {
                messageArea.innerHTML = "> **LOGS**: User request HTTP/1.1. Status code: **200 OK**. Everything seems stable for now.";
            });
        }
    }

    if (document.location.pathname.includes('server_room.html')) {
        const getKeyButton = document.getElementById('get-key-button');
        const continueLink = document.getElementById('continue-link');
        const messageArea = document.getElementById('message-area');

        if (localStorage.getItem('hasKey') === 'true') {
            getKeyButton.style.display = 'none';
            continueLink.style.display = 'block';
            messageArea.innerHTML = "> **INFO**: The Service Key is already in your inventory. Proceed!";
        }

        if (getKeyButton) {
            getKeyButton.addEventListener('click', () => {
                localStorage.setItem('hasKey', 'true');
                updateInventoryDisplay();
                messageArea.innerHTML = "> **ACQUISITION**: Service Key added to inventory! A new path is now available.";
                getKeyButton.style.display = 'none';
                continueLink.style.display = 'block';
            });
        }
    }

    if (document.location.pathname.includes('firewall.html')) {
        const submitButton = document.getElementById('submit-code-button');
        const passInput = document.getElementById('pass-input');
        const messageArea = document.getElementById('message-area');
        const hasKey = localStorage.getItem('hasKey') === 'true';

        if (!hasKey) {
            passInput.disabled = true;
            submitButton.disabled = true;
            messageArea.innerHTML = "> **ERROR**: Access denied. You must have the Service Key.";
        } else {
            messageArea.innerHTML = "> **READY**: Key detected. Enter the code to proceed.";

            submitButton.addEventListener('click', () => {
                const answer = passInput.value.trim();
                const CORRECT_ANSWER = '404';

                if (answer === CORRECT_ANSWER) {
                    messageArea.innerHTML = "> **ACCESS GRANTED**: Code 404 correct! Redirecting to destination...";
                    setTimeout(() => {
                        window.location.href = 'success.html';
                    }, 1000);
                } else {
                    messageArea.innerHTML = "> **ACCESS DENIED**: Incorrect code. You have been reset!";
                    setTimeout(() => {
                        window.location.href = 'failure.html';
                        localStorage.removeItem('hasKey');
                    }, 1000);
                }
            });
        }
    }

    updateInventoryDisplay();
});