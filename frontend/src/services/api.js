/**
 * ═══════════════════════════════════════════════════════════════════
 * ESP32 CONTROL PANEL - API SERVICE
 * ═══════════════════════════════════════════════════════════════════
 * Communication layer with ESP32 via HTTP and WebSocket
 */

import { 
  connectionStatus, 
  systemStatus, 
  wifiStatus, 
  espNowStatus,
  sensorData,
  actuatorStatus,
  otaStatus,
  wifiNetworks,
  espNowPeers,
  messageLog,
  addActivity,
  showToast
} from '../stores/index.js';

let ws = null;
let reconnectInterval = null;
const RECONNECT_DELAY = 3000;

/**
 * Initialize WebSocket connection
 */
export function initWebSocket() {
  const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
  const wsUrl = `${protocol}//${location.host}/ws`;
  
  console.log('🔌 Connecting to WebSocket:', wsUrl);
  
  ws = new WebSocket(wsUrl);
  
  ws.onopen = () => {
    console.log('✅ WebSocket connected');
    connectionStatus.update(s => ({ 
      ...s, 
      connected: true, 
      lastConnected: new Date(),
      reconnectAttempts: 0 
    }));
    clearInterval(reconnectInterval);
    
    // Request initial data
    sendWSMessage({ type: 'getStatus' });
    sendWSMessage({ type: 'getSensorData' });
    sendWSMessage({ type: 'getPeers' });
    sendWSMessage({ type: 'getActuatorStatus' });
    
    showToast('Connected to ESP32', 'success');
    addActivity('WebSocket connected');
  };
  
  ws.onclose = () => {
    console.log('❌ WebSocket disconnected');
    connectionStatus.update(s => ({ ...s, connected: false }));
    addActivity('WebSocket disconnected', 'warning');
    
    // Attempt reconnection
    reconnectInterval = setInterval(() => {
      console.log('🔄 Attempting to reconnect...');
      connectionStatus.update(s => ({ ...s, reconnectAttempts: s.reconnectAttempts + 1 }));
      initWebSocket();
    }, RECONNECT_DELAY);
  };
  
  ws.onerror = (error) => {
    console.error('⚠️ WebSocket error:', error);
    showToast('Connection error', 'error');
  };
  
  ws.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      handleWSMessage(data);
    } catch (e) {
      console.error('Error parsing message:', e);
    }
  };
}

/**
 * Send message via WebSocket
 * @param {Object} message - Message to send
 */
export function sendWSMessage(message) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify(message));
    console.log('📤 Sent:', message);
  } else {
    console.error('WebSocket not connected');
    showToast('Not connected to device', 'error');
  }
}

/**
 * Handle incoming WebSocket messages
 * @param {Object} data - Received message
 */
function handleWSMessage(data) {
  console.log('📨 WebSocket message:', data);
  
  switch (data.type) {
    case 'status':
      systemStatus.set(data);
      break;
      
    case 'sensor':
      sensorData.update(s => ({ ...s, ...data, lastUpdate: new Date() }));
      break;
      
    case 'peers':
      espNowPeers.set(data.peers || []);
      espNowStatus.update(s => ({ 
        ...s, 
        peers: data.peers?.length || 0 
      }));
      break;
      
    case 'espnowMessage':
      messageLog.update(log => [...log, data].slice(-100));
      addActivity(`ESP-NOW: ${data.message || 'Message received'}`, 'info');
      break;
      
    case 'actuatorStatus':
      actuatorStatus.set(data);
      break;
      
    case 'alert':
      showToast(data.message, 'error');
      addActivity(`Alert: ${data.message}`, 'error');
      break;
      
    case 'connected':
      showToast(data.message, 'success');
      break;
      
    case 'log':
      addActivity(data.message, data.level || 'info');
      break;
      
    case 'wifiNetworks':
      wifiNetworks.set(data.networks || []);
      break;
      
    case 'wifiConnecting':
      showToast(`Connecting to ${data.ssid}...`, 'info');
      break;
      
    case 'otaProgress':
      otaStatus.update(s => ({ 
        ...s, 
        updating: data.progress < 100,
        progress: data.progress 
      }));
      break;
  }
}

/**
 * Fetch WiFi status
 */
export async function fetchWiFiStatus() {
  try {
    const response = await fetch('/api/wifi/status');
    const data = await response.json();
    wifiStatus.set(data);
    return data;
  } catch (error) {
    console.error('Error fetching WiFi status:', error);
    showToast('Failed to load WiFi status', 'error');
    throw error;
  }
}

/**
 * Scan for WiFi networks
 */
export async function scanWiFiNetworks() {
  try {
    const response = await fetch('/api/wifi/scan');
    const data = await response.json();
    wifiNetworks.set(data.networks || []);
    showToast(`Found ${data.count} networks`, 'success');
    addActivity(`Found ${data.count} WiFi networks`);
    return data;
  } catch (error) {
    console.error('Error scanning WiFi:', error);
    showToast('Failed to scan networks', 'error');
    throw error;
  }
}

/**
 * Connect to WiFi network
 * @param {string} ssid - Network SSID
 * @param {string} password - Network password
 */
export async function connectToWiFi(ssid, password) {
  try {
    const response = await fetch('/api/wifi/connect', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid, password })
    });
    const data = await response.json();
    
    if (data.success) {
      showToast(`Connected to ${ssid}`, 'success');
      addActivity(`Connected to WiFi: ${ssid} (${data.ip})`);
      await fetchWiFiStatus();
    } else {
      showToast(`Failed: ${data.error}`, 'error');
      addActivity(`WiFi connection failed: ${data.error}`, 'error');
    }
    
    return data;
  } catch (error) {
    console.error('Error connecting to WiFi:', error);
    showToast('Connection error', 'error');
    throw error;
  }
}

/**
 * Disconnect from WiFi
 */
export async function disconnectWiFi() {
  try {
    const response = await fetch('/api/wifi/disconnect', { method: 'POST' });
    const data = await response.json();
    
    if (data.success) {
      showToast('WiFi disconnected', 'success');
      addActivity('WiFi disconnected');
      await fetchWiFiStatus();
    }
    
    return data;
  } catch (error) {
    console.error('Error disconnecting WiFi:', error);
    showToast('Failed to disconnect', 'error');
    throw error;
  }
}

/**
 * Start Access Point
 * @param {string} ssid - AP SSID
 * @param {string} password - AP password
 */
export async function startAccessPoint(ssid, password) {
  try {
    const response = await fetch('/api/wifi/ap/start', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid, password })
    });
    const data = await response.json();
    
    if (data.success) {
      showToast(`AP started: ${data.ssid}`, 'success');
      addActivity(`Access Point started: ${data.ssid} (${data.ip})`);
      await fetchWiFiStatus();
    }
    
    return data;
  } catch (error) {
    console.error('Error starting AP:', error);
    showToast('Failed to start AP', 'error');
    throw error;
  }
}

/**
 * Stop Access Point
 */
export async function stopAccessPoint() {
  try {
    const response = await fetch('/api/wifi/ap/stop', { method: 'POST' });
    const data = await response.json();
    
    if (data.success) {
      showToast('Access Point stopped', 'success');
      addActivity('Access Point stopped');
      await fetchWiFiStatus();
    }
    
    return data;
  } catch (error) {
    console.error('Error stopping AP:', error);
    showToast('Failed to stop AP', 'error');
    throw error;
  }
}

/**
 * Toggle actuator state
 * @param {string} actuator - Actuator name
 * @param {*} value - New value
 */
export async function toggleActuator(actuator, value) {
  try {
    const response = await fetch('/api/actuator', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ actuator, value })
    });
    const data = await response.json();
    
    if (data.success) {
      showToast(`${actuator} turned ${value ? 'ON' : 'OFF'}`, 'success');
      addActivity(`${actuator} ${value ? 'ON' : 'OFF'}`);
    } else {
      showToast('Command failed', 'error');
    }
    
    return data;
  } catch (error) {
    console.error('Error toggling actuator:', error);
    showToast('Communication error', 'error');
    throw error;
  }
}

/**
 * Set LED brightness
 * @param {number} brightness - Brightness value 0-255
 */
export async function setLEDBrightness(brightness) {
  return toggleActuator('led', brightness);
}

/**
 * Set RGB LED color
 * @param {number} r - Red value 0-255
 * @param {number} g - Green value 0-255
 * @param {number} b - Blue value 0-255
 */
export async function setRGBColor(r, g, b) {
  return toggleActuator('rgb', { r, g, b });
}

/**
 * Trigger buzzer
 * @param {number} duration - Duration in ms
 * @param {number} frequency - Frequency in Hz
 */
export async function triggerBuzzer(duration, frequency = 1000) {
  try {
    const response = await fetch('/api/buzzer', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ action: 'playTone', frequency, duration })
    });
    return await response.json();
  } catch (error) {
    console.error('Error triggering buzzer:', error);
    throw error;
  }
}

/**
 * Control motor
 * @param {string} command - Command: 'forward', 'backward', 'stop', 'brake'
 * @param {number} speed - Speed 0-100
 */
export async function controlMotor(command, speed = 100) {
  try {
    const response = await fetch(`/api/motor/${command}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ speed })
    });
    return await response.json();
  } catch (error) {
    console.error('Error controlling motor:', error);
    throw error;
  }
}

/**
 * Control servo
 * @param {number} angle - Angle 0-180
 */
export async function controlServo(angle) {
  return toggleActuator('servo', angle);
}

/**
 * Toggle relay
 * @param {number} relayNum - Relay number 1-3
 * @param {boolean} state - State
 */
export async function toggleRelay(relayNum, state) {
  return toggleActuator(`relay${relayNum}`, state);
}

/**
 * Emergency stop all actuators
 */
export async function emergencyStop() {
  try {
    const response = await fetch('/api/actuators/emergency-stop', { method: 'POST' });
    const data = await response.json();
    
    if (data.success) {
      showToast('EMERGENCY STOP ACTIVATED', 'error');
      addActivity('EMERGENCY STOP', 'error');
      actuatorStatus.set({
        led: { on: false, brightness: 0 },
        rgb: { on: false, r: 0, g: 0, b: 0, effect: null },
        buzzer: { on: false, frequency: 1000 },
        motor: { on: false, speed: 0, direction: 'stop' },
        relays: [false, false, false],
        servo: { on: false, angle: 0 }
      });
    }
    
    return data;
  } catch (error) {
    console.error('Error in emergency stop:', error);
    showToast('Communication error', 'error');
    throw error;
  }
}

/**
 * Reset all actuators
 */
export async function resetAllActuators() {
  try {
    const response = await fetch('/api/actuators/reset', { method: 'POST' });
    const data = await response.json();
    
    if (data.success) {
      showToast('All actuators reset', 'success');
      addActivity('Actuators reset');
    }
    
    return data;
  } catch (error) {
    console.error('Error resetting actuators:', error);
    throw error;
  }
}

/**
 * Refresh ESP-NOW peers
 */
export async function refreshPeers() {
  sendWSMessage({ type: 'getPeers' });
  addActivity('Refreshing ESP-NOW peers', 'info');
}

/**
 * Get OTA status
 */
export async function fetchOTAStatus() {
  try {
    const response = await fetch('/api/ota/status');
    const data = await response.json();
    otaStatus.set(data);
    return data;
  } catch (error) {
    console.error('Error fetching OTA status:', error);
    throw error;
  }
}

/**
 * Upload firmware for OTA update
 * @param {File} file - Firmware file
 * @param {Function} onProgress - Progress callback
 */
export async function uploadFirmware(file, onProgress) {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    
    xhr.upload.addEventListener('progress', (e) => {
      if (e.lengthComputable && onProgress) {
        const progress = Math.round((e.loaded / e.total) * 100);
        onProgress(progress);
        otaStatus.update(s => ({ ...s, progress, updating: true }));
      }
    });
    
    xhr.addEventListener('load', () => {
      if (xhr.status === 200) {
        otaStatus.update(s => ({ 
          ...s, 
          progress: 100, 
          updating: false,
          totalUpdates: s.totalUpdates + 1 
        }));
        showToast('Firmware uploaded successfully', 'success');
        addActivity('Firmware update complete');
        resolve(JSON.parse(xhr.responseText));
      } else {
        otaStatus.update(s => ({ 
          ...s, 
          progress: 0, 
          updating: false,
          failedUpdates: s.failedUpdates + 1 
        }));
        showToast('Firmware upload failed', 'error');
        reject(new Error('Upload failed'));
      }
    });
    
    xhr.addEventListener('error', () => {
      otaStatus.update(s => ({ ...s, progress: 0, updating: false }));
      showToast('Upload error', 'error');
      reject(new Error('Upload error'));
    });
    
    const formData = new FormData();
    formData.append('firmware', file);
    
    xhr.open('POST', '/api/ota/update');
    xhr.send(formData);
  });
}

/**
 * Disconnect WebSocket
 */
export function disconnectWebSocket() {
  if (ws) {
    ws.close();
    clearInterval(reconnectInterval);
  }
}
