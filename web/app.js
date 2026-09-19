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
  const installPwaBtn = document.getElementById('installPwaBtn');

  let deferredPrompt = null;
  let socket = null;
  let isConnected = false;

  // PWA Service Worker Registration & Install Prompt
  if ('serviceWorker' in navigator) {
    window.addEventListener('load', () => {
      navigator.serviceWorker.register('/sw.js')
        .then(reg => console.log('[PWA] ServiceWorker registered:', reg.scope))
        .catch(err => console.log('[PWA] ServiceWorker failed:', err));
    });
  }

  window.addEventListener('beforeinstallprompt', (e) => {
    e.preventDefault();
    deferredPrompt = e;
    if (installPwaBtn) {
      installPwaBtn.style.display = 'flex';
    }
  });

  if (installPwaBtn) {
    installPwaBtn.addEventListener('click', async () => {
      if (deferredPrompt) {
        deferredPrompt.prompt();
        const { outcome } = await deferredPrompt.userChoice;
        console.log('[PWA] User choice:', outcome);
        deferredPrompt = null;
        installPwaBtn.style.display = 'none';
      } else {
        alert('To install on iPhone/iPad: Tap the Share icon (box with up arrow) in Safari and tap "Add to Home Screen".');
      }
    });
  }

  window.addEventListener('appinstalled', () => {
    console.log('[PWA] App successfully installed');
    if (installPwaBtn) {
      installPwaBtn.style.display = 'none';
    }
  });

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

  // Rich Haptic Vibration Engine
  function triggerHaptic(type = 'default') {
    if (!('vibrate' in navigator)) return;
    try {
      if (type === 'heavy' || type === 'power') {
        // Distinct double-pulse for power & dangerous actions
        navigator.vibrate([40, 50, 40]);
      } else if (type === 'select' || type === 'ok') {
        // Firm tactile pulse for OK / Select button
        navigator.vibrate(50);
      } else if (type === 'nav') {
        // Quick, crisp click for D-pad arrows
        navigator.vibrate(28);
      } else if (type === 'success') {
        // Ascending double-affirmation pulse
        navigator.vibrate([30, 40, 40]);
      } else {
        // Standard tactile click for remote buttons
        navigator.vibrate(38);
      }
    } catch (e) {
      // Gracefully ignore if platform restricts vibration
    }
  }

  // Send action to ESP32
  function triggerAction(actionName) {
    if (actionName === 'KPPOWER' || actionName === 'MUTE') {
      triggerHaptic('power');
    } else if (actionName === 'DPAD_CENTER') {
      triggerHaptic('select');
    } else if (actionName.startsWith('DPAD_')) {
      triggerHaptic('nav');
    } else {
      triggerHaptic('default');
    }

    if (socket && socket.readyState === WebSocket.OPEN) {
      socket.send(actionName);
    } else {
      fetch(`/api/press?action=${encodeURIComponent(actionName)}`, { method: 'POST' })
        .catch(err => console.error('Failed sending key press:', err));
    }
  }

  // Pairing Modal Event Handlers
  openPairingBtn.addEventListener('click', () => {
    triggerHaptic('default');
    pairingModal.classList.add('open');
    fetchPairingStatus();
    fetchWifiStatus();
  });

  closePairingBtn.addEventListener('click', () => {
    triggerHaptic('nav');
    pairingModal.classList.remove('open');
  });

  pairingModal.addEventListener('click', (e) => {
    if (e.target === pairingModal) {
      triggerHaptic('nav');
      pairingModal.classList.remove('open');
    }
  });

  startPairingBtn.addEventListener('click', () => {
    triggerHaptic('heavy');
    fetch('/api/pair/start', { method: 'POST' })
      .then(res => res.json())
      .then(data => {
        triggerHaptic('success');
        alert('BLE Pairing Mode Enabled for 60s! On your XGIMI Projector, go to Settings -> Remotes & Accessories -> Add Accessory and select XGIMI RC pseudo.');
        fetchPairingStatus();
      })
      .catch(err => alert('Failed starting BLE pairing mode: ' + err));
  });

  clearBondsBtn.addEventListener('click', () => {
    triggerHaptic('heavy');
    if (confirm('Are you sure you want to clear all stored Bluetooth bonds?')) {
      fetch('/api/pair/clear', { method: 'POST' })
        .then(res => res.json())
        .then(data => {
          triggerHaptic('success');
          alert('Cleared all saved Bluetooth bonds!');
          fetchPairingStatus();
        })
        .catch(err => alert('Failed clearing bonds: ' + err));
    }
  });

  submitPinBtn.addEventListener('click', () => {
    triggerHaptic('default');
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

  const wifiStatusText = document.getElementById('wifiStatusText');
  const wifiSsidInput = document.getElementById('wifiSsidInput');
  const wifiPassInput = document.getElementById('wifiPassInput');
  const saveWifiBtn = document.getElementById('saveWifiBtn');

  function fetchWifiStatus() {
    if (!wifiStatusText) return;
    fetch('/api/wifi/status')
      .then(res => res.json())
      .then(data => {
        if (data.connected) {
          wifiStatusText.textContent = `Connected to ${data.ssid} (${data.ip})`;
          wifiStatusText.style.color = '#00e676';
        } else {
          wifiStatusText.textContent = 'Not connected to Home Wi-Fi';
          wifiStatusText.style.color = '#ff4b4b';
        }
      })
      .catch(err => console.error('Failed fetching Wi-Fi status:', err));
  }

  if (saveWifiBtn) {
    saveWifiBtn.addEventListener('click', () => {
      triggerHaptic('default');
      const ssid = wifiSsidInput.value.trim();
      const pass = wifiPassInput.value.trim();
      if (!ssid) {
        alert('Please enter a Wi-Fi SSID.');
        return;
      }
      saveWifiBtn.textContent = 'Connecting...';
      saveWifiBtn.disabled = true;

      fetch('/api/wifi/save', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`
      })
        .then(res => res.json())
        .then(data => {
          saveWifiBtn.textContent = 'Connect & Save';
          saveWifiBtn.disabled = false;
          if (data.connected) {
            triggerHaptic('success');
            alert(`Success! Connected to '${ssid}'.\nHome IP: http://${data.ip}\nmDNS URL: http://xgimi-remote.local`);
            fetchWifiStatus();
          } else {
            alert(`Failed connecting to '${ssid}'. Please check credentials.`);
            fetchWifiStatus();
          }
        })
        .catch(err => {
          saveWifiBtn.textContent = 'Connect & Save';
          saveWifiBtn.disabled = false;
          alert('Network request error: ' + err);
        });
    });
  }

  // Attach event listeners to all remote buttons
  const buttons = document.querySelectorAll('[data-action]');
  buttons.forEach(button => {
    const actionName = button.getAttribute('data-action');
    
    button.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      button.classList.add('active');
      triggerAction(actionName);
    });

    const releaseButton = () => {
      button.classList.remove('active');
      button.blur();
    };

    button.addEventListener('pointerup', releaseButton);
    button.addEventListener('pointerleave', releaseButton);
    button.addEventListener('pointercancel', releaseButton);
    button.addEventListener('touchend', releaseButton);
    button.addEventListener('click', () => {
      button.classList.remove('active');
      button.blur();
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
