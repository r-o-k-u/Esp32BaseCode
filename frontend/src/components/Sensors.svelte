<script>
  import { sensorData, addActivity, showToast } from '../stores/index.js';
  import { sendWSMessage } from '../services/api.js';
  
  let sensors = {};
  let autoRefresh = true;
  let refreshInterval;
  
  sensorData.subscribe(value => sensors = value);
  
  function refreshSensorData() {
    sendWSMessage({ type: 'getSensorData' });
    addActivity('Sensor data refreshed', 'info');
  }
  
  function calibrateSensor(type) {
    if (confirm(`Calibrate ${type.toUpperCase()} sensor?`)) {
      showToast(`Calibrating ${type} sensor...`, 'info');
      sendWSMessage({ type: 'calibrate', sensor: type });
    }
  }
</script>

<div class="sensors-page">
  <div class="page-header">
    <h2><i class="fas fa-thermometer-half"></i> Sensor Monitoring</h2>
    <div class="header-actions">
      <button class="btn btn-primary" on:click={refreshSensorData}>
        <i class="fas fa-sync-alt"></i> Refresh
      </button>
      <label class="toggle">
        <input type="checkbox" bind:checked={autoRefresh}>
        <span class="toggle-slider"></span>
        <span class="toggle-label">Auto Refresh</span>
      </label>
    </div>
  </div>
  
  <!-- Sensor Cards Grid -->
  <div class="sensor-grid">
    <!-- Temperature & Humidity -->
    <div class="sensor-card">
      <div class="sensor-header">
        <div class="sensor-title">
          <i class="fas fa-thermometer-half" style="color: #FF6384;"></i>
          <span>DHT22 Sensor</span>
        </div>
        <div class="sensor-status online">
          <span class="status-dot"></span>
          Active
        </div>
      </div>
      <div class="sensor-body">
        <div class="sensor-reading">
          <span class="reading-label">Temperature</span>
          <span class="reading-value">{sensors.temperature?.toFixed(1) || '--'}°C</span>
        </div>
        <div class="sensor-reading">
          <span class="reading-label">Humidity</span>
          <span class="reading-value">{sensors.humidity?.toFixed(1) || '--'}%</span>
        </div>
      </div>
      <div class="sensor-actions">
        <button class="btn btn-small" on:click={() => calibrateSensor('dht')}>
          <i class="fas fa-sliders-h"></i> Calibrate
        </button>
      </div>
    </div>
    
    <!-- Pressure (BMP280) -->
    <div class="sensor-card">
      <div class="sensor-header">
        <div class="sensor-title">
          <i class="fas fa-tachometer-alt" style="color: #36A2EB;"></i>
          <span>BMP280 Sensor</span>
        </div>
        <div class="sensor-status online">
          <span class="status-dot"></span>
          Active
        </div>
      </div>
      <div class="sensor-body">
        <div class="sensor-reading">
          <span class="reading-label">Pressure</span>
          <span class="reading-value">{sensors.pressure?.toFixed(1) || '--'} hPa</span>
        </div>
      </div>
      <div class="sensor-actions">
        <button class="btn btn-small" on:click={() => calibrateSensor('bmp')}>
          <i class="fas fa-sliders-h"></i> Calibrate
        </button>
      </div>
    </div>
    
    <!-- Light (LDR) -->
    <div class="sensor-card">
      <div class="sensor-header">
        <div class="sensor-title">
          <i class="fas fa-lightbulb" style="color: #FFCE56;"></i>
          <span>LDR Sensor</span>
        </div>
        <div class="sensor-status online">
          <span class="status-dot"></span>
          Active
        </div>
      </div>
      <div class="sensor-body">
        <div class="sensor-reading">
          <span class="reading-label">Light Level</span>
          <span class="reading-value">{sensors.light || '--'} lux</span>
        </div>
      </div>
    </div>
    
    <!-- Motion (PIR) -->
    <div class="sensor-card">
      <div class="sensor-header">
        <div class="sensor-title">
          <i class="fas fa-running" style="color: #FF6384;"></i>
          <span>PIR Sensor</span>
        </div>
        <div class="sensor-status" class:online={sensors.motion}>
          <span class="status-dot"></span>
          {sensors.motion ? 'Motion Detected' : 'No Motion'}
        </div>
      </div>
      <div class="sensor-body">
        <div class="sensor-reading">
          <span class="reading-label">Status</span>
          <span class="reading-value">{sensors.motion ? 'DETECTED' : 'CLEAR'}</span>
        </div>
      </div>
    </div>
    
    <!-- Distance (Ultrasonic) -->
    <div class="sensor-card">
      <div class="sensor-header">
        <div class="sensor-title">
          <i class="fas fa-ruler" style="color: #4BC0C0;"></i>
          <span>Ultrasonic Sensor</span>
        </div>
        <div class="sensor-status online">
          <span class="status-dot"></span>
          Active
        </div>
      </div>
      <div class="sensor-body">
        <div class="sensor-reading">
          <span class="reading-label">Distance</span>
          <span class="reading-value">{sensors.distance || '--'} cm</span>
        </div>
      </div>
    </div>
    
    <!-- Air Quality (MQ135) -->
    <div class="sensor-card">
      <div class="sensor-header">
        <div class="sensor-title">
          <i class="fas fa-wind" style="color: #9966FF;"></i>
          <span>MQ135 Sensor</span>
        </div>
        <div class="sensor-status online">
          <span class="status-dot"></span>
          Active
        </div>
      </div>
      <div class="sensor-body">
        <div class="sensor-reading">
          <span class="reading-label">Air Quality</span>
          <span class="reading-value">{sensors.airQuality || '--'} ppm</span>
        </div>
      </div>
    </div>
  </div>
</div>

<style>
  .sensors-page {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
  }
  
  .page-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 1rem;
  }
  
  .page-header h2 {
    display: flex;
    align-items: center;
    gap: 0.75rem;
    font-family: var(--font-display);
    font-size: 1.5rem;
    color: var(--primary);
    text-transform: uppercase;
  }
  
  .header-actions {
    display: flex;
    align-items: center;
    gap: 1rem;
  }
  
  .btn {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.6rem 1rem;
    border: 1px solid var(--primary);
    border-radius: 0.5rem;
    background: rgba(0, 255, 255, 0.1);
    color: var(--primary);
    font-family: var(--font-display);
    font-size: 0.75rem;
    font-weight: 600;
    text-transform: uppercase;
    cursor: pointer;
    transition: all 0.3s ease;
  }
  
  .btn:hover {
    background: var(--primary);
    color: var(--bg-primary);
  }
  
  .btn-small {
    padding: 0.4rem 0.75rem;
    font-size: 0.7rem;
  }
  
  .toggle {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    cursor: pointer;
  }
  
  .toggle input {
    display: none;
  }
  
  .toggle-slider {
    width: 40px;
    height: 20px;
    background: rgba(255, 255, 255, 0.1);
    border-radius: 10px;
    position: relative;
    transition: all 0.3s ease;
  }
  
  .toggle-slider::after {
    content: '';
    position: absolute;
    top: 2px;
    left: 2px;
    width: 16px;
    height: 16px;
    background: var(--text-muted);
    border-radius: 50%;
    transition: all 0.3s ease;
  }
  
  .toggle input:checked + .toggle-slider {
    background: var(--primary);
  }
  
  .toggle input:checked + .toggle-slider::after {
    left: 22px;
    background: var(--bg-primary);
  }
  
  .toggle-label {
    font-size: 0.8rem;
    color: var(--text-secondary);
  }
  
  .sensor-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 1.25rem;
  }
  
  .sensor-card {
    background: var(--glass-bg);
    backdrop-filter: blur(10px);
    border: 1px solid var(--glass-border);
    border-radius: 1rem;
    overflow: hidden;
    transition: all 0.3s ease;
  }
  
  .sensor-card:hover {
    border-color: var(--primary);
    transform: translateY(-4px);
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
  }
  
  .sensor-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 1rem;
    background: rgba(255, 255, 255, 0.03);
    border-bottom: 1px solid var(--glass-border);
  }
  
  .sensor-title {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    font-weight: 600;
  }
  
  .sensor-title i {
    font-size: 1.1rem;
  }
  
  .sensor-status {
    display: flex;
    align-items: center;
    gap: 0.4rem;
    font-size: 0.75rem;
    color: var(--text-muted);
  }
  
  .sensor-status.online {
    color: var(--success);
  }
  
  .status-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: currentColor;
    animation: pulse 2s infinite;
  }
  
  @keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.5; }
  }
  
  .sensor-body {
    padding: 1.25rem;
    display: flex;
    flex-direction: column;
    gap: 1rem;
  }
  
  .sensor-reading {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0.75rem;
    background: rgba(255, 255, 255, 0.03);
    border-radius: 0.5rem;
  }
  
  .reading-label {
    font-size: 0.85rem;
    color: var(--text-muted);
  }
  
  .reading-value {
    font-family: var(--font-mono);
    font-size: 1.25rem;
    font-weight: 700;
    color: var(--primary);
  }
  
  .sensor-actions {
    padding: 0.75rem 1rem;
    border-top: 1px solid var(--glass-border);
    display: flex;
    justify-content: flex-end;
  }
</style>
