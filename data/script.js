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

// ═══════════════════════════════════════════════════════════════
// COMPREHENSIVE ACTUATOR CONTROL FUNCTIONS
// ═══════════════════════════════════════════════════════════════

// LED Control Functions
function updateLEDBrightness(brightness) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'led', value: brightness})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            document.getElementById('led-brightness-value').textContent = Math.round((brightness / 255) * 100);
        }
    })
    .catch(error => console.error('Error setting LED brightness:', error));
}

function updateRGBColor(color) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({
            actuator: 'rgb',
            r: color.r,
            g: color.g,
            b: color.b
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            addActivityLog(`RGB set to: R${color.r} G${color.g} B${color.b}`);
        }
    })
    .catch(error => console.error('Error setting RGB color:', error));
}

// Buzzer Control Functions
function triggerBuzzer(duration, frequency = 1000) {
    fetch('/api/buzzer', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({
            action: 'playTone',
            frequency: frequency,
            duration: duration
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`Buzzer: ${frequency}Hz for ${duration}ms`, 'info');
            addActivityLog(`Buzzer: ${frequency}Hz for ${duration}ms`);
        }
    })
    .catch(error => console.error('Error triggering buzzer:', error));
}

function stopBuzzer() {
    fetch('/api/buzzer', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({action: 'stop'})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('Buzzer stopped', 'info');
        }
    })
    .catch(error => console.error('Error stopping buzzer:', error));
}

function playBuzzerMelody(melodyType) {
    fetch('/api/buzzer/melody', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({melody: melodyType})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`Playing ${melodyType} melody`, 'info');
            addActivityLog(`Buzzer melody: ${melodyType}`);
        }
    })
    .catch(error => console.error('Error playing melody:', error));
}

// L298N Motor Control Functions (DUAL H-BRIDGE)
function updateMotorSpeed(speed) {
    fetch('/api/motor/speed', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({speed: speed})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            document.getElementById('speed-value').textContent = speed;
            addActivityLog(`Motor speed: ${speed}%`);
        }
    })
    .catch(error => console.error('Error setting motor speed:', error));
}

function motorForward() {
    const speed = document.getElementById('motor-speed')?.value || 100;
    fetch('/api/motor/speed', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({speed: speed, direction: 'forward'})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            document.getElementById('motor-status').textContent = 'FORWARD';
            document.getElementById('motor-status').classList.add('online');
            showToast('Motor forward', 'success');
            addActivityLog('Motor: Forward at ' + speed + '%');
        }
    })
    .catch(error => console.error('Error:', error));
}

function motorBackward() {
    const speed = document.getElementById('motor-speed')?.value || 100;
    fetch('/api/motor/speed', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({speed: speed, direction: 'backward'})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            document.getElementById('motor-status').textContent = 'BACKWARD';
            document.getElementById('motor-status').classList.add('online');
            showToast('Motor backward', 'success');
            addActivityLog('Motor: Backward at ' + speed + '%');
        }
    })
    .catch(error => console.error('Error:', error));
}

function motorStop() {
    fetch('/api/motor/stop', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            document.getElementById('motor-status').textContent = 'STOPPED';
            document.getElementById('motor-status').classList.remove('online');
            const speedInput = document.getElementById('motor-speed');
            if (speedInput) speedInput.value = 0;
            const speedVal = document.getElementById('speed-value');
            if (speedVal) speedVal.textContent = '0';
            showToast('Motor stopped', 'info');
            addActivityLog('Motor: Stopped');
        }
    })
    .catch(error => console.error('Error:', error));
}

function motorBrake() {
    fetch('/api/motor/brake', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            document.getElementById('motor-status').textContent = 'BRAKING';
            setTimeout(() => {
                document.getElementById('motor-status').textContent = 'STOPPED';
            }, 500);
            showToast('Motor brake applied', 'info');
            addActivityLog('Motor: Brake');
        }
    })
    .catch(error => console.error('Error:', error));
}

// L298N Dual Motor Controls
function motor1Forward() {
    // Control Motor 1 only - Forward
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'motorControl',
            command: 'forward',
            motor: 1,
            speed: document.getElementById('motor-speed')?.value || 100
        });
        showToast('Motor 1 forward', 'success');
        addActivityLog('Motor 1: Forward');
    }
}

function motor1Backward() {
    // Control Motor 1 only - Backward
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'motorControl',
            command: 'backward',
            motor: 1,
            speed: document.getElementById('motor-speed')?.value || 100
        });
        showToast('Motor 1 backward', 'success');
        addActivityLog('Motor 1: Backward');
    }
}

function motor2Forward() {
    // Control Motor 2 only - Forward
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'motorControl',
            command: 'forward',
            motor: 2,
            speed: document.getElementById('motor-speed')?.value || 100
        });
        showToast('Motor 2 forward', 'success');
        addActivityLog('Motor 2: Forward');
    }
}

function motor2Backward() {
    // Control Motor 2 only - Backward
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'motorControl',
            command: 'backward',
            motor: 2,
            speed: document.getElementById('motor-speed')?.value || 100
        });
        showToast('Motor 2 backward', 'success');
        addActivityLog('Motor 2: Backward');
    }
}

// L298N Tank Turn Functions
function turnLeft() {
    // Tank turn left: Motor 1 backward, Motor 2 forward
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'motorControl',
            command: 'turnLeft'
        });
        showToast('Turning left (tank turn)', 'info');
        addActivityLog('Motor: Tank turn left');
    }
}

function turnRight() {
    // Tank turn right: Motor 1 forward, Motor 2 backward
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'motorControl',
            command: 'turnRight'
        });
        showToast('Turning right (tank turn)', 'info');
        addActivityLog('Motor: Tank turn right');
    }
}

function spinLeft() {
    // Spin left: Both motors backward at different speeds
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'motorControl',
            command: 'spinLeft'
        });
        showToast('Spinning left', 'info');
        addActivityLog('Motor: Spin left');
    }
}

function spinRight() {
    // Spin right: Both motors forward at different speeds
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'motorControl',
            command: 'spinRight'
        });
        showToast('Spinning right', 'info');
        addActivityLog('Motor: Spin right');
    }
}

// Get Motor Status
function getMotorStatus() {
    fetch('/api/motor/status')
        .then(response => response.json())
        .then(data => {
            console.log('Motor status:', data);
            if (data.speed !== undefined) {
                document.getElementById('speed-value').textContent = data.speed;
                document.getElementById('motor-speed').value = data.speed;
            }
            if (data.direction) {
                document.getElementById('motor-status').textContent = data.direction.toUpperCase();
            }
        })
        .catch(error => console.error('Error getting motor status:', error));
}

// Servo Control Functions
function updateServo(servoNum, angle) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'servo' + servoNum, angle: angle})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            document.getElementById('servo-status').textContent = angle + '°';
            addActivityLog(`Servo ${servoNum}: ${angle}°`);
        }
    })
    .catch(error => console.error('Error setting servo angle:', error));
}

// Relay Control Functions
function toggleRelay(relayNum, state) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'relay' + relayNum, value: state})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            const statusEl = document.getElementById('relay' + relayNum + '-status');
            if (statusEl) {
                statusEl.textContent = state ? 'ON' : 'OFF';
                statusEl.classList.toggle('online', state);
            }
            const relayStatus = document.getElementById('relay-status');
            if (relayStatus) {
                relayStatus.textContent = state ? 'ON' : 'OFF';
                relayStatus.classList.toggle('online', state);
            }
            showToast(`Relay ${relayNum} ${state ? 'ON' : 'OFF'}`, 'success');
            addActivityLog(`Relay ${relayNum}: ${state ? 'ON' : 'OFF'}`);
        }
    })
    .catch(error => console.error('Error toggling relay:', error));
}

function pulseRelay(relayNum, duration) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'relay' + relayNum, action: 'pulse', duration: duration})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`Relay ${relayNum} pulsed for ${duration}ms`, 'info');
            addActivityLog(`Relay ${relayNum}: Pulse ${duration}ms`);
        }
    })
    .catch(error => console.error('Error pulsing relay:', error));
}

// Emergency and Reset Functions
function emergencyStop() {
    fetch('/api/actuators/emergency-stop', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('EMERGENCY STOP ACTIVATED', 'error');
            addActivityLog('EMERGENCY STOP');
            
            // Update all status indicators
            document.getElementById('led-status').textContent = 'OFF';
            document.getElementById('buzzer-status').textContent = 'OFF';
            document.getElementById('rgb-status').textContent = 'OFF';
            document.getElementById('motor-status').textContent = 'STOPPED';
            document.getElementById('relay-status').textContent = 'OFF';
            document.getElementById('servo-status').textContent = '0°';
            
            // Remove online class from all
            document.querySelectorAll('.status-indicator').forEach(el => {
                el.classList.remove('online');
            });
        }
    })
    .catch(error => console.error('Error:', error));
}

function resetAllActuators() {
    fetch('/api/actuators/reset', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('All actuators reset to defaults', 'success');
            addActivityLog('Actuators reset');
            
            // Reset UI elements
            document.getElementById('led-brightness').value = 0;
            document.getElementById('led-brightness-value').textContent = '0';
            document.getElementById('motor-speed').value = 0;
            document.getElementById('speed-value').textContent = '0';
            document.getElementById('servo-angle').value = 90;
            document.getElementById('angle-value').textContent = '90';
            document.getElementById('rgb-r').value = 0;
            document.getElementById('rgb-g').value = 0;
            document.getElementById('rgb-b').value = 0;
            updateRGBSliders();
        }
    })
    .catch(error => console.error('Error:', error));
}

// RGB LED Effects
function startRGBEffect(effect) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        sendWSMessage({
            type: 'rgbEffect',
            effect: effect
        });
        showToast(`RGB Effect: ${effect}`, 'info');
        addActivityLog(`RGB Effect: ${effect}`);
    }
}

function setRGBBrightness(brightness) {
    fetch('/api/actuator', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({actuator: 'rgb', brightness: brightness})
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            addActivityLog(`RGB Brightness: ${brightness}%`);
        }
    })
    .catch(error => console.error('Error:', error));
}

// WebSocket-based Actuator Control
function sendActuatorCommand(actuator, command, value) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        const msg = {
            type: 'actuatorCommand',
            actuator: actuator,
            command: command
        };
        if (value !== undefined) {
            msg.value = value;
        }
        sendWSMessage(msg);
    } else {
        // Fallback to HTTP
        fetch('/api/actuator', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({actuator: actuator, value: value, command: command})
        });
    }
}

// Blink LED
function blinkLED(times, interval) {
    const data = {
        times: times,
        interval: interval
    };

    $.ajax({
        type: "POST",
        url: "/api/led/blink",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("LED blink:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error blinking LED:", error);
        }
    });
}

// Motor Ramp
function rampMotor(direction, duration) {
    const data = {
        direction: direction,
        duration: duration
    };

    $.ajax({
        type: "POST",
        url: "/api/motor/ramp",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("Motor ramp:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error ramping motor:", error);
        }
    });
}

// Servo Sweep
function sweepServo(servo, startAngle, endAngle, speed) {
    const data = {
        servo: servo,
        startAngle: startAngle,
        endAngle: endAngle,
        speed: speed
    };

    $.ajax({
        type: "POST",
        url: "/api/servo/sweep",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("Servo sweep:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error sweeping servo:", error);
        }
    });
}

// Cycle Relays
function cycleRelays(times, interval) {
    const data = {
        times: times,
        interval: interval
    };

    $.ajax({
        type: "POST",
        url: "/api/relays/cycle",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("Relays cycle:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error cycling relays:", error);
        }
    });
}

// Play Melody
function playMelody(melody) {
    const data = {
        melody: melody
    };

    $.ajax({
        type: "POST",
        url: "/api/buzzer/melody",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("Melody played:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error playing melody:", error);
        }
    });
}

// Set Motor Speed
function setMotorSpeed(speed) {
    const data = {
        speed: speed
    };

    $.ajax({
        type: "POST",
        url: "/api/motor/speed",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("Motor speed set:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error setting motor speed:", error);
        }
    });
}

// Set Servo Angle
function setServoAngle(servo, angle) {
    const data = {
        servo: servo,
        angle: angle
    };

    $.ajax({
        type: "POST",
        url: "/api/servo/angle",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("Servo angle set:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error setting servo angle:", error);
        }
    });
}

// Set RGB LED Color
function setRGBLEDColor(red, green, blue) {
    const data = {
        red: red,
        green: green,
        blue: blue
    };

    $.ajax({
        type: "POST",
        url: "/api/rgbled/color",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("RGB LED color set:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error setting RGB LED color:", error);
        }
    });
}

// Set Buzzer Frequency
function setBuzzerFrequency(frequency, duration) {
    const data = {
        frequency: frequency,
        duration: duration
    };

    $.ajax({
        type: "POST",
        url: "/api/buzzer/frequency",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("Buzzer frequency set:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error setting buzzer frequency:", error);
        }
    });
}

// Set Relay State
function setRelayState(relay, state) {
    const data = {
        relay: relay,
        state: state
    };

    $.ajax({
        type: "POST",
        url: "/api/relay/state",
        data: JSON.stringify(data),
        contentType: "application/json",
        success: function(response) {
            console.log("Relay state set:", response);
        },
        error: function(xhr, status, error) {
            console.error("Error setting relay state:", error);
        }
    });
}

// Get Actuator Status
function getActuatorStatus() {
    fetch('/api/actuator/status')
        .then(response => response.json())
        .then(data => {
            updateActuatorStatus(data);
        })
        .catch(error => {
            console.error('Error getting actuator status:', error);
        });
}

// Update Actuator Status Display
function updateActuatorStatus(data) {
    const actuatorContainer = document.getElementById('actuator-status');
    if (!actuatorContainer) return;
    
    let html = '';
    
    if (data.led) {
        html += `<div class="actuator-status-item">
                    <span class="actuator-name">LED</span>
                    <span class="actuator-value">${data.led ? 'ON' : 'OFF'}</span>
                 </div>`;
    }
    
    if (data.motor) {
        html += `<div class="actuator-status-item">
                    <span class="actuator-name">Motor</span>
                    <span class="actuator-value">${data.motor.speed || 0}%</span>
                 </div>`;
    }
    
    if (data.servo) {
        html += `<div class="actuator-status-item">
                    <span class="actuator-name">Servo</span>
                    <span class="actuator-value">${data.servo.angle || 0}°</span>
                 </div>`;
    }
    
    if (data.relay) {
        html += `<div class="actuator-status-item">
                    <span class="actuator-name">Relay</span>
                    <span class="actuator-value">${data.relay.state ? 'ON' : 'OFF'}</span>
                 </div>`;
    }
    
    if (data.rgbled) {
        html += `<div class="actuator-status-item">
                    <span class="actuator-name">RGB LED</span>
                    <span class="actuator-value">R:${data.rgbled.red || 0} G:${data.rgbled.green || 0} B:${data.rgbled.blue || 0}</span>
                 </div>`;
    }
    
    if (data.buzzer) {
        html += `<div class="actuator-status-item">
                    <span class="actuator-name">Buzzer</span>
                    <span class="actuator-value">${data.buzzer.frequency || 0}Hz</span>
                 </div>`;
    }
    
    actuatorContainer.innerHTML = html || '<p style="color: var(--text-muted);">No actuator data available</p>';
}

