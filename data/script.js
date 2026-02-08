/**
 * ═══════════════════════════════════════════════════════════════
 * ESP32 CONTROL PANEL - ULTIMATE COMPLETE JAVASCRIPT v4.0
 * ═══════════════════════════════════════════════════════════════
 * 
 * ✅ ALL ORIGINAL FEATURES PRESERVED
 * ✅ WiFi Manager Integration (scan, connect, AP mode)
 * ✅ OTA Updates with Progress Tracking
 * ✅ Enhanced Animations & Micro-interactions
 * ✅ Real-time WebSocket Communication
 * ✅ Complete Sensor Monitoring
 * ✅ Full Actuator Control
 * ✅ ESP-NOW Communication
 * ✅ Camera Support
 * ✅ Data Logging & Export
 * ✅ Activity Tracking
 * ✅ System Health Monitoring
 * ✅ Dark Mode Support
 * ✅ Keyboard Shortcuts
 */

// ═══════════════════════════════════════════════════════════════
// GLOBAL VARIABLES
// ═══════════════════════════════════════════════════════════════
let ws = null;
let reconnectInterval = null;
let statusUpdateInterval = null;
let sensorData = {};
let messageLog = [];
let tempHistory = [];
let humidityHistory = [];
let tempChart = null;
let humidityChart = null;
let activityLog = [];
let wifiNetworks = [];
let otaProgress = 0;
let otaUpdating = false;

// Configuration
const WS_RECONNECT_DELAY = 3000;
const CHART_MAX_POINTS = 20;
const STATUS_UPDATE_INTERVAL = 5000;
const ANIMATION_DURATION = 300;

// ═══════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════
document.addEventListener('DOMContentLoaded', function() {
    console.log('🚀 ESP32 Control Panel v4.0 ULTIMATE initializing...');
    
    // Initialize all components
    initTabs();
    initializeDashboard();
    connectWebSocket();
    startStatusUpdates();
    initializeCharts();
    initializeAnimations();
    
    // Update clock
    updateClock();
    setInterval(updateClock, 1000);
    
    // Initialize dark mode
    if (localStorage.getItem('darkMode') === 'true') {
        document.body.classList.add('dark-mode');
    }
    
    console.log('✅ Dashboard initialized with WiFi Manager & OTA support');
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
            initializeSensorCharts();
            break;
        case 'communication':
            refreshPeers();
            updatePeerSelect();
            break;
        case 'logs':
            loadLogs();
            break;
        case 'camera':
            initializeCamera();
            break;
        case 'wifi':
            loadWiFiStatus();
            break;
        case 'ota':
            loadOTAStatus();
            break;
        case 'dashboard':
            refreshAllData();
            break;
    }
}

// ═══════════════════════════════════════════════════════════════
// DASHBOARD INITIALIZATION
// ═══════════════════════════════════════════════════════════════
function initializeDashboard() {
    // Add entry animations to cards
    animateCardsOnLoad();
    
    // Initialize tooltips
    initializeTooltips();
    
    // Initialize keyboard shortcuts
    initializeKeyboardShortcuts();
    
    // Initialize event listeners
    initializeEventListeners();
    
    // Add activity log entry
    addActivityLog('Dashboard loaded');
}

function animateCardsOnLoad() {
    const cards = document.querySelectorAll('.stat-card, .card, .sensor-reading, .peer-item');
    cards.forEach((card, index) => {
        card.style.opacity = '0';
        card.style.transform = 'translateY(20px)';
        
        setTimeout(() => {
            card.style.transition = `opacity ${ANIMATION_DURATION}ms ease, transform ${ANIMATION_DURATION}ms ease`;
            card.style.opacity = '1';
            card.style.transform = 'translateY(0)';
        }, index * 50);
    });
}

// ═══════════════════════════════════════════════════════════════
// WEBSOCKET CONNECTION
// ═══════════════════════════════════════════════════════════════
function connectWebSocket() {
    const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${location.host}/ws`;

    console.log('🔌 Connecting to WebSocket:', wsUrl);

    ws = new WebSocket(wsUrl);

    ws.onopen = function() {
        console.log('✅ WebSocket connected');
        updateConnectionStatus(true);
        clearInterval(reconnectInterval);

        // Request initial data
        sendWSMessage({type: 'getStatus'});
        sendWSMessage({type: 'getSensorData'});
        sendWSMessage({type: 'getPeers'});
        sendWSMessage({type: 'getActuatorStatus'});
        
        // Show success notification
        showToast('Connected to ESP32', 'success');
        addActivityLog('WebSocket connected');
    };

    ws.onclose = function() {
        console.log('❌ WebSocket disconnected');
        updateConnectionStatus(false);
        addActivityLog('WebSocket disconnected');

        // Attempt reconnection
        reconnectInterval = setInterval(() => {
            console.log('🔄 Attempting to reconnect...');
            connectWebSocket();
        }, WS_RECONNECT_DELAY);
    };

    ws.onerror = function(error) {
        console.error('⚠️ WebSocket error:', error);
        showToast('Connection error', 'error');
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
        console.log('📤 Sent:', message);
    } else {
        console.error('WebSocket not connected');
        showToast('Not connected to device', 'error');
    }
}

function handleWSMessage(data) {
    console.log('📨 WebSocket message:', data);

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
        case 'actuatorStatus':
            updateActuatorStatus(data);
            break;
        case 'alert':
            showToast(data.message, 'error');
            addActivityLog(`Alert: ${data.message}`);
            break;
        case 'connected':
            showToast(data.message, 'success');
            break;
        case 'log':
            addToLogViewer(data);
            break;
        case 'wifiNetworks':
            displayWiFiNetworks(data.networks);
            break;
        case 'wifiConnecting':
            showToast(`Connecting to ${data.ssid}...`, 'info');
            break;
        case 'otaProgress':
            updateOTAProgress(data.progress);
            break;
    }
}

function updateConnectionStatus(connected) {
    const indicator = document.getElementById('status-indicator');
    const status = document.getElementById('connection-status');
    
    if (!indicator || !status) return;

    if (connected) {
        indicator.classList.add('connected');
        indicator.classList.remove('disconnected');
        indicator.style.color = '#4CAF50';
        indicator.style.boxShadow = '0 0 10px #4CAF50';
        status.textContent = 'Connected';
        
        // Add animation
        indicator.style.animation = 'pulse 2s infinite';
    } else {
        indicator.classList.add('disconnected');
        indicator.classList.remove('connected');
        indicator.style.color = '#f44336';
        indicator.style.boxShadow = 'none';
        indicator.style.animation = 'none';
        status.textContent = 'Disconnected';
    }
}

// ═══════════════════════════════════════════════════════════════
// SYSTEM STATUS
// ═══════════════════════════════════════════════════════════════
function loadSystemInfo() {
    fetch('/api/status')
        .then(response => response.json())
        .then(data => {
            updateSystemStatus(data);
            addActivityLog('System status updated');
        })
        .catch(error => {
            console.error('Error loading status:', error);
            showToast('Failed to load system status', 'error');
        });
}

function updateSystemStatus(data) {
    // Update header
    updateElementText('device-name', data.device || 'ESP32');

    // Update header overview
    updateElementText('wifi-quick-status', data.wifiConnected ? 'Connected' : 'Disconnected');
    updateElementText('espnow-quick-status', (data.espnow?.peers || 0) > 0 ? 'Active' : 'Inactive');
    updateElementText('sensors-quick-count', data.sensorCount || '0');

    // Update header stats
    updateElementText('uptime-display', 'Uptime: ' + formatUptime(data.uptime || 0));
    updateElementText('heap-display', 'Heap: ' + formatBytes(data.freeHeap || 0));
    updateElementText('cpu-display', 'CPU: ' + (data.cpuUsage || 0) + '%');

    // Update system info card
    updateElementText('info-device', data.device || '-');
    updateElementText('info-version', data.version || '-');
    updateElementText('info-uptime', formatUptime(data.uptime || 0));
    updateElementText('info-heap', formatBytes(data.freeHeap || 0));
    updateElementText('info-cpu', (data.cpuUsage || 0) + '%');
    updateElementText('info-rssi', (data.wifiRSSI || 0) + ' dBm');
    updateElementText('info-ip', data.ip || '-');

    // Update WiFi status (NEW)
    if (data.wifi) {
        updateElementText('wifi-status-text', data.wifi.connected ? 'Connected' : 'Disconnected');
        updateElementText('wifi-current-ssid', data.wifi.ssid || 'Not connected');
        updateElementText('wifi-current-ip', data.wifi.ip || '-');
        updateElementText('wifi-current-gateway', data.wifi.gateway || '-');
        updateElementText('wifi-current-subnet', data.wifi.subnet || '-');
        updateElementText('wifi-current-dns', data.wifi.dns || '-');
        updateElementText('wifi-current-mac', data.mac || '-');
        updateElementText('wifi-current-rssi', (data.wifi.rssi || 0) + ' dBm');
        updateElementText('wifi-ap-status', data.wifi.apMode ? 'Active' : 'Inactive');
        
        if (data.wifi.apMode) {
            updateElementText('wifi-ap-ssid', data.wifi.apSSID || '-');
            updateElementText('wifi-ap-ip', data.wifi.apIP || '-');
            updateElementText('wifi-ap-clients', data.wifi.apClients || 0);
        }
        
        const wifiStatusIndicator = document.getElementById('wifi-connection-status');
        if (wifiStatusIndicator) {
            wifiStatusIndicator.textContent = data.wifi.connected ? 'Connected' : 'Disconnected';
            wifiStatusIndicator.style.color = data.wifi.connected ? '#4CAF50' : '#f44336';
        }
    }

    // Update OTA status (NEW)
    if (data.ota) {
        updateElementText('ota-status-text', data.ota.updating ? 'Updating...' : 'Ready');
        updateElementText('ota-hostname', data.ota.hostname || '-');
        updateElementText('ota-port', data.ota.port || '-');
        updateElementText('ota-total-updates', data.ota.totalUpdates || 0);
        updateElementText('ota-failed-updates', data.ota.failedUpdates || 0);
        updateElementText('ota-last-update', data.ota.lastUpdate || 'Never');
        
        const otaStatusIndicator = document.getElementById('ota-ready-status');
        if (otaStatusIndicator) {
            otaStatusIndicator.textContent = data.ota.initialized ? 'Ready' : 'Not Initialized';
            otaStatusIndicator.style.color = data.ota.initialized ? '#4CAF50' : '#FF9800';
        }
        
        if (data.ota.updating) {
            otaUpdating = true;
            updateOTAProgress(data.ota.progress || 0);
        } else {
            if (otaUpdating) {
                otaUpdating = false;
                updateOTAProgress(0);
            }
        }
    }

    // Update ESP-NOW stats
    if (data.espnow) {
        updateElementText('espnow-sent', data.espnow.sent || 0);
        updateElementText('espnow-received', data.espnow.received || 0);
        updateElementText('espnow-failed', data.espnow.failed || 0);
        updateElementText('espnow-peers', data.espnow.peers || 0);

        // Update ESP-NOW status indicator
        const statusDot = document.getElementById('espnow-status-dot');
        const statusText = document.getElementById('espnow-status-text');
        const peers = data.espnow.peers || 0;

        if (statusDot && statusText) {
            if (peers > 0) {
                statusDot.style.color = '#4CAF50';
                statusText.textContent = `Active (${peers} peer${peers !== 1 ? 's' : ''})`;
            } else {
                statusDot.style.color = '#FF9800';
                statusText.textContent = 'Inactive';
            }
        }
    }

    // Update sidebar info
    updateElementText('sidebar-device', data.device || 'ESP32');
    updateElementText('sidebar-uptime', formatUptime(data.uptime || 0));
    updateElementText('sidebar-memory', formatBytes(data.freeHeap || 0));
    updateElementText('sidebar-ip', data.ip || '---');
    
    // Update footer
    updateElementText('footer-memory', formatBytes(data.freeHeap || 0));
    updateElementText('footer-uptime', formatUptime(data.uptime || 0));
    
    // Update stats cards with animation
    updateStatCard('stat-health', calculateHealth(data));
    
    // Update health indicators
    updateHealthIndicators(data);

    // Update performance metrics
    updatePerformanceMetrics(data);

    // Update environmental summary
    updateEnvironmentalSummary(data);

    // Update network info
    updateNetworkInfo(data);

    // Check for notifications
    checkSystemNotifications(data);

    // Show camera tab if available
    if (data.hasCamera) {
        const cameraTab = document.getElementById('camera-tab');
        if (cameraTab) cameraTab.style.display = 'flex';
    }
}

function startStatusUpdates() {
    if (statusUpdateInterval) clearInterval(statusUpdateInterval);
    
    statusUpdateInterval = setInterval(() => {
        if (ws && ws.readyState === WebSocket.OPEN) {
            sendWSMessage({type: 'getStatus'});
            sendWSMessage({type: 'getSensorData'});
        } else {
            loadSystemInfo();
        }
    }, STATUS_UPDATE_INTERVAL);
}

// ═══════════════════════════════════════════════════════════════
// WIFI MANAGER FUNCTIONS (NEW)
// ═══════════════════════════════════════════════════════════════

function loadWiFiStatus() {
    fetch('/api/wifi/status')
        .then(response => response.json())
        .then(data => {
            updateElementText('wifi-current-ssid', data.ssid || 'Not connected');
            updateElementText('wifi-current-ip', data.ip || '-');
            updateElementText('wifi-current-rssi', (data.rssi || 0) + ' dBm');
            updateElementText('wifi-current-mac', data.mac || '-');
            updateElementText('wifi-current-gateway', data.gateway || '-');
            updateElementText('wifi-current-subnet', data.subnet || '-');
            updateElementText('wifi-current-dns', data.dns || '-');
            
            const statusIndicator = document.getElementById('wifi-connection-status');
            if (statusIndicator) {
                statusIndicator.textContent = data.connected ? 'Connected' : 'Disconnected';
                statusIndicator.style.color = data.connected ? '#4CAF50' : '#f44336';
            }
            
            addActivityLog('WiFi status updated');
        })
        .catch(error => {
            console.error('Error loading WiFi status:', error);
        });
}

function scanWiFiNetworks() {
    showToast('Scanning for WiFi networks...', 'info');
    addActivityLog('Scanning WiFi networks');
    
    // Show loading indicator
    const networkList = document.getElementById('wifi-network-list');
    if (networkList) {
        networkList.innerHTML = '<p style="text-align: center; padding: 20px;">🔍 Scanning...</p>';
    }
    
    fetch('/api/wifi/scan')
        .then(response => response.json())
        .then(data => {
            wifiNetworks = data.networks || [];
            displayWiFiNetworks(wifiNetworks);
            showToast(`Found ${data.count} networks`, 'success');
            addActivityLog(`Found ${data.count} WiFi networks`);
        })
        .catch(error => {
            console.error('Error scanning WiFi:', error);
            showToast('Failed to scan networks', 'error');
            if (networkList) {
                networkList.innerHTML = '<p style="text-align: center; padding: 20px; color: #f44336;">❌ Failed to scan networks</p>';
            }
        });
}

function displayWiFiNetworks(networks) {
    const networkList = document.getElementById('wifi-network-list');
    if (!networkList) return;
    
    if (!networks || networks.length === 0) {
        networkList.innerHTML = '<p style="text-align: center; padding: 20px;">No networks found</p>';
        return;
    }
    
    let html = '';
    networks.sort((a, b) => b.rssi - a.rssi); // Sort by signal strength
    
    networks.forEach((network, index) => {
        const signalStrength = getSignalStrength(network.rssi);
        const secureIcon = network.encryption === 'Open' ? '🔓' : '🔒';
        const barCount = Math.ceil((network.rssi + 100) / 12.5); // 0-4 bars
        const bars = '📶'.repeat(Math.max(1, Math.min(4, barCount)));
        
        html += `
            <div class="wifi-network-item" style="
                padding: 15px;
                margin: 10px 0;
                background: white;
                border-radius: 8px;
                cursor: pointer;
                transition: all 0.3s ease;
                border: 2px solid transparent;
                opacity: 0;
                animation: fadeInUp 0.3s ease forwards;
                animation-delay: ${index * 30}ms;
            " onclick="selectWiFiNetwork('${network.ssid.replace(/'/g, "\\'")}', '${network.encryption}')">
                <div style="display: flex; justify-content: space-between; align-items: center;">
                    <div style="flex: 1;">
                        <strong style="font-size: 1.1rem;">${network.ssid}</strong>
                        <div style="font-size: 0.85rem; color: #666; margin-top: 4px;">
                            ${secureIcon} ${network.encryption} • Channel ${network.channel}
                        </div>
                    </div>
                    <div style="text-align: right;">
                        <div style="font-size: 1.5rem;">${bars}</div>
                        <div style="font-size: 0.8rem; color: #666; margin-top: 4px;">
                            ${network.rssi} dBm
                        </div>
                    </div>
                </div>
            </div>
        `;
    });
    
    networkList.innerHTML = html;
}

function getSignalStrength(rssi) {
    if (rssi >= -50) return { icon: '📶📶📶📶', text: 'Excellent', color: '#4CAF50' };
    if (rssi >= -60) return { icon: '📶📶📶', text: 'Good', color: '#8BC34A' };
    if (rssi >= -70) return { icon: '📶📶', text: 'Fair', color: '#FF9800' };
    return { icon: '📶', text: 'Weak', color: '#F44336' };
}

function selectWiFiNetwork(ssid, encryption) {
    const ssidInput = document.getElementById('wifi-ssid-input');
    const passwordInput = document.getElementById('wifi-password-input');
    const passwordGroup = document.getElementById('wifi-password-group');
    
    if (ssidInput) ssidInput.value = ssid;
    
    if (passwordGroup) {
        passwordGroup.style.display = encryption === 'Open' ? 'none' : 'block';
    }
    
    if (passwordInput && encryption === 'Open') {
        passwordInput.value = '';
    }
    
    // Scroll to connect form
    const connectForm = document.getElementById('wifi-connect-form');
    if (connectForm) {
        connectForm.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
    }
    
    showToast(`Selected: ${ssid}`, 'info');
}

function connectToWiFi() {
    const ssid = document.getElementById('wifi-ssid-input')?.value;
    const password = document.getElementById('wifi-password-input')?.value;
    
    if (!ssid) {
        showToast('Please enter SSID', 'warning');
        return;
    }
    
    showToast(`Connecting to ${ssid}...`, 'info');
    addActivityLog(`Connecting to WiFi: ${ssid}`);
    
    fetch('/api/wifi/connect', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({ ssid, password })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`✅ Connected to ${ssid}`, 'success');
            addActivityLog(`Connected to WiFi: ${ssid} (${data.ip})`);
            loadWiFiStatus();
            
            // Clear form
            const ssidInput = document.getElementById('wifi-ssid-input');
            const passwordInput = document.getElementById('wifi-password-input');
            if (ssidInput) ssidInput.value = '';
            if (passwordInput) passwordInput.value = '';
        } else {
            showToast(`❌ Failed: ${data.error}`, 'error');
            addActivityLog(`WiFi connection failed: ${data.error}`);
        }
    })
    .catch(error => {
        console.error('Error connecting to WiFi:', error);
        showToast('Connection error', 'error');
    });
}

function disconnectWiFi() {
    if (!confirm('Disconnect from current WiFi network?')) return;
    
    fetch('/api/wifi/disconnect', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('WiFi disconnected', 'success');
                addActivityLog('WiFi disconnected');
                loadWiFiStatus();
            }
        })
        .catch(error => {
            console.error('Error disconnecting WiFi:', error);
            showToast('Failed to disconnect', 'error');
        });
}

function startAccessPoint() {
    const apSSID = document.getElementById('ap-ssid-input')?.value || 'ESP32-AP';
    const apPassword = document.getElementById('ap-password-input')?.value || '';
    
    if (apPassword && apPassword.length < 8) {
        showToast('AP password must be at least 8 characters', 'warning');
        return;
    }
    
    showToast('Starting Access Point...', 'info');
    addActivityLog(`Starting AP: ${apSSID}`);
    
    fetch('/api/wifi/ap/start', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({ ssid: apSSID, password: apPassword })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`✅ AP started: ${data.ssid}`, 'success');
            addActivityLog(`Access Point started: ${data.ssid} (${data.ip})`);
            updateElementText('wifi-ap-status', 'Active');
            updateElementText('wifi-ap-ssid', data.ssid);
            updateElementText('wifi-ap-ip', data.ip);
        }
    })
    .catch(error => {
        console.error('Error starting AP:', error);
        showToast('Failed to start AP', 'error');
    });
}

function stopAccessPoint() {
    if (!confirm('Stop Access Point?')) return;
    
    fetch('/api/wifi/ap/stop', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Access Point stopped', 'success');
                addActivityLog('Access Point stopped');
                updateElementText('wifi-ap-status', 'Inactive');
                updateElementText('wifi-ap-ssid', '-');
                updateElementText('wifi-ap-ip', '-');
            }
        })
        .catch(error => {
            console.error('Error stopping AP:', error);
        });
}

// ═══════════════════════════════════════════════════════════════
// OTA UPDATE FUNCTIONS (NEW)
// ═══════════════════════════════════════════════════════════════

function loadOTAStatus() {
    fetch('/api/ota/status')
        .then(response => response.json())
        .then(data => {
            updateElementText('ota-status-text', data.updating ? 'Updating...' : 'Ready');
            updateElementText('ota-hostname', data.hostname || '-');
            updateElementText('ota-port', data.port || '-');
            updateElementText('ota-total-updates', data.totalUpdates || 0);
            updateElementText('ota-failed-updates', data.failedUpdates || 0);
            updateElementText('ota-last-update', data.lastUpdate || 'Never');
            updateElementText('ota-state', data.state || 'Idle');
            
            const statusIndicator = document.getElementById('ota-ready-status');
            if (statusIndicator) {
                statusIndicator.textContent = data.initialized ? '✅ Ready' : '⚠️ Not Initialized';
                statusIndicator.style.color = data.initialized ? '#4CAF50' : '#FF9800';
            }
            
            if (data.updating) {
                otaUpdating = true;
                updateOTAProgress(data.progress || 0);
            } else {
                otaUpdating = false;
                updateOTAProgress(0);
            }
            
            addActivityLog('OTA status updated');
        })
        .catch(error => {
            console.error('Error loading OTA status:', error);
        });
}

function uploadFirmware() {
    const fileInput = document.getElementById('firmware-file-input');
    if (!fileInput || !fileInput.files[0]) {
        showToast('Please select a firmware file', 'warning');
        return;
    }
    
    const file = fileInput.files[0];
    
    if (!file.name.endsWith('.bin')) {
        showToast('Please select a .bin firmware file', 'warning');
        return;
    }
    
    if (!confirm(`Upload firmware: ${file.name}?\n\n⚠️ Device will restart after update.\n\nFile size: ${formatBytes(file.size)}`)) {
        return;
    }
    
    const formData = new FormData();
    formData.append('firmware', file);
    
    otaUpdating = true;
    updateOTAProgress(0);
    showToast('📤 Uploading firmware...', 'info');
    addActivityLog(`Uploading firmware: ${file.name} (${formatBytes(file.size)})`);
    
    // Show progress container
    const progressContainer = document.getElementById('ota-progress-container');
    if (progressContainer) {
        progressContainer.style.display = 'block';
    }
    
    const xhr = new XMLHttpRequest();
    
    xhr.upload.addEventListener('progress', function(e) {
        if (e.lengthComputable) {
            const percentComplete = Math.round((e.loaded / e.total) * 100);
            updateOTAProgress(percentComplete);
            console.log(`Upload progress: ${percentComplete}%`);
        }
    });
    
    xhr.addEventListener('load', function() {
        if (xhr.status === 200) {
            updateOTAProgress(100);
            showToast('✅ Firmware uploaded successfully', 'success');
            addActivityLog('Firmware update complete');
            
            setTimeout(() => {
                showToast('🔄 Device restarting...', 'info');
                addActivityLog('Device restarting after OTA update');
                
                // Reload page after device restarts
                setTimeout(() => {
                    location.reload();
                }, 10000);
            }, 2000);
        } else {
            otaUpdating = false;
            updateOTAProgress(0);
            showToast('❌ Firmware upload failed', 'error');
            addActivityLog('Firmware upload failed');
        }
    });
    
    xhr.addEventListener('error', function() {
        otaUpdating = false;
        updateOTAProgress(0);
        showToast('❌ Upload error', 'error');
        addActivityLog('Firmware upload error');
    });
    
    xhr.addEventListener('abort', function() {
        otaUpdating = false;
        updateOTAProgress(0);
        showToast('Upload cancelled', 'warning');
    });
    
    xhr.open('POST', '/api/ota/update');
    xhr.send(formData);
}

function updateOTAProgress(progress) {
    otaProgress = progress;
    
    const progressBar = document.getElementById('ota-progress-bar');
    const progressText = document.getElementById('ota-progress-text');
    const progressContainer = document.getElementById('ota-progress-container');
    
    if (progressBar) {
        progressBar.style.width = progress + '%';
        progressBar.style.background = progress < 100 ? 
            `linear-gradient(90deg, #4CAF50 ${progress}%, #e0e0e0 ${progress}%)` : 
            '#4CAF50';
    }
    
    if (progressText) {
        progressText.textContent = progress + '%';
    }
    
    if (progressContainer) {
        progressContainer.style.display = (otaUpdating || progress > 0) ? 'block' : 'none';
    }
    
    // Update status text
    const statusText = document.getElementById('ota-status-text');
    if (statusText) {
        if (progress === 0 && !otaUpdating) {
            statusText.textContent = 'Ready';
        } else if (progress === 100) {
            statusText.textContent = 'Complete - Restarting...';
        } else {
            statusText.textContent = `Updating... ${progress}%`;
        }
    }
    
    if (progress >= 100 && !otaUpdating) {
        setTimeout(() => {
            if (progressContainer) {
                progressContainer.style.display = 'none';
            }
        }, 3000);
    }
}

// ═══════════════════════════════════════════════════════════════
// SENSOR DATA (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function updateSensorData(data) {
    sensorData = data;
    const now = new Date();

    // Update quick sensors
    const quickDiv = document.getElementById('quick-sensors');
    let html = '';

    if (data.temperature !== undefined) {
        html += `<div class="sensor-reading">
                    <span class="sensor-label">🌡️ Temperature</span>
                    <span class="sensor-value">${data.temperature.toFixed(1)}°C</span>
                 </div>`;
        tempHistory.push({
            x: now,
            y: data.temperature
        });
        if (tempHistory.length > CHART_MAX_POINTS) tempHistory.shift();
    }

    if (data.humidity !== undefined) {
        html += `<div class="sensor-reading">
                    <span class="sensor-label">💧 Humidity</span>
                    <span class="sensor-value">${data.humidity.toFixed(1)}%</span>
                 </div>`;
        humidityHistory.push({
            x: now,
            y: data.humidity
        });
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

    if (data.pressure !== undefined) {
        html += `<div class="sensor-reading">
                    <span class="sensor-label">🏔️ Pressure</span>
                    <span class="sensor-value">${data.pressure.toFixed(1)} hPa</span>
                 </div>`;
    }

    if (data.light !== undefined) {
        html += `<div class="sensor-reading">
                    <span class="sensor-label">💡 Light</span>
                    <span class="sensor-value">${data.light} lux</span>
                 </div>`;
    }

    if (quickDiv) {
        quickDiv.innerHTML = html || '<p class="empty-state">No sensor data</p>';
    }

    // Update all sensors tab
    const allSensors = document.getElementById('all-sensors');
    if (allSensors) {
        allSensors.innerHTML = html;
    }

    // Update sensor display in dashboard
    const sensorReadings = document.getElementById('sensor-readings');
    if (sensorReadings) {
        sensorReadings.innerHTML = html || '<p style="color: var(--text-muted);">No sensor data available</p>';
    }

    // Update sensor last update time
    updateSensorLastUpdate();

    // Update charts if they exist
    updateCharts();
}

function refreshSensors() {
    sendWSMessage({type: 'getSensorData'});
    showToast('Refreshing sensor data...', 'info');
    addActivityLog('Refreshed sensor data');
}

function updateSensorLastUpdate() {
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    updateElementText('sensor-last-update', timeString);
    updateElementText('sensor-update-time', timeString);
}

// ═══════════════════════════════════════════════════════════════
// CHART FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function initializeCharts() {
    // Only initialize if we're on the sensors tab
    if (document.getElementById('tempChart')) {
        initializeSensorCharts();
    }
}

function initializeSensorCharts() {
    const tempCtx = document.getElementById('tempChart')?.getContext('2d');
    const humidityCtx = document.getElementById('humidityChart')?.getContext('2d');

    if (tempCtx && !tempChart) {
        tempChart = new Chart(tempCtx, {
            type: 'line',
            data: {
                datasets: [{
                    label: 'Temperature (°C)',
                    data: tempHistory,
                    borderColor: '#ff6384',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                animation: {
                    duration: 1000,
                    easing: 'easeOutQuart'
                },
                scales: {
                    x: {
                        type: 'time',
                        time: {
                            unit: 'minute'
                        }
                    },
                    y: {
                        beginAtZero: false,
                        title: {
                            display: true,
                            text: 'Temperature (°C)'
                        }
                    }
                }
            }
        });
    }

    if (humidityCtx && !humidityChart) {
        humidityChart = new Chart(humidityCtx, {
            type: 'line',
            data: {
                datasets: [{
                    label: 'Humidity (%)',
                    data: humidityHistory,
                    borderColor: '#36a2eb',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                animation: {
                    duration: 1000,
                    easing: 'easeOutQuart'
                },
                scales: {
                    x: {
                        type: 'time',
                        time: {
                            unit: 'minute'
                        }
                    },
                    y: {
                        beginAtZero: false,
                        min: 0,
                        max: 100,
                        title: {
                            display: true,
                            text: 'Humidity (%)'
                        }
                    }
                }
            }
        });
    }
}

function updateCharts() {
    if (tempChart) {
        tempChart.data.datasets[0].data = tempHistory;
        tempChart.update('none');
    }
    
    if (humidityChart) {
        humidityChart.data.datasets[0].data = humidityHistory;
        humidityChart.update('none');
    }
}

// ═══════════════════════════════════════════════════════════════
// ACTUATOR CONTROL (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function toggleActuator(actuator, state) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator, value: state})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`${actuator} turned ${state ? 'ON' : 'OFF'}`, 'success');
            addActivityLog(`${actuator} ${state ? 'ON' : 'OFF'}`);
            
            // Animate button
            const button = document.querySelector(`[data-actuator="${actuator}"]`);
            if (button) {
                button.classList.add('active');
                setTimeout(() => button.classList.remove('active'), 300);
            }
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
    updateElementText(`servo${servo}-value`, angle);

    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: `servo${servo}`, value: parseInt(angle)})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            addActivityLog(`Servo ${servo} set to ${angle}°`);
        }
    })
    .catch(error => console.error('Error:', error));
}

function setServo(servo, angle) {
    const slider = document.getElementById(`servo${servo}-slider`);
    if (slider) {
        slider.value = angle;
    }
    updateServo(servo, angle);
}

function triggerBuzzer(duration) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'buzzer', value: duration})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`Buzzer activated for ${duration}ms`, 'success');
            addActivityLog(`Buzzer activated (${duration}ms)`);
        }
    })
    .catch(error => console.error('Error:', error));
}

function updateActuatorStatus(data) {
    // Update actuator status indicators based on received data
    console.log('Actuator status:', data);
}

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
    updateElementText('motor-speed-value', speed);
    toggleActuator('motor-speed', parseInt(speed));
}

function updateLEDBrightness(brightness) {
    updateElementText('led-brightness-value', Math.round((brightness / 255) * 100));
    toggleActuator('led-brightness', parseInt(brightness));
}

function setRGBColor(r, g, b) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'rgb', r: r, g: g, b: b})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`RGB color set to (${r}, ${g}, ${b})`, 'success');
            addActivityLog(`RGB color set to R:${r} G:${g} B:${b}`);
        }
    })
    .catch(error => console.error('Error setting RGB color:', error));
}

function updateRGBColor(hexColor) {
    // Convert hex to RGB
    const r = parseInt(hexColor.substr(1, 2), 16);
    const g = parseInt(hexColor.substr(3, 2), 16);
    const b = parseInt(hexColor.substr(5, 2), 16);
    setRGBColor(r, g, b);
}

// ═══════════════════════════════════════════════════════════════
// COMMUNICATION FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function refreshPeers() {
    sendWSMessage({type: 'getPeers'});
    showToast('Refreshing peer list...', 'info');
    addActivityLog('Refreshed peer list');
}

function updatePeerList(peers) {
    const peerList = document.getElementById('peer-list');
    const peerSelect = document.getElementById('peer-select');
    
    if (!peerList && !peerSelect) return;

    let listHtml = '';
    let selectHtml = '<option value="">Select peer...</option>';

    if (peers && peers.length > 0) {
        peers.forEach((peer, index) => {
            listHtml += `
                <div class="peer-item" style="animation-delay: ${index * 50}ms">
                    <span class="peer-mac">${peer.mac}</span>
                    <span class="peer-rssi">${peer.rssi || '--'} dBm</span>
                    <span class="peer-status ${peer.connected ? 'connected' : 'disconnected'}">
                        ${peer.connected ? 'Connected' : 'Disconnected'}
                    </span>
                </div>
            `;
            
            selectHtml += `<option value="${peer.mac}">${peer.mac}${peer.name ? ' - ' + peer.name : ''}</option>`;
        });
    } else {
        listHtml = '<p class="empty-state">No peers found</p>';
    }

    if (peerList) peerList.innerHTML = listHtml;
    if (peerSelect) peerSelect.innerHTML = selectHtml;
}

function updatePeerSelect() {
    // Refresh peer list to update select dropdown
    refreshPeers();
}

function sendMessageToPeer() {
    const peerSelect = document.getElementById('peer-select');
    const messageInput = document.getElementById('message-input');
    
    if (!peerSelect || !messageInput) return;
    
    const peer = peerSelect.value;
    const message = messageInput.value;
    
    if (!peer) {
        showToast('Please select a peer', 'warning');
        return;
    }
    
    if (!message) {
        showToast('Please enter a message', 'warning');
        return;
    }
    
    try {
        // Try to parse as JSON
        const parsedMessage = JSON.parse(message);
        
        sendWSMessage({
            type: 'sendToPeer',
            peer: peer,
            message: parsedMessage
        });
        
        showToast('Message sent', 'success');
        addActivityLog(`Sent message to ${peer}`);
        messageInput.value = '';
        
    } catch (e) {
        // If not valid JSON, send as text
        sendWSMessage({
            type: 'sendToPeer',
            peer: peer,
            message: message
        });
        
        showToast('Message sent as text', 'success');
        addActivityLog(`Sent text to ${peer}`);
        messageInput.value = '';
    }
}

function addMessageToLog(messageData) {
    const messageLog = document.getElementById('message-log');
    if (!messageLog) return;
    
    const messageItem = document.createElement('div');
    messageItem.className = 'message-item';
    messageItem.style.opacity = '0';
    messageItem.style.transform = 'translateX(-20px)';
    
    const timestamp = new Date().toLocaleTimeString();
    const direction = messageData.direction || 'received';
    const peer = messageData.peer || 'Unknown';
    const message = messageData.message || 'No message content';
    
    messageItem.innerHTML = `
        <span class="message-time">${timestamp}</span>
        <span class="message-direction ${direction}">${direction.toUpperCase()}</span>
        <span class="message-peer">${peer}</span>
        <span class="message-content">${typeof message === 'object' ? JSON.stringify(message) : message}</span>
    `;
    
    // Keep only last 20 messages
    if (messageLog.children.length >= 20) {
        messageLog.removeChild(messageLog.lastChild);
    }
    
    messageLog.insertBefore(messageItem, messageLog.firstChild);
    
    // Animate in
    setTimeout(() => {
        messageItem.style.transition = 'all 0.3s ease';
        messageItem.style.opacity = '1';
        messageItem.style.transform = 'translateX(0)';
    }, 10);
}

function clearMessageLog() {
    const messageLog = document.getElementById('message-log');
    if (messageLog) {
        messageLog.innerHTML = '<p class="empty-state">No messages yet...</p>';
        showToast('Message log cleared', 'success');
        addActivityLog('Cleared message log');
    }
}

// ═══════════════════════════════════════════════════════════════
// CAMERA FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function initializeCamera() {
    const cameraTab = document.getElementById('camera-tab');
    if (cameraTab && cameraTab.style.display !== 'none') {
        refreshCamera();
    }
}

function capturePhoto() {
    fetch('/api/camera/capture')
        .then(response => response.blob())
        .then(blob => {
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `esp32_photo_${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.jpg`;
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(url);
            document.body.removeChild(a);
            showToast('Photo captured', 'success');
            addActivityLog('Captured photo');
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
            addActivityLog(`Flash ${data.state ? 'ON' : 'OFF'}`);
        })
        .catch(error => console.error('Error toggling flash:', error));
}

function refreshCamera() {
    const img = document.getElementById('camera-stream');
    if (img) {
        // Add loading animation
        img.style.opacity = '0.5';
        img.src = '/cam?' + new Date().getTime();
        
        img.onload = function() {
            this.style.opacity = '1';
            this.style.transition = 'opacity 0.3s ease';
        };
        
        showToast('Camera refreshed', 'info');
    }
}

// ═══════════════════════════════════════════════════════════════
// LOG FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function loadLogs() {
    const category = document.getElementById('log-category')?.value || 'events';
    fetch(`/api/logs?category=${category}`)
        .then(response => response.text())
        .then(data => {
            const logContent = document.getElementById('log-content');
            if (logContent) {
                logContent.textContent = data;
            }
            addActivityLog(`Loaded ${category} logs`);
        })
        .catch(error => {
            console.error('Error loading logs:', error);
            const logContent = document.getElementById('log-content');
            if (logContent) {
                logContent.textContent = 'Error loading logs';
            }
            showToast('Failed to load logs', 'error');
        });
}

function clearLogs() {
    if (confirm('Are you sure you want to clear all logs?')) {
        fetch('/api/logs', {method: 'DELETE'})
            .then(() => {
                const logContent = document.getElementById('log-content');
                if (logContent) {
                    logContent.textContent = 'Logs cleared';
                }
                showToast('Logs cleared', 'success');
                addActivityLog('Cleared all logs');
            })
            .catch(error => {
                console.error('Error clearing logs:', error);
                showToast('Failed to clear logs', 'error');
            });
    }
}

function addToLogViewer(logData) {
    const logContent = document.getElementById('log-content');
    if (logContent) {
        const timestamp = new Date().toLocaleTimeString();
        const logEntry = `[${timestamp}] ${logData.message}\n`;
        logContent.textContent = logEntry + logContent.textContent;
        
        // Keep log manageable
        const lines = logContent.textContent.split('\n');
        if (lines.length > 100) {
            logContent.textContent = lines.slice(0, 100).join('\n');
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// CONFIGURATION FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function saveConfig(event) {
    if (event) event.preventDefault();

    const config = {
        deviceName: document.getElementById('device-name-input')?.value || 'ESP32',
        sensorInterval: parseInt(document.getElementById('sensor-interval')?.value || 1000),
        enableLogging: document.getElementById('enable-logging')?.checked || false,
        enableESPNow: document.getElementById('enable-espnow')?.checked || false
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
            addActivityLog('Saved configuration');
            // Update device name in UI
            updateElementText('device-name', config.deviceName);
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
            a.download = `esp32_data_${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.json`;
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(url);
            document.body.removeChild(a);
            showToast('Data exported', 'success');
            addActivityLog('Exported data');
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
                addActivityLog(`Update available: ${data.version}`);
            } else {
                showToast('No updates available', 'info');
            }
        })
        .catch(error => {
            console.error('Error checking updates:', error);
            showToast('Failed to check for updates', 'error');
        });
}

// ═══════════════════════════════════════════════════════════════
// UTILITY FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
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
    if (minutes > 0) return `${minutes}m ${seconds % 60}s`;
    return `${seconds}s`;
}

function updateClock() {
    updateElementText('current-time', new Date().toLocaleTimeString());
}

// ═══════════════════════════════════════════════════════════════
// TOAST NOTIFICATION (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function showToast(message, type = 'success') {
    // Create toast container if it doesn't exist
    let container = document.getElementById('toast-container');
    if (!container) {
        container = document.createElement('div');
        container.id = 'toast-container';
        container.style.cssText = `
            position: fixed;
            bottom: 20px;
            right: 20px;
            z-index: 9999;
            display: flex;
            flex-direction: column;
            gap: 10px;
        `;
        document.body.appendChild(container);
    }
    
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;
    toast.style.cssText = `
        background: ${type === 'success' ? '#4CAF50' : 
                    type === 'error' ? '#F44336' : 
                    type === 'warning' ? '#FF9800' : '#2196F3'};
        color: white;
        padding: 12px 20px;
        border-radius: 8px;
        box-shadow: 0 4px 12px rgba(0,0,0,0.15);
        transform: translateX(100%);
        opacity: 0;
        transition: all 0.3s ease;
        cursor: pointer;
    `;
    
    container.appendChild(toast);
    
    // Animate in
    setTimeout(() => {
        toast.style.transform = 'translateX(0)';
        toast.style.opacity = '1';
    }, 10);
    
    // Remove on click
    toast.addEventListener('click', () => {
        toast.style.transform = 'translateX(100%)';
        toast.style.opacity = '0';
        setTimeout(() => toast.remove(), 300);
    });
    
    // Auto remove after 3 seconds
    setTimeout(() => {
        if (toast.parentNode) {
            toast.style.transform = 'translateX(100%)';
            toast.style.opacity = '0';
            setTimeout(() => toast.remove(), 300);
        }
    }, 3000);
}

// ═══════════════════════════════════════════════════════════════
// SYSTEM CONTROL FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function restartDevice() {
    if (confirm('Are you sure you want to restart the device?')) {
        fetch('/api/restart', {method: 'POST'})
            .then(() => {
                showToast('Device restarting...', 'warning');
                addActivityLog('Device restart initiated');
                setTimeout(() => location.reload(), 5000);
            })
            .catch(error => {
                console.error('Error restarting:', error);
                showToast('Failed to restart device', 'error');
            });
    }
}

function resetToFactory() {
    if (confirm('WARNING: This will reset all settings. Continue?')) {
        if (confirm('This action cannot be undone. Are you sure?')) {
            fetch('/api/reset', {method: 'POST'})
                .then(() => {
                    showToast('Factory reset initiated', 'success');
                    addActivityLog('Factory reset initiated');
                    setTimeout(() => location.reload(), 3000);
                })
                .catch(error => {
                    console.error('Error resetting:', error);
                    showToast('Failed to reset', 'error');
                });
        }
    }
}

function refreshSystemInfo() {
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({type: 'getStatus'});
    } else {
        loadSystemInfo();
    }
    showToast('System info refreshed', 'success');
}

function refreshAllData() {
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({type: 'getStatus'});
        sendWSMessage({type: 'getSensorData'});
        sendWSMessage({type: 'getPeers'});
    } else {
        loadSystemInfo();
    }
    showToast('All data refreshed', 'success');
    addActivityLog('Refreshed all data');
}

function exportCurrentData() {
    const data = {
        timestamp: new Date().toISOString(),
        system: {
            device: document.getElementById('info-device')?.textContent || '-',
            version: document.getElementById('info-version')?.textContent || '-',
            uptime: document.getElementById('info-uptime')?.textContent || '-',
            heap: document.getElementById('info-heap')?.textContent || '-',
            wifiRSSI: document.getElementById('info-rssi')?.textContent || '-',
            ip: document.getElementById('info-ip')?.textContent || '-'
        },
        espnow: {
            sent: document.getElementById('espnow-sent')?.textContent || 0,
            received: document.getElementById('espnow-received')?.textContent || 0,
            failed: document.getElementById('espnow-failed')?.textContent || 0,
            peers: document.getElementById('espnow-peers')?.textContent || 0
        },
        sensors: sensorData,
        activityLog: activityLog.slice(-50) // Last 50 activities
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
    addActivityLog('Exported dashboard data');
}

function resetAllActuators() {
    if (confirm('Are you sure you want to reset all actuators to default state?')) {
        fetch('/api/actuators/reset', {method: 'POST'})
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showToast('All actuators reset', 'success');
                    addActivityLog('Reset all actuators');
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
                    
                    // Add visual feedback
                    document.body.style.backgroundColor = '#ffebee';
                    setTimeout(() => {
                        document.body.style.backgroundColor = '';
                    }, 500);
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

// ═══════════════════════════════════════════════════════════════
// HEALTH & PERFORMANCE FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function calculateHealth(data) {
    let health = 100;
    
    if (!data.wifiConnected) health -= 20;
    if ((data.freeHeap || 0) < 50000) health -= 20;
    if ((data.cpuUsage || 0) > 80) health -= 20;
    if ((data.espnow?.peers || 0) === 0) health -= 10;
    
    return Math.max(0, health) + '%';
}

function updateHealthIndicators(data) {
    // Update individual health indicators
    const indicators = {
        'health-wifi': data.wifiConnected,
        'health-espnow': (data.espnow?.peers || 0) > 0,
        'health-sensors': (data.sensorCount || 0) > 0,
        'health-storage': (data.storageUsage || 0) < 80,
        'health-memory': (data.heapUsage || 0) < 80,
        'health-cpu': (data.cpuUsage || 0) < 80
    };
    
    Object.entries(indicators).forEach(([id, isHealthy]) => {
        const element = document.getElementById(id);
        if (element) {
            element.style.color = isHealthy ? '#4CAF50' : 
                                id === 'health-wifi' ? '#F44336' : '#FF9800';
        }
    });
    
    // Update overall health
    const healthOverall = document.getElementById('health-overall');
    if (healthOverall) {
        const issues = Object.values(indicators).filter(v => !v).length;
        
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
}

function updatePerformanceMetrics(data) {
    updateElementText('metric-loop-time', (data.loopTime || 0) + ' μs');
    updateElementText('metric-free-stack', formatBytes(data.freeStack || 0));
    updateElementText('metric-uptime', formatUptime(data.uptime || 0));
    updateElementText('metric-restarts', data.restartCount || 0);
}

function updateEnvironmentalSummary(data) {
    updateElementText('env-temp', data.temperature ? data.temperature.toFixed(1) + '°C' : '--°C');
    updateElementText('env-humidity', data.humidity ? data.humidity.toFixed(1) + '%' : '--%');
    updateElementText('env-pressure', data.pressure ? data.pressure.toFixed(1) + ' hPa' : '-- hPa');
    updateElementText('env-air-quality', data.airQuality || '--');

    const comfortLevel = document.getElementById('comfort-level');
    if (comfortLevel && data.temperature && data.humidity) {
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
    } else if (comfortLevel) {
        comfortLevel.textContent = '---';
        comfortLevel.style.color = '#666';
    }
}

function updateNetworkInfo(data) {
    updateElementText('net-wifi-status', data.wifiConnected ? 'Connected' : 'Disconnected');
    updateElementText('net-ssid', data.ssid || '--');
    updateElementText('net-signal', data.wifiRSSI ? `${data.wifiRSSI} dBm` : '--');
    updateElementText('net-mac', data.mac || '--');
}

function updateElementText(id, text) {
    const element = document.getElementById(id);
    if (element) {
        // Animate text change if it's different
        if (element.textContent !== text) {
            element.style.transform = 'scale(1.1)';
            setTimeout(() => {
                element.style.transform = 'scale(1)';
            }, 150);
        }
        element.textContent = text;
    }
}

function updateStatCard(elementId, value) {
    const element = document.getElementById(elementId);
    if (element) {
        element.style.transform = 'scale(1.1)';
        element.textContent = value;
        
        setTimeout(() => {
            element.style.transform = 'scale(1)';
        }, 200);
    }
}

// ═══════════════════════════════════════════════════════════════
// NOTIFICATION FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

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
}

function showNotification(id, message, type = 'info') {
    console.log(`Notification [${type}]:`, message);
    // Can be enhanced with a notification UI component
}

function dismissNotification(id) {
    // Placeholder for notification dismissal
}

// ═══════════════════════════════════════════════════════════════
// ACTIVITY LOG FUNCTIONS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function addActivityLog(activity) {
    const logContainer = document.getElementById('activity-log') || document.getElementById('activity-timeline');
    if (!logContainer) return;
    
    const now = new Date();
    const activityItem = document.createElement('div');
    activityItem.className = 'activity-item';
    activityItem.style.opacity = '0';
    activityItem.style.transform = 'translateX(-20px)';
    
    activityItem.innerHTML = `
        <div style="display: flex; align-items: center; gap: 12px; padding: 12px; border-bottom: 1px solid #eee;">
            <div style="width: 8px; height: 8px; background: #4CAF50; border-radius: 50%; box-shadow: 0 0 10px #4CAF50;"></div>
            <span style="font-family: monospace; font-size: 0.75rem; color: #666;">${now.toLocaleTimeString()}</span>
            <span style="flex: 1;">${activity}</span>
        </div>
    `;
    
    // Keep only last 10 activities
    while (logContainer.children.length >= 10) {
        logContainer.removeChild(logContainer.lastChild);
    }
    
    logContainer.insertBefore(activityItem, logContainer.firstChild);
    
    // Animate in
    setTimeout(() => {
        activityItem.style.transition = 'all 0.3s ease';
        activityItem.style.opacity = '1';
        activityItem.style.transform = 'translateX(0)';
    }, 10);
    
    // Store in memory
    activityLog.push({
        time: now.toISOString(),
        activity: activity
    });
    
    if (activityLog.length > 1000) {
        activityLog.shift();
    }
}

function clearActivityLog() {
    if (confirm('Clear activity log?')) {
        const logContainer = document.getElementById('activity-log') || document.getElementById('activity-timeline');
        if (logContainer) {
            logContainer.innerHTML = '';
            addActivityLog('Activity log cleared');
        }
        activityLog = [];
        showToast('Activity log cleared', 'info');
    }
}

// ═══════════════════════════════════════════════════════════════
// ANIMATIONS & UX ENHANCEMENTS (ALL ORIGINAL FUNCTIONS PRESERVED)
// ═══════════════════════════════════════════════════════════════

function initializeAnimations() {
    // Add hover effects to all cards
    const cards = document.querySelectorAll('.card, .stat-card, .sensor-reading, .peer-item');
    cards.forEach(card => {
        card.addEventListener('mouseenter', function() {
            this.style.transform = 'translateY(-4px)';
            this.style.boxShadow = '0 8px 25px rgba(0,0,0,0.15)';
        });
        
        card.addEventListener('mouseleave', function() {
            this.style.transform = 'translateY(0)';
            this.style.boxShadow = '';
        });
    });
    
    // Add click animations to buttons
    const buttons = document.querySelectorAll('button:not(.no-animate)');
    buttons.forEach(button => {
        button.addEventListener('click', function() {
            this.style.transform = 'scale(0.95)';
            setTimeout(() => {
                this.style.transform = '';
            }, 150);
        });
    });
}

function initializeTooltips() {
    const elementsWithTitle = document.querySelectorAll('[title]');
    elementsWithTitle.forEach(element => {
        element.addEventListener('mouseenter', function(e) {
            const tooltip = document.createElement('div');
            tooltip.className = 'tooltip';
            tooltip.textContent = this.getAttribute('title');
            tooltip.style.cssText = `
                position: fixed;
                background: rgba(0, 0, 0, 0.8);
                backdrop-filter: blur(10px);
                border: 1px solid rgba(255, 255, 255, 0.1);
                padding: 6px 12px;
                border-radius: 6px;
                font-size: 0.75rem;
                color: white;
                pointer-events: none;
                z-index: 10000;
                box-shadow: 0 4px 12px rgba(0,0,0,0.2);
                white-space: nowrap;
                transform: translateY(5px);
                opacity: 0;
                transition: all 0.2s ease;
            `;
            document.body.appendChild(tooltip);
            
            const rect = this.getBoundingClientRect();
            tooltip.style.top = (rect.top - tooltip.offsetHeight - 8) + 'px';
            tooltip.style.left = (rect.left + rect.width / 2 - tooltip.offsetWidth / 2) + 'px';
            
            this._tooltip = tooltip;
            
            // Animate in
            setTimeout(() => {
                tooltip.style.transform = 'translateY(0)';
                tooltip.style.opacity = '1';
            }, 10);
        });
        
        element.addEventListener('mouseleave', function() {
            if (this._tooltip) {
                this._tooltip.style.transform = 'translateY(5px)';
                this._tooltip.style.opacity = '0';
                setTimeout(() => {
                    if (this._tooltip) {
                        this._tooltip.remove();
                        delete this._tooltip;
                    }
                }, 200);
            }
        });
    });
}

function initializeKeyboardShortcuts() {
    document.addEventListener('keydown', function(e) {
        // Ctrl/Cmd + R: Refresh
        if ((e.ctrlKey || e.metaKey) && e.key === 'r') {
            e.preventDefault();
            refreshAllData();
        }
        
        // Escape: Close modals
        if (e.key === 'Escape') {
            closeAllModals();
        }
        
        // F1: Show help
        if (e.key === 'F1') {
            e.preventDefault();
            showToast('Shortcuts: Ctrl+R=Refresh, Esc=Close, F1=Help', 'info');
        }
    });
}

function closeAllModals() {
    const modals = document.querySelectorAll('.modal');
    modals.forEach(modal => {
        modal.style.display = 'none';
    });
}

function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.style.opacity = '0';
        modal.style.transform = 'scale(0.95)';
        setTimeout(() => {
            modal.style.display = 'none';
        }, 200);
    }
}

function initializeEventListeners() {
    // LED brightness slider
    const ledBrightness = document.getElementById('led-brightness');
    if (ledBrightness) {
        ledBrightness.addEventListener('input', function() {
            updateLEDBrightness(this.value);
        });
    }

    // Motor speed slider
    const motorSpeed = document.getElementById('motor-speed');
    if (motorSpeed) {
        motorSpeed.addEventListener('input', function() {
            updateMotorSpeed(this.value);
        });
    }

    // RGB color picker
    const rgbColor = document.getElementById('rgb-color');
    if (rgbColor) {
        rgbColor.addEventListener('change', function() {
            updateRGBColor(this.value);
        });
    }

    // Log category selector
    const logCategory = document.getElementById('log-category');
    if (logCategory) {
        logCategory.addEventListener('change', loadLogs);
    }

    // Quick restart button
    const quickRestart = document.getElementById('quick-restart');
    if (quickRestart) {
        quickRestart.addEventListener('click', restartDevice);
    }
    
    // Send message button
    const sendMessageBtn = document.getElementById('send-message-btn');
    if (sendMessageBtn) {
        sendMessageBtn.addEventListener('click', sendMessageToPeer);
    }
    
    // Message input enter key
    const messageInput = document.getElementById('message-input');
    if (messageInput) {
        messageInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                sendMessageToPeer();
            }
        });
    }
    
    // WiFi scan button
    const wifiScanBtn = document.getElementById('wifi-scan-btn');
    if (wifiScanBtn) {
        wifiScanBtn.addEventListener('click', scanWiFiNetworks);
    }
    
    // WiFi connect button
    const wifiConnectBtn = document.getElementById('wifi-connect-btn');
    if (wifiConnectBtn) {
        wifiConnectBtn.addEventListener('click', connectToWiFi);
    }
    
    // WiFi disconnect button
    const wifiDisconnectBtn = document.getElementById('wifi-disconnect-btn');
    if (wifiDisconnectBtn) {
        wifiDisconnectBtn.addEventListener('click', disconnectWiFi);
    }
    
    // Start AP button
    const startAPBtn = document.getElementById('start-ap-btn');
    if (startAPBtn) {
        startAPBtn.addEventListener('click', startAccessPoint);
    }
    
    // Stop AP button
    const stopAPBtn = document.getElementById('stop-ap-btn');
    if (stopAPBtn) {
        stopAPBtn.addEventListener('click', stopAccessPoint);
    }
    
    // Upload firmware button
    const uploadFirmwareBtn = document.getElementById('upload-firmware-btn');
    if (uploadFirmwareBtn) {
        uploadFirmwareBtn.addEventListener('click', uploadFirmware);
    }
}

// ═══════════════════════════════════════════════════════════════
// ADDITIONAL FUNCTIONS
// ═══════════════════════════════════════════════════════════════

function triggerAlert() {
    if (confirm('Trigger alert on all connected devices?')) {
        sendWSMessage({
            type: 'triggerAlert',
            message: 'Alert triggered from dashboard',
            priority: 'high'
        });
        showToast('Alert triggered', 'warning');
        addActivityLog('Triggered system alert');
    }
}

function toggleDarkMode() {
    document.body.classList.toggle('dark-mode');
    const isDark = document.body.classList.contains('dark-mode');
    localStorage.setItem('darkMode', isDark);
    showToast(`${isDark ? 'Dark' : 'Light'} mode activated`, 'info');
}

// ═══════════════════════════════════════════════════════════════
// EXPORT FUNCTIONS TO GLOBAL SCOPE
// ═══════════════════════════════════════════════════════════════

// Core functions
window.connectWebSocket = connectWebSocket;
window.sendWSMessage = sendWSMessage;
window.refreshAllData = refreshAllData;
window.restartDevice = restartDevice;
window.emergencyStop = emergencyStop;

// Sensor functions
window.refreshSensors = refreshSensors;
window.refreshSystemInfo = refreshSystemInfo;

// Actuator functions
window.toggleActuator = toggleActuator;
window.toggleRelay = toggleRelay;
window.setServo = setServo;
window.updateServo = updateServo;
window.triggerBuzzer = triggerBuzzer;
window.updateMotorSpeed = updateMotorSpeed;
window.updateLEDBrightness = updateLEDBrightness;
window.updateRGBColor = updateRGBColor;
window.setRGBColor = setRGBColor;

// Motor control
window.motorForward = motorForward;
window.motorBackward = motorBackward;
window.motorLeft = motorLeft;
window.motorRight = motorRight;
window.motorStop = motorStop;

// Communication functions
window.refreshPeers = refreshPeers;
window.sendMessageToPeer = sendMessageToPeer;
window.clearMessageLog = clearMessageLog;

// Log functions
window.loadLogs = loadLogs;
window.clearLogs = clearLogs;

// Camera functions
window.capturePhoto = capturePhoto;
window.toggleFlash = toggleFlash;
window.refreshCamera = refreshCamera;

// Configuration functions
window.saveConfig = saveConfig;
window.exportData = exportData;
window.checkForUpdates = checkForUpdates;
window.exportCurrentData = exportCurrentData;
window.resetAllActuators = resetAllActuators;
window.resetToFactory = resetToFactory;

// WiFi Manager functions (NEW)
window.loadWiFiStatus = loadWiFiStatus;
window.scanWiFiNetworks = scanWiFiNetworks;
window.displayWiFiNetworks = displayWiFiNetworks;
window.selectWiFiNetwork = selectWiFiNetwork;
window.connectToWiFi = connectToWiFi;
window.disconnectWiFi = disconnectWiFi;
window.startAccessPoint = startAccessPoint;
window.stopAccessPoint = stopAccessPoint;

// OTA functions (NEW)
window.loadOTAStatus = loadOTAStatus;
window.uploadFirmware = uploadFirmware;
window.updateOTAProgress = updateOTAProgress;

// UI functions
window.closeModal = closeModal;
window.clearActivityLog = clearActivityLog;
window.toggleDarkMode = toggleDarkMode;
window.triggerAlert = triggerAlert;
window.onTabChange = onTabChange;

// Initialize activity log
setTimeout(() => {
    addActivityLog('System initialized');
    addActivityLog('WiFi Manager ready');
    addActivityLog('OTA Manager ready');
    addActivityLog('Dashboard loaded');
}, 1000);

// Add CSS for animations and WiFi/OTA features
const style = document.createElement('style');
style.textContent = `
    @keyframes pulse {
        0% { box-shadow: 0 0 0 0 rgba(76, 175, 80, 0.7); }
        70% { box-shadow: 0 0 0 10px rgba(76, 175, 80, 0); }
        100% { box-shadow: 0 0 0 0 rgba(76, 175, 80, 0); }
    }
    
    @keyframes fadeInUp {
        from {
            opacity: 0;
            transform: translateY(20px);
        }
        to {
            opacity: 1;
            transform: translateY(0);
        }
    }
    
    .toast {
        animation: slideIn 0.3s ease;
    }
    
    @keyframes slideIn {
        from { transform: translateX(100%); opacity: 0; }
        to { transform: translateX(0); opacity: 1; }
    }
    
    .card, .stat-card, .sensor-reading, .peer-item {
        transition: transform 0.3s ease, box-shadow 0.3s ease;
    }
    
    button:not(.no-animate) {
        transition: transform 0.15s ease;
    }
    
    .modal {
        transition: opacity 0.2s ease, transform 0.2s ease;
    }
    
    .wifi-network-item {
        transition: all 0.3s ease;
    }
    
    .wifi-network-item:hover {
        transform: translateY(-2px);
        box-shadow: 0 4px 12px rgba(0,0,0,0.1);
        border-color: #4CAF50 !important;
    }
    
    .dark-mode {
        --bg-primary: #121212;
        --bg-secondary: #1e1e1e;
        --text-primary: #ffffff;
        --text-secondary: #cccccc;
    }
    
    #ota-progress-bar {
        transition: width 0.3s ease, background 0.3s ease;
    }
`;
document.head.appendChild(style);

console.log('🎯 ESP32 Control Panel v4.0 ULTIMATE loaded successfully!');
console.log('✅ All original features preserved');
console.log('✅ WiFi Manager integrated');
console.log('✅ OTA Updates integrated');
console.log('✅ Enhanced animations enabled');