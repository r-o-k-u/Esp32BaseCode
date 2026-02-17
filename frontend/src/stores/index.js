/**
 * ═══════════════════════════════════════════════════════════════════
 * ESP32 CONTROL PANEL - REACTIVE STORES
 * ═══════════════════════════════════════════════════════════════════
 * State management using Svelte stores for reactive data handling
 */

import { writable, derived } from 'svelte/store';

/**
 * WebSocket connection state
 */
export const connectionStatus = writable({
  connected: false,
  lastConnected: null,
  reconnectAttempts: 0
});

/**
 * System status from ESP32
 */
export const systemStatus = writable({
  device: 'ESP32',
  version: 'v4.0',
  uptime: 0,
  freeHeap: 0,
  cpuUsage: 0,
  wifiRSSI: 0,
  ip: '---',
  mac: '---'
});

/**
 * WiFi connection state
 */
export const wifiStatus = writable({
  connected: false,
  ssid: '',
  ip: '',
  gateway: '',
  subnet: '',
  dns: '',
  mac: '',
  rssi: 0,
  apMode: false,
  apSSID: '',
  apIP: '',
  apClients: 0
});

/**
 * ESP-NOW communication state
 */
export const espNowStatus = writable({
  initialized: false,
  peers: 0,
  sent: 0,
  received: 0,
  failed: 0
});

/**
 * Sensor data store
 */
export const sensorData = writable({
  temperature: 0,
  humidity: 0,
  pressure: 0,
  light: 0,
  motion: false,
  distance: 0,
  airQuality: 0,
  soilMoisture: 0,
  lastUpdate: null
});

/**
 * Actuator states
 */
export const actuatorStatus = writable({
  led: { on: false, brightness: 0 },
  rgb: { on: false, r: 0, g: 0, b: 0, effect: null },
  buzzer: { on: false, frequency: 1000 },
  motor: { on: false, speed: 0, direction: 'stop' },
  relays: [false, false, false],
  servo: { on: false, angle: 0 }
});

/**
 * Activity log
 */
export const activityLog = writable([]);

/**
 * Toast notifications
 */
export const toasts = writable([]);

/**
 * Current navigation tab
 */
export const currentTab = writable('dashboard');

/**
 * Loading states
 */
export const loadingStates = writable({
  sensors: false,
  actuators: false,
  wifi: false,
  espnow: false,
  ota: false
});

/**
 * OTA update state
 */
export const otaStatus = writable({
  initialized: false,
  updating: false,
  progress: 0,
  hostname: '',
  port: 3232,
  totalUpdates: 0,
  failedUpdates: 0,
  lastUpdate: null
});

/**
 * WiFi networks list
 */
export const wifiNetworks = writable([]);

/**
 * ESP-NOW peers list
 */
export const espNowPeers = writable([]);

/**
 * Message log for communication
 */
export const messageLog = writable([]);

/**
 * Derived store for health calculation
 */
export const systemHealth = derived(
  [systemStatus, sensorData],
  ([$systemStatus, $sensorData]) => {
    let health = 100;
    
    // Deduct based on conditions
    if ($sensorData.temperature > 30) health -= 10;
    if ($sensorData.humidity > 80) health -= 10;
    if ($systemStatus.cpuUsage > 70) health -= 10;
    if ($systemStatus.freeHeap < 50000) health -= 10;
    if ($systemStatus.wifiRSSI < -70) health -= 10;
    
    return Math.max(0, Math.min(100, health));
  }
);

/**
 * Add activity to log
 * @param {string} message - Activity message
 * @param {string} type - Activity type: 'info', 'success', 'warning', 'error'
 */
export function addActivity(message, type = 'info') {
  const timestamp = new Date().toLocaleTimeString();
  activityLog.update(log => {
    const entry = { message, type, timestamp, id: Date.now() };
    return [entry, ...log].slice(0, 100); // Keep last 100 entries
  });
}

/**
 * Show toast notification
 * @param {string} message - Toast message
 * @param {string} type - Toast type: 'success', 'error', 'warning', 'info'
 * @param {number} duration - Duration in ms
 */
export function showToast(message, type = 'success', duration = 5000) {
  const id = Date.now();
  toasts.update(t => [...t, { id, message, type }]);
  
  setTimeout(() => {
    toasts.update(t => t.filter(toast => toast.id !== id));
  }, duration);
}

/**
 * Reset all stores (for testing/logout)
 */
export function resetStores() {
  connectionStatus.set({ connected: false, lastConnected: null, reconnectAttempts: 0 });
  systemStatus.set({
    device: 'ESP32',
    version: 'v4.0',
    uptime: 0,
    freeHeap: 0,
    cpuUsage: 0,
    wifiRSSI: 0,
    ip: '---',
    mac: '---'
  });
  wifiStatus.set({
    connected: false,
    ssid: '',
    ip: '',
    gateway: '',
    subnet: '',
    dns: '',
    mac: '',
    rssi: 0,
    apMode: false,
    apSSID: '',
    apIP: '',
    apClients: 0
  });
  espNowStatus.set({
    initialized: false,
    peers: 0,
    sent: 0,
    received: 0,
    failed: 0
  });
  sensorData.set({
    temperature: 0,
    humidity: 0,
    pressure: 0,
    light: 0,
    motion: false,
    distance: 0,
    airQuality: 0,
    soilMoisture: 0,
    lastUpdate: null
  });
  actuatorStatus.set({
    led: { on: false, brightness: 0 },
    rgb: { on: false, r: 0, g: 0, b: 0, effect: null },
    buzzer: { on: false, frequency: 1000 },
    motor: { on: false, speed: 0, direction: 'stop' },
    relays: [false, false, false],
    servo: { on: false, angle: 0 }
  });
  activityLog.set([]);
  wifiNetworks.set([]);
  espNowPeers.set([]);
  messageLog.set([]);
}
