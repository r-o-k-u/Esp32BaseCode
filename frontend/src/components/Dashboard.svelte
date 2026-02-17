<script>
  import { onMount } from 'svelte';
  import { 
    systemStatus, 
    sensorData, 
    espNowStatus, 
    wifiStatus,
    systemHealth,
    activityLog 
  } from '../stores/index.js';
  
  // Format bytes
  function formatBytes(bytes) {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1048576) return (bytes / 1024).toFixed(0) + ' KB';
    return (bytes / 1048576).toFixed(1) + ' MB';
  }
  
  // Format uptime
  function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    if (days > 0) return `${days}d ${hours}h`;
    if (hours > 0) return `${hours}h ${minutes}m`;
    return `${minutes}m`;
  }
  
  // Get health color
  function getHealthColor(health) {
    if (health >= 80) return 'var(--success)';
    if (health >= 60) return 'var(--warning)';
    return 'var(--danger)';
  }
  
  // Get sensor status
  function getSensorStatus(value, thresholds) {
    if (value < thresholds.warning) return 'normal';
    if (value < thresholds.danger) return 'warning';
    return 'danger';
  }
  
  let health = 0;
  let status = {};
  let sensors = {};
  let espnow = {};
  let wifi = {};
  let activities = [];
  
  // Subscribe to stores
  systemHealth.subscribe(value => health = value);
  systemStatus.subscribe(value => status = value);
  sensorData.subscribe(value => sensors = value);
  espNowStatus.subscribe(value => espnow = value);
  wifiStatus.subscribe(value => wifi = value);
  activityLog.subscribe(value => activities = value.slice(0, 10));
</script>

<div class="dashboard">
  <!-- Status Overview Cards -->
  <div class="stats-grid">
    <div class="stat-card">
      <div class="stat-header">
        <i class="fas fa-wifi"></i>
        <span>WiFi Status</span>
      </div>
      <div class="stat-value" class:connected={wifi.connected}>
        {wifi.connected ? 'Connected' : 'Disconnected'}
      </div>
      <div class="stat-details">
        <span>IP: {wifi.ip || '---'}</span>
        <span>RSSI: {wifi.rssi || 0} dBm</span>
      </div>
    </div>
    
    <div class="stat-card">
      <div class="stat-header">
        <i class="fas fa-satellite-dish"></i>
        <span>ESP-NOW</span>
      </div>
      <div class="stat-value">
        {espnow.peers || 0}
      </div>
      <div class="stat-details">
        <span>Sent: {espnow.sent || 0}</span>
        <span>Received: {espnow.received || 0}</span>
      </div>
    </div>
    
    <div class="stat-card">
      <div class="stat-header">
        <i class="fas fa-microchip"></i>
        <span>System</span>
      </div>
      <div class="stat-value">
        {formatUptime(status.uptime || 0)}
      </div>
      <div class="stat-details">
        <span>Heap: {formatBytes(status.freeHeap || 0)}</span>
        <span>CPU: {status.cpuUsage || 0}%</span>
      </div>
    </div>
    
    <div class="stat-card">
      <div class="stat-header">
        <i class="fas fa-heartbeat"></i>
        <span>Health</span>
      </div>
      <div class="stat-value" style="color: {getHealthColor(health)}">
        {health}%
      </div>
      <div class="stat-details">
        <span>RSSI: {status.wifiRSSI || 0} dBm</span>
        <span>v{status.version || '4.0'}</span>
      </div>
    </div>
  </div>
  
  <!-- Main Content Grid -->
  <div class="content-grid">
    <!-- Sensor Readings -->
    <div class="card">
      <div class="card-header">
        <h3><i class="fas fa-chart-line"></i> Real-time Sensors</h3>
      </div>
      <div class="card-body">
        <div class="sensor-grid">
          <div class="sensor-item">
            <div class="sensor-icon" style="color: #FF6384;">
              <i class="fas fa-thermometer-half"></i>
            </div>
            <div class="sensor-info">
              <span class="sensor-label">Temperature</span>
              <span class="sensor-value">{sensors.temperature?.toFixed(1) || '--'}°C</span>
            </div>
          </div>
          
          <div class="sensor-item">
            <div class="sensor-icon" style="color: #36A2EB;">
              <i class="fas fa-tint"></i>
            </div>
            <div class="sensor-info">
              <span class="sensor-label">Humidity</span>
              <span class="sensor-value">{sensors.humidity?.toFixed(1) || '--'}%</span>
            </div>
          </div>
          
          <div class="sensor-item">
            <div class="sensor-icon" style="color: #4BC0C0;">
              <i class="fas fa-tachometer-alt"></i>
            </div>
            <div class="sensor-info">
              <span class="sensor-label">Pressure</span>
              <span class="sensor-value">{sensors.pressure?.toFixed(1) || '--'} hPa</span>
            </div>
          </div>
          
          <div class="sensor-item">
            <div class="sensor-icon" style="color: #FFCE56;">
              <i class="fas fa-lightbulb"></i>
            </div>
            <div class="sensor-info">
              <span class="sensor-label">Light</span>
              <span class="sensor-value">{sensors.light || '--'} lux</span>
            </div>
          </div>
          
          <div class="sensor-item">
            <div class="sensor-icon" style="color: #9966FF;">
              <i class="fas fa-running"></i>
            </div>
            <div class="sensor-info">
              <span class="sensor-label">Motion</span>
              <span class="sensor-value">{sensors.motion ? 'Detected' : 'None'}</span>
            </div>
          </div>
          
          <div class="sensor-item">
            <div class="sensor-icon" style="color: #FF9F40;">
              <i class="fas fa-ruler"></i>
            </div>
            <div class="sensor-info">
              <span class="sensor-label">Distance</span>
              <span class="sensor-value">{sensors.distance || '--'} cm</span>
            </div>
          </div>
        </div>
        
        <div class="last-update">
          Last updated: {sensors.lastUpdate ? new Date(sensors.lastUpdate).toLocaleTimeString() : '--'}
        </div>
      </div>
    </div>
    
    <!-- Activity Timeline -->
    <div class="card">
      <div class="card-header">
        <h3><i class="fas fa-history"></i> Recent Activity</h3>
      </div>
      <div class="card-body">
        <div class="activity-list">
          {#if activities.length === 0}
            <div class="activity-empty">
              <i class="fas fa-inbox"></i>
              <span>No recent activity</span>
            </div>
          {:else}
            {#each activities as activity}
              <div class="activity-item {activity.type}">
                <div class="activity-icon">
                  {#if activity.type === 'success'}
                    <i class="fas fa-check-circle"></i>
                  {:else if activity.type === 'warning'}
                    <i class="fas fa-exclamation-triangle"></i>
                  {:else if activity.type === 'error'}
                    <i class="fas fa-times-circle"></i>
                  {:else}
                    <i class="fas fa-info-circle"></i>
                  {/if}
                </div>
                <div class="activity-content">
                  <span class="activity-message">{activity.message}</span>
                  <span class="activity-time">{activity.timestamp}</span>
                </div>
              </div>
            {/each}
          {/if}
        </div>
      </div>
    </div>
  </div>
  
  <!-- System Overview -->
  <div class="card system-overview">
    <div class="card-header">
      <h3><i class="fas fa-desktop"></i> System Overview</h3>
    </div>
    <div class="card-body">
      <div class="overview-grid">
        <div class="overview-item">
          <span class="overview-label">Device</span>
          <span class="overview-value">{status.device || 'ESP32'}</span>
        </div>
        <div class="overview-item">
          <span class="overview-label">Firmware</span>
          <span class="overview-value">{status.version || 'v4.0'}</span>
        </div>
        <div class="overview-item">
          <span class="overview-label">IP Address</span>
          <span class="overview-value">{status.ip || '---'}</span>
        </div>
        <div class="overview-item">
          <span class="overview-label">MAC Address</span>
          <span class="overview-value">{status.mac || '---'}</span>
        </div>
        <div class="overview-item">
          <span class="overview-label">WiFi RSSI</span>
          <span class="overview-value">{status.wifiRSSI || 0} dBm</span>
        </div>
        <div class="overview-item">
          <span class="overview-label">Free Heap</span>
          <span class="overview-value">{formatBytes(status.freeHeap || 0)}</span>
        </div>
        <div class="overview-item">
          <span class="overview-label">CPU Usage</span>
          <span class="overview-value">{status.cpuUsage || 0}%</span>
        </div>
        <div class="overview-item">
          <span class="overview-label">Uptime</span>
          <span class="overview-value">{formatUptime(status.uptime || 0)}</span>
        </div>
      </div>
    </div>
  </div>
</div>

<style>
  .dashboard {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
  }
  
  /* Stats Grid */
  .stats-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 1rem;
  }
  
  .stat-card {
    background: var(--glass-bg);
    backdrop-filter: blur(10px);
    border: 1px solid var(--glass-border);
    border-radius: 1rem;
    padding: 1.25rem;
    transition: all 0.3s ease;
  }
  
  .stat-card:hover {
    transform: translateY(-4px);
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3), 0 0 20px rgba(0, 255, 255, 0.1);
    border-color: var(--primary);
  }
  
  .stat-header {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    color: var(--text-muted);
    font-size: 0.8rem;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    margin-bottom: 0.75rem;
  }
  
  .stat-header i {
    color: var(--primary);
  }
  
  .stat-value {
    font-family: var(--font-mono);
    font-size: 1.8rem;
    font-weight: 700;
    color: var(--primary);
    margin-bottom: 0.5rem;
  }
  
  .stat-value.connected {
    color: var(--success);
  }
  
  .stat-details {
    display: flex;
    justify-content: space-between;
    font-size: 0.75rem;
    color: var(--text-muted);
  }
  
  /* Content Grid */
  .content-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
    gap: 1.5rem;
  }
  
  /* Card */
  .card {
    background: var(--glass-bg);
    backdrop-filter: blur(10px);
    border: 1px solid var(--glass-border);
    border-radius: 1rem;
    overflow: hidden;
    transition: all 0.3s ease;
  }
  
  .card:hover {
    border-color: var(--primary);
  }
  
  .card-header {
    padding: 1rem 1.25rem;
    background: rgba(255, 255, 255, 0.03);
    border-bottom: 1px solid var(--glass-border);
  }
  
  .card-header h3 {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    font-family: var(--font-display);
    font-size: 1rem;
    color: var(--primary);
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }
  
  .card-header h3 i {
    font-size: 1.1rem;
  }
  
  .card-body {
    padding: 1.25rem;
  }
  
  /* Sensor Grid */
  .sensor-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
    gap: 1rem;
  }
  
  .sensor-item {
    display: flex;
    align-items: center;
    gap: 0.75rem;
    padding: 0.75rem;
    background: rgba(255, 255, 255, 0.03);
    border: 1px solid var(--glass-border);
    border-radius: 0.75rem;
    transition: all 0.3s ease;
  }
  
  .sensor-item:hover {
    background: rgba(0, 255, 255, 0.05);
    border-color: var(--primary);
    transform: translateX(4px);
  }
  
  .sensor-icon {
    width: 40px;
    height: 40px;
    display: flex;
    align-items: center;
    justify-content: center;
    background: rgba(255, 255, 255, 0.05);
    border-radius: 0.5rem;
    font-size: 1.2rem;
  }
  
  .sensor-info {
    display: flex;
    flex-direction: column;
  }
  
  .sensor-label {
    font-size: 0.7rem;
    color: var(--text-muted);
    text-transform: uppercase;
  }
  
  .sensor-value {
    font-family: var(--font-mono);
    font-size: 1rem;
    font-weight: 600;
    color: var(--text-primary);
  }
  
  .last-update {
    margin-top: 1rem;
    padding-top: 0.75rem;
    border-top: 1px solid var(--glass-border);
    font-size: 0.75rem;
    color: var(--text-muted);
    text-align: center;
  }
  
  /* Activity List */
  .activity-list {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    max-height: 300px;
    overflow-y: auto;
  }
  
  .activity-empty {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.5rem;
    padding: 2rem;
    color: var(--text-muted);
  }
  
  .activity-item {
    display: flex;
    align-items: center;
    gap: 0.75rem;
    padding: 0.6rem 0.75rem;
    background: rgba(255, 255, 255, 0.02);
    border-radius: 0.5rem;
    border-left: 3px solid var(--info);
    transition: all 0.3s ease;
  }
  
  .activity-item:hover {
    background: rgba(0, 255, 255, 0.05);
    transform: translateX(4px);
  }
  
  .activity-item.success {
    border-left-color: var(--success);
  }
  
  .activity-item.warning {
    border-left-color: var(--warning);
  }
  
  .activity-item.error {
    border-left-color: var(--danger);
  }
  
  .activity-icon {
    font-size: 0.9rem;
  }
  
  .activity-item.success .activity-icon { color: var(--success); }
  .activity-item.warning .activity-icon { color: var(--warning); }
  .activity-item.error .activity-icon { color: var(--danger); }
  .activity-item.info .activity-icon { color: var(--info); }
  
  .activity-content {
    flex: 1;
    display: flex;
    justify-content: space-between;
    align-items: center;
    gap: 0.5rem;
  }
  
  .activity-message {
    font-size: 0.85rem;
    color: var(--text-primary);
  }
  
  .activity-time {
    font-family: var(--font-mono);
    font-size: 0.7rem;
    color: var(--text-muted);
  }
  
  /* System Overview */
  .system-overview {
    width: 100%;
  }
  
  .overview-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
    gap: 1rem;
  }
  
  .overview-item {
    display: flex;
    flex-direction: column;
    gap: 0.25rem;
    padding: 0.75rem;
    background: rgba(255, 255, 255, 0.03);
    border-radius: 0.5rem;
    text-align: center;
  }
  
  .overview-label {
    font-size: 0.7rem;
    color: var(--text-muted);
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }
  
  .overview-value {
    font-family: var(--font-mono);
    font-size: 1rem;
    font-weight: 600;
    color: var(--primary);
  }
  
  @media (max-width: 768px) {
    .content-grid {
      grid-template-columns: 1fr;
    }
    
    .stats-grid {
      grid-template-columns: repeat(2, 1fr);
    }
  }
  
  @media (max-width: 480px) {
    .stats-grid {
      grid-template-columns: 1fr;
    }
    
    .sensor-grid {
      grid-template-columns: 1fr;
    }
  }
</style>
