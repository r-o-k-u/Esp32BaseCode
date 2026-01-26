/**
 * ═══════════════════════════════════════════════════════════════
 * ESP32 CONTROL PANEL - JAVASCRIPT
 * WebSocket communication, UI updates, and user interactions
 * ═══════════════════════════════════════════════════════════════
 */

// Global Variables
let ws = null;
let reconnectInterval = null;
let sensorData = {};
let messageLog = [];
let tempHistory = [];
let humidityHistory = [];

// Configuration
const WS_RECONNECT_DELAY = 3000;
const CHART_MAX_POINTS = 20;
const STATUS_UPDATE_INTERVAL = 5000;

// Initialize on page load
document.addEventListener('DOMContentLoaded', function() {
    console.log('ESP32 Control Panel initializing...');
    initTabs();
    connectWebSocket();
    loadSystemInfo();
    startStatusUpdates();
});

// ═══════════════════════════════════════════════════════════════
// TAB NAVIGATION
// ═══════════════════════════════════════════════════════════════

function initTabs() {
    const tabBtns = document.querySelectorAll('.tab-btn');
    const tabContents = document.querySelectorAll('.tab-content');

    tabBtns.forEach(btn => {
        btn.addEventListener('click', function() {
            const targetTab = this.dataset.tab;

            // Remove active class from all
            tabBtns.forEach(b => b.classList.remove('active'));
            tabContents.forEach(c => c.classList.remove('active'));

            // Add active class to clicked
            this.classList.add('active');
            document.getElementById(targetTab).classList.add('active');

            // Load tab-specific data
            onTabChange(targetTab);
        });
    });
}

function onTabChange(tab) {
    switch(tab) {
        case 'sensors':
            refreshSensors();
            break;
        case 'communication':
            refreshPeers();
            break;
        case 'logs':
            loadLogs();
            break;
    }
}

// ═══════════════════════════════════════════════════════════════
// WEBSOCKET CONNECTION
// ═══════════════════════════════════════════════════════════════

function connectWebSocket() {
    const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${location.host}/ws`;

    console.log('Connecting to WebSocket:', wsUrl);

    ws = new WebSocket(wsUrl);

    ws.onopen = function() {
        console.log('WebSocket connected');
        updateConnectionStatus(true);
        clearInterval(reconnectInterval);

        // Request initial data
        sendWSMessage({type: 'getStatus'});
        sendWSMessage({type: 'getSensorData'});
        sendWSMessage({type: 'getPeers'});
    };

    ws.onclose = function() {
        console.log('WebSocket disconnected');
        updateConnectionStatus(false);

        // Attempt reconnection
        reconnectInterval = setInterval(function() {
            console.log('Reconnecting...');
            connectWebSocket();
        }, WS_RECONNECT_DELAY);
    };

    ws.onerror = function(error) {
        console.error('WebSocket error:', error);
    };

    ws.onmessage = function(event) {
        try {
            const data = JSON.parse(event.data);
            handleWSMessage(data);
        } catch (e) {
            console.error('Error parsing message:', e);
        }
    };
}

function sendWSMessage(message) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(message));
    } else {
        console.error('WebSocket not connected');
        showToast('Not connected to device', 'error');
    }
}

function handleWSMessage(data) {
    console.log('WebSocket message:', data);

    switch(data.type) {
        case 'status':
            updateSystemStatus(data);
            break;
        case 'sensor':
            updateSensorData(data);
            break;
        case 'peers':
            updatePeerList(data.peers);
            break;
        case 'espnowMessage':
            addMessageToLog(data);
            break;
        case 'alert':
            showToast(data.message, 'error');
            break;
    }
}

function updateConnectionStatus(connected) {
    const indicator = document.getElementById('status-indicator');
    const status = document.getElementById('connection-status');

    if (connected) {
        indicator.classList.add('connected');
        indicator.classList.remove('disconnected');
        status.textContent = 'Connected';
    } else {
        indicator.classList.add('disconnected');
        indicator.classList.remove('connected');
        status.textContent = 'Disconnected';
    }
}

// ═══════════════════════════════════════════════════════════════
// SYSTEM STATUS
// ═══════════════════════════════════════════════════════════════

function loadSystemInfo() {
    fetch('/api/status')
        .then(response => response.json())
        .then(data => updateSystemStatus(data))
        .catch(error => console.error('Error loading status:', error));
}

// Enhanced system status update
function updateSystemStatus(data) {
    // Update header
    document.getElementById('device-name').textContent = data.device || 'ESP32';

    // Update header overview
    document.getElementById('wifi-quick-status').textContent = data.wifiConnected ? 'Connected' : 'Disconnected';
    document.getElementById('espnow-quick-status').textContent = (data.espnow?.peers || 0) > 0 ? 'Active' : 'Inactive';
    document.getElementById('sensors-quick-count').textContent = data.sensorCount || '0';

    // Update header stats
    document.getElementById('uptime-display').textContent = 'Uptime: ' + formatUptime(data.uptime || 0);
    document.getElementById('heap-display').textContent = 'Heap: ' + formatBytes(data.freeHeap || 0);
    document.getElementById('cpu-display').textContent = 'CPU: ' + (data.cpuUsage || 0) + '%';

    // Update system info card
    document.getElementById('info-device').textContent = data.device || '-';
    document.getElementById('info-version').textContent = data.version || '-';
    document.getElementById('info-uptime').textContent = formatUptime(data.uptime || 0);
    document.getElementById('info-heap').textContent = formatBytes(data.freeHeap || 0);
    document.getElementById('info-cpu').textContent = (data.cpuUsage || 0) + '%';
    document.getElementById('info-rssi').textContent = (data.wifiRSSI || 0) + ' dBm';
    document.getElementById('info-ip').textContent = data.ip || '-';

    // Update ESP-NOW stats
    if (data.espnow) {
        document.getElementById('espnow-sent').textContent = data.espnow.sent || 0;
        document.getElementById('espnow-received').textContent = data.espnow.received || 0;
        document.getElementById('espnow-failed').textContent = data.espnow.failed || 0;
        document.getElementById('espnow-peers').textContent = data.espnow.peers || 0;

        // Update ESP-NOW status indicator
        const statusDot = document.getElementById('espnow-status-dot');
        const statusText = document.getElementById('espnow-status-text');
        const peers = data.espnow.peers || 0;

        if (peers > 0) {
            statusDot.style.color = '#4CAF50';
            statusText.textContent = `Active (${peers} peer${peers !== 1 ? 's' : ''})`;
        } else {
            statusDot.style.color = '#FF9800';
            statusText.textContent = 'Inactive';
        }
    }

    // Update health indicators
    updateHealthIndicators(data);

    // Update performance metrics
    updatePerformanceMetrics(data);

    // Update environmental summary
    updateEnvironmentalSummary(data);

    // Check for notifications
    checkSystemNotifications(data);

    // Show camera tab if available
    if (data.hasCamera) {
        document.getElementById('camera-tab').style.display = 'flex';
    }
}

function startStatusUpdates() {
    setInterval(loadSystemInfo, STATUS_UPDATE_INTERVAL);
}

// ═══════════════════════════════════════════════════════════════
// SENSOR DATA
// ═══════════════════════════════════════════════════════════════

function updateSensorData(data) {
    sensorData = data;

    // Update quick sensors
    const quickDiv = document.getElementById('quick-sensors');
    let html = '';

    if (data.temperature !== undefined) {
        html += `<div class="sensor-reading">
                    <span class="sensor-label">🌡️ Temperature</span>
                    <span class="sensor-value">${data.temperature.toFixed(1)}°C</span>
                 </div>`;
        tempHistory.push(data.temperature);
        if (tempHistory.length > CHART_MAX_POINTS) tempHistory.shift();
    }

    if (data.humidity !== undefined) {
        html += `<div class="sensor-reading">
                    <span class="sensor-label">💧 Humidity</span>
                    <span class="sensor-value">${data.humidity.toFixed(1)}%</span>
                 </div>`;
        humidityHistory.push(data.humidity);
        if (humidityHistory.length > CHART_MAX_POINTS) humidityHistory.shift();
    }

    if (data.distance !== undefined) {
        html += `<div class="sensor-reading">
                    <span class="sensor-label">📏 Distance</span>
                    <span class="sensor-value">${data.distance} cm</span>
                 </div>`;
    }

    if (data.motion !== undefined) {
        html += `<div class="sensor-reading">
                    <span class="sensor-label">👤 Motion</span>
                    <span class="sensor-value">${data.motion ? 'Detected' : 'None'}</span>
                 </div>`;
    }

    quickDiv.innerHTML = html || '<p class="empty-state">No sensor data</p>';

    // Update all sensors tab
    document.getElementById('all-sensors').innerHTML = html;
}

function refreshSensors() {
    sendWSMessage({type: 'getSensorData'});
}

// ═══════════════════════════════════════════════════════════════
// ACTUATOR CONTROL
// ═══════════════════════════════════════════════════════════════

function toggleActuator(actuator, state) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator, state})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`${actuator} turned ${state ? 'ON' : 'OFF'}`, 'success');
        } else {
            showToast('Command failed', 'error');
        }
    })
    .catch(error => {
        console.error('Error:', error);
        showToast('Communication error', 'error');
    });
}

function toggleRelay(relay, state) {
    toggleActuator(`relay${relay}`, state ? 1 : 0);
}

function updateServo(servo, angle) {
    document.getElementById(`servo${servo}-value`).textContent = angle;

    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: `servo${servo}`, angle: parseInt(angle)})
    })
    .then(response => response.json())
    .catch(error => console.error('Error:', error));
}

function setServo(servo, angle) {
    document.getElementById(`servo${servo}-slider`).value = angle;
    updateServo(servo, angle);
}

function triggerBuzzer(duration) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'buzzer', duration})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('Buzzer activated', 'success');
        }
    })
    .catch(error => console.error('Error:', error));
}

// ═══════════════════════════════════════════════════════════════
// UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════

function formatBytes(bytes) {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1048576) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / 1048576).toFixed(1) + ' MB';
}

function formatUptime(ms) {
    const seconds = Math.floor(ms / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);
    const days = Math.floor(hours / 24);

    if (days > 0) return `${days}d ${hours % 24}h`;
    if (hours > 0) return `${hours}h ${minutes % 60}m`;
    if (minutes > 0) return `${minutes}m`;
    return `${seconds}s`;
}

// Missing functions referenced in HTML
function refreshPeers() {
    sendWSMessage({type: 'getPeers'});
}

function loadLogs() {
    const category = document.getElementById('log-category').value;
    fetch(`/api/logs?category=${category}`)
        .then(response => response.text())
        .then(data => {
            document.getElementById('log-content').textContent = data;
        })
        .catch(error => {
            console.error('Error loading logs:', error);
            document.getElementById('log-content').textContent = 'Error loading logs';
        });
}

function clearLogs() {
    if (confirm('Are you sure you want to clear all logs?')) {
        fetch('/api/logs', {method: 'DELETE'})
            .then(() => {
                document.getElementById('log-content').textContent = 'Logs cleared';
                showToast('Logs cleared', 'success');
            })
            .catch(error => console.error('Error clearing logs:', error));
    }
}

function saveConfig(event) {
    event.preventDefault();

    const config = {
        deviceName: document.getElementById('device-name-input').value,
        sensorInterval: parseInt(document.getElementById('sensor-interval').value),
        enableLogging: document.getElementById('enable-logging').checked,
        enableESPNow: document.getElementById('enable-espnow').checked
    };

    fetch('/api/config', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(config)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('Configuration saved', 'success');
        } else {
            showToast('Failed to save configuration', 'error');
        }
    })
    .catch(error => {
        console.error('Error saving config:', error);
        showToast('Communication error', 'error');
    });
}

function exportData() {
    fetch('/api/export')
        .then(response => response.blob())
        .then(blob => {
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'esp32_data.json';
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(url);
            document.body.removeChild(a);
            showToast('Data exported', 'success');
        })
        .catch(error => {
            console.error('Error exporting data:', error);
            showToast('Export failed', 'error');
        });
}

function checkForUpdates() {
    fetch('/api/check-updates')
        .then(response => response.json())
        .then(data => {
            if (data.updateAvailable) {
                showToast(`Update available: ${data.version}`, 'success');
            } else {
                showToast('No updates available', 'info');
            }
        })
        .catch(error => console.error('Error checking updates:', error));
}

function capturePhoto() {
    fetch('/api/camera/capture')
        .then(response => response.blob())
        .then(blob => {
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'esp32_photo.jpg';
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(url);
            document.body.removeChild(a);
            showToast('Photo captured', 'success');
        })
        .catch(error => {
            console.error('Error capturing photo:', error);
            showToast('Photo capture failed', 'error');
        });
}

function toggleFlash() {
    fetch('/api/camera/flash', {method: 'POST'})
        .then(response => response.json())
        .then(data => {
            showToast(data.message, 'success');
        })
        .catch(error => console.error('Error toggling flash:', error));
}

function refreshCamera() {
    const img = document.getElementById('camera-stream');
    img.src = '/cam?' + new Date().getTime();
}

// Motor control functions
function motorForward() {
    toggleActuator('motor-forward', 1);
}

function motorBackward() {
    toggleActuator('motor-backward', 1);
}

function motorLeft() {
    toggleActuator('motor-left', 1);
}

function motorRight() {
    toggleActuator('motor-right', 1);
}

function motorStop() {
    toggleActuator('motor-stop', 1);
}

function updateMotorSpeed(speed) {
    document.getElementById('motor-speed-value').textContent = speed;
    toggleActuator('motor-speed', parseInt(speed));
}

// LED brightness control
function updateLEDBrightness(brightness) {
    document.getElementById('led-brightness-value').textContent = Math.round((brightness / 255) * 100);
    toggleActuator('led-brightness', parseInt(brightness));
}

// RGB LED control
function setRGBColor(r, g, b) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'rgb', r: r, g: g, b: b})
    })
    .then(response => response.json())
    .catch(error => console.error('Error setting RGB color:', error));
}

function updateRGBColor(hexColor) {
    // Convert hex to RGB
    const r = parseInt(hexColor.substr(1, 2), 16);
    const g = parseInt(hexColor.substr(3, 2), 16);
    const b = parseInt(hexColor.substr(5, 2), 16);
    setRGBColor(r, g, b);
}

function showToast(message, type = 'success') {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.className = `toast ${type} show`;

    setTimeout(() => {
        toast.classList.remove('show');
    }, 3000);
}

function restartDevice() {
    if (confirm('Are you sure you want to restart the device?')) {
        fetch('/api/restart', {method: 'POST'})
            .then(() => {
                showToast('Device restarting...', 'success');
                setTimeout(() => location.reload(), 5000);
            });
    }
}

function resetToFactory() {
    if (confirm('WARNING: This will reset all settings. Continue?')) {
        if (confirm('This action cannot be undone. Are you sure?')) {
            fetch('/api/reset', {method: 'POST'})
                .then(() => showToast('Factory reset initiated', 'success'));
        }
    }
}

// Enhanced dashboard functions
function refreshSystemInfo() {
    loadSystemInfo();
    showToast('System info refreshed', 'success');
}

function refreshAllData() {
    loadSystemInfo();
    refreshSensors();
    refreshPeers();
    showToast('All data refreshed', 'success');
}

function exportCurrentData() {
    const data = {
        timestamp: new Date().toISOString(),
        system: {
            device: document.getElementById('info-device').textContent,
            version: document.getElementById('info-version').textContent,
            uptime: document.getElementById('info-uptime').textContent,
            heap: document.getElementById('info-heap').textContent,
            wifiRSSI: document.getElementById('info-rssi').textContent,
            ip: document.getElementById('info-ip').textContent
        },
        espnow: {
            sent: document.getElementById('espnow-sent').textContent,
            received: document.getElementById('espnow-received').textContent,
            failed: document.getElementById('espnow-failed').textContent,
            peers: document.getElementById('espnow-peers').textContent
        },
        sensors: sensorData
    };

    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `esp32_dashboard_${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.json`;
    document.body.appendChild(a);
    a.click();
    window.URL.revokeObjectURL(url);
    document.body.removeChild(a);
    showToast('Dashboard data exported', 'success');
}

function resetAllActuators() {
    if (confirm('Are you sure you want to reset all actuators to default state?')) {
        fetch('/api/actuators/reset', {method: 'POST'})
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showToast('All actuators reset', 'success');
                } else {
                    showToast('Failed to reset actuators', 'error');
                }
            })
            .catch(error => {
                console.error('Error resetting actuators:', error);
                showToast('Communication error', 'error');
            });
    }
}

function emergencyStop() {
    if (confirm('EMERGENCY STOP: This will immediately stop all actuators. Continue?')) {
        fetch('/api/actuators/emergency-stop', {method: 'POST'})
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showToast('Emergency stop activated', 'warning');
                    addActivityLog('Emergency stop activated');
                } else {
                    showToast('Failed to activate emergency stop', 'error');
                }
            })
            .catch(error => {
                console.error('Error activating emergency stop:', error);
                showToast('Communication error', 'error');
            });
    }
}

function updateHealthIndicators(data) {
    // WiFi health
    const wifiHealth = document.getElementById('health-wifi');
    wifiHealth.style.color = data.wifiConnected ? '#4CAF50' : '#F44336';

    // ESP-NOW health
    const espnowHealth = document.getElementById('health-espnow');
    const espnowPeers = data.espnow?.peers || 0;
    espnowHealth.style.color = espnowPeers > 0 ? '#4CAF50' : '#FF9800';

    // Sensors health
    const sensorsHealth = document.getElementById('health-sensors');
    const sensorCount = data.sensorCount || 0;
    sensorsHealth.style.color = sensorCount > 0 ? '#4CAF50' : '#FF9800';

    // Storage health
    const storageHealth = document.getElementById('health-storage');
    const storagePercent = data.storageUsage || 0;
    if (storagePercent > 90) {
        storageHealth.style.color = '#F44336';
    } else if (storagePercent > 70) {
        storageHealth.style.color = '#FF9800';
    } else {
        storageHealth.style.color = '#4CAF50';
    }

    // Memory health
    const memoryHealth = document.getElementById('health-memory');
    const heapPercent = data.heapUsage || 0;
    if (heapPercent > 90) {
        memoryHealth.style.color = '#F44336';
    } else if (heapPercent > 70) {
        memoryHealth.style.color = '#FF9800';
    } else {
        memoryHealth.style.color = '#4CAF50';
    }

    // CPU health
    const cpuHealth = document.getElementById('health-cpu');
    const cpuUsage = data.cpuUsage || 0;
    if (cpuUsage > 90) {
        cpuHealth.style.color = '#F44336';
    } else if (cpuUsage > 70) {
        cpuHealth.style.color = '#FF9800';
    } else {
        cpuHealth.style.color = '#4CAF50';
    }

    // Overall health
    const healthOverall = document.getElementById('health-overall');
    const issues = [
        !data.wifiConnected,
        espnowPeers === 0,
        sensorCount === 0,
        storagePercent > 80,
        heapPercent > 80,
        cpuUsage > 80
    ].filter(Boolean).length;

    if (issues === 0) {
        healthOverall.textContent = 'Excellent';
        healthOverall.style.color = '#4CAF50';
    } else if (issues <= 2) {
        healthOverall.textContent = 'Good';
        healthOverall.style.color = '#8BC34A';
    } else if (issues <= 4) {
        healthOverall.textContent = 'Fair';
        healthOverall.style.color = '#FF9800';
    } else {
        healthOverall.textContent = 'Poor';
        healthOverall.style.color = '#F44336';
    }
}

function updatePerformanceMetrics(data) {
    document.getElementById('metric-loop-time').textContent = (data.loopTime || 0) + ' μs';
    document.getElementById('metric-free-stack').textContent = formatBytes(data.freeStack || 0);
    document.getElementById('metric-uptime').textContent = formatUptime(data.uptime || 0);
    document.getElementById('metric-restarts').textContent = data.restartCount || 0;
}

function updateEnvironmentalSummary(data) {
    // Update environmental values
    document.getElementById('env-temp').textContent = data.temperature ? data.temperature.toFixed(1) + '°C' : '--°C';
    document.getElementById('env-humidity').textContent = data.humidity ? data.humidity.toFixed(1) + '%' : '--%';
    document.getElementById('env-pressure').textContent = data.pressure ? data.pressure.toFixed(1) + ' hPa' : '-- hPa';
    document.getElementById('env-air-quality').textContent = data.airQuality || '--';

    // Calculate comfort level
    const comfortLevel = document.getElementById('comfort-level');
    if (data.temperature && data.humidity) {
        const temp = data.temperature;
        const humidity = data.humidity;

        if (temp >= 18 && temp <= 24 && humidity >= 40 && humidity <= 60) {
            comfortLevel.textContent = 'Comfortable';
            comfortLevel.style.color = '#4CAF50';
        } else if (temp >= 16 && temp <= 26 && humidity >= 30 && humidity <= 70) {
            comfortLevel.textContent = 'Acceptable';
            comfortLevel.style.color = '#8BC34A';
        } else if (temp >= 14 && temp <= 28 && humidity >= 20 && humidity <= 80) {
            comfortLevel.textContent = 'Uncomfortable';
            comfortLevel.style.color = '#FF9800';
        } else {
            comfortLevel.textContent = 'Harsh';
            comfortLevel.style.color = '#F44336';
        }
    } else {
        comfortLevel.textContent = '---';
        comfortLevel.style.color = '#666';
    }
}

function checkSystemNotifications(data) {
    // Check WiFi connection
    if (!data.wifiConnected) {
        showNotification('wifi', 'WiFi connection lost', 'error');
    } else {
        dismissNotification('wifi');
    }

    // Check high temperature
    if (data.temperature && data.temperature > 35) {
        showNotification('high-temp', `High temperature: ${data.temperature.toFixed(1)}°C`, 'warning');
    } else {
        dismissNotification('high-temp');
    }

    // Check low memory
    if (data.heapUsage && data.heapUsage > 90) {
        showNotification('low-memory', `Low memory: ${data.heapUsage}% used`, 'warning');
    } else {
        dismissNotification('low-memory');
    }

    // Check high CPU usage
    if (data.cpuUsage && data.cpuUsage > 90) {
        showNotification('high-cpu', `High CPU usage: ${data.cpuUsage}%`, 'warning');
    } else {
        dismissNotification('high-cpu');
    }

    // Check system update (placeholder)
    // This would be checked against a remote server
    const lastUpdate = localStorage.getItem('lastUpdateCheck') || 0;
    const now = Date.now();
    if (now - lastUpdate > 24 * 60 * 60 * 1000) { // Check once per day
        // Simulate update check
        if (Math.random() < 0.1) { // 10% chance of update available
            showNotification('update', 'System update available', 'success');
        }
        localStorage.setItem('lastUpdateCheck', now);
    }
}

function showNotification(id, message, type = 'info') {
    const notification = document.getElementById(`notification-${id}`);
    if (notification) {
        notification.style.display = 'flex';
        notification.className = `notification-item ${type}`;
        notification.querySelector('.notification-text').textContent = message;
    }
}

function dismissNotification(id) {
    const notification = document.getElementById(`notification-${id}`);
    if (notification) {
        notification.style.display = 'none';
    }
}

function addActivityLog(activity) {
    const logContainer = document.getElementById('activity-log');
    const activityItem = document.createElement('div');
    activityItem.className = 'activity-item';

    const timeSpan = document.createElement('span');
    timeSpan.className = 'activity-time';
    timeSpan.textContent = new Date().toLocaleTimeString();

    const textSpan = document.createElement('span');
    textSpan.className = 'activity-text';
    textSpan.textContent = activity;

    activityItem.appendChild(timeSpan);
    activityItem.appendChild(textSpan);

    // Keep only last 10 activities
    if (logContainer.children.length >= 10) {
        logContainer.removeChild(logContainer.lastChild);
    }

    logContainer.insertBefore(activityItem, logContainer.firstChild);
}

function clearActivityLog() {
    if (confirm('Are you sure you want to clear the activity log?')) {
        document.getElementById('activity-log').innerHTML = `
            <div class="activity-item">
                <span class="activity-time">Now</span>
                <span class="activity-text">Activity log cleared</span>
            </div>
        `;
        showToast('Activity log cleared', 'success');
    }
}

// Initialize activity log
addActivityLog('System initialized');
addActivityLog('Dashboard loaded');
