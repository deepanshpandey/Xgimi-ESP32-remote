/**
 * XGIMI ESP32 BLE Remote & Pairing Web Application
 */

document.addEventListener('DOMContentLoaded', () => {
  const statusPill = document.getElementById('statusPill');
  const statusText = document.getElementById('statusText');
  const openPairingBtn = document.getElementById('openPairingBtn');
  const closePairingBtn = document.getElementById('closePairingBtn');
  const pairingModal = document.getElementById('pairingModal');

  const modalStatusDot = document.getElementById('modalStatusDot');
  const modalStatusText = document.getElementById('modalStatusText');
  const modalBondedCount = document.getElementById('modalBondedCount');
  const modalPeerAddr = document.getElementById('modalPeerAddr');

  const startPairingBtn = document.getElementById('startPairingBtn');
  const clearBondsBtn = document.getElementById('clearBondsBtn');
  const pinInput = document.getElementById('pinInput');
  const submitPinBtn = document.getElementById('submitPinBtn');

  let socket = null;
  let isConnected = false;

  // Initialize WebSocket or HTTP polling fallback
  function initConnection() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;

    try {
      socket = new WebSocket(wsUrl);

      socket.onopen = () => {
        setConnectedState(true, 'Connected');
        fetchPairingStatus();
      };

      socket.onclose = () => {
        setConnectedState(false, 'Reconnecting...');
        setTimeout(initConnection, 2000);
      };

      socket.onerror = () => {
        setConnectedState(false, 'HTTP Fallback Mode');
      };

      socket.onmessage = (event) => {
        try {
          const msg = JSON.parse(event.data);
          if (msg.type === 'pairing_status') {
            updatePairingUI(msg);
          } else if (msg.status === 'ble_connected') {
            setConnectedState(true, 'BLE Connected');
          } else if (msg.status === 'ble_disconnected') {
            setConnectedState(false, 'BLE Disconnected');
          }
        } catch (e) {
          console.log('WS msg:', event.data);
        }
      };
    } catch (err) {
      setConnectedState(false, 'HTTP Mode');
    }
  }

  function setConnectedState(connected, text) {
    isConnected = connected;
    statusText.textContent = text;
    if (connected) {
      statusPill.classList.remove('disconnected');
      statusPill.classList.add('connected');
    } else {
      statusPill.classList.remove('connected');
      statusPill.classList.add('disconnected');
    }
  }

  function updatePairingUI(status) {
    modalBondedCount.textContent = status.bonded_count ?? 0;
    modalPeerAddr.textContent = status.peer_address ?? 'None';

    if (status.connected) {
      modalStatusText.textContent = 'Paired & Connected to Projector';
      modalStatusDot.style.backgroundColor = '#00e676';
    } else if (status.advertising) {
      modalStatusText.textContent = 'BLE Pairing Mode Active (Discoverable)';
      modalStatusDot.style.backgroundColor = '#00f2fe';
    } else {
      modalStatusText.textContent = 'Not Connected / Idle';
      modalStatusDot.style.backgroundColor = '#ff4b4b';
    }
  }

  function fetchPairingStatus() {
    fetch('/api/pair/status')
      .then(res => res.json())
      .then(data => updatePairingUI(data))
      .catch(err => console.error('Failed fetching pairing status:', err));
  }

  // Send action to ESP32
  function triggerAction(actionName) {
    if ('vibrate' in navigator) {
      navigator.vibrate(25);
    }

    if (socket && socket.readyState === WebSocket.OPEN) {
      socket.send(JSON.stringify({ type: 'key_press', action: actionName }));
    } else {
      fetch(`/api/press?action=${encodeURIComponent(actionName)}`, { method: 'POST' })
        .catch(err => console.error('Failed sending key press:', err));
    }
  }

  // Pairing Modal Event Handlers
  openPairingBtn.addEventListener('click', () => {
    pairingModal.classList.add('open');
    fetchPairingStatus();
  });

  closePairingBtn.addEventListener('click', () => {
    pairingModal.classList.remove('open');
  });

  pairingModal.addEventListener('click', (e) => {
    if (e.target === pairingModal) {
      pairingModal.classList.remove('open');
    }
  });

  startPairingBtn.addEventListener('click', () => {
    fetch('/api/pair/start', { method: 'POST' })
      .then(res => res.json())
      .then(data => {
        alert('BLE Pairing Mode Enabled for 60s! On your XGIMI Projector, go to Settings -> Remotes & Accessories -> Add Accessory and select XGIMI-RC-pseudo.');
        fetchPairingStatus();
      })
      .catch(err => alert('Failed starting BLE pairing mode: ' + err));
  });

  clearBondsBtn.addEventListener('click', () => {
    if (confirm('Are you sure you want to clear all stored Bluetooth bonds?')) {
      fetch('/api/pair/clear', { method: 'POST' })
        .then(res => res.json())
        .then(data => {
          alert('Cleared all saved Bluetooth bonds!');
          fetchPairingStatus();
        })
        .catch(err => alert('Failed clearing bonds: ' + err));
    }
  });

  submitPinBtn.addEventListener('click', () => {
    const pin = pinInput.value.trim();
    if (!pin) {
      alert('Please enter a PIN code.');
      return;
    }
    fetch('/api/pair/pin', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: `pin=${encodeURIComponent(pin)}`
    })
      .then(res => res.json())
      .then(data => {
        alert(`PIN '${pin}' submitted successfully!`);
        pinInput.value = '';
      })
      .catch(err => alert('Failed submitting PIN: ' + err));
  });

  // Attach event listeners to all remote buttons
  const buttons = document.querySelectorAll('[data-action]');
  buttons.forEach(button => {
    const actionName = button.getAttribute('data-action');
    
    button.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      button.classList.add('active');
      triggerAction(actionName);
    });

    button.addEventListener('pointerup', () => {
      button.classList.remove('active');
    });

    button.addEventListener('pointerleave', () => {
      button.classList.remove('active');
    });
  });

  // Physical Keyboard shortcut support
  window.addEventListener('keydown', (e) => {
    if (e.repeat) return;
    let action = null;
    switch (e.key) {
      case 'ArrowUp': action = 'DPAD_UP'; break;
      case 'ArrowDown': action = 'DPAD_DOWN'; break;
      case 'ArrowLeft': action = 'DPAD_LEFT'; break;
      case 'ArrowRight': action = 'DPAD_RIGHT'; break;
      case 'Enter': action = 'DPAD_CENTER'; break;
      case 'Escape': case 'Backspace': action = 'BACK'; break;
      case 'Home': action = 'HOME'; break;
      case 'm': case 'M': action = 'MENU'; break;
      case 'f': case 'F': action = 'FOCUS_AUTO'; break;
      case 'g': case 'G': action = 'FOCUS_MAN_NEW'; break;
      case '[': action = 'FOCUS_LEFT'; break;
      case ']': action = 'FOCUS_RIGHT'; break;
      case 's': case 'S': action = 'XGIMI_MISCKEY'; break;
      case '+': action = 'VOLUME_UP'; break;
      case '-': action = 'VOLUME_DOWN'; break;
    }
    if (action) {
      const btn = document.querySelector(`[data-action="${action}"]`);
      if (btn) {
        btn.classList.add('active');
        setTimeout(() => btn.classList.remove('active'), 200);
      }
      triggerAction(action);
    }
  });

  initConnection();
});
