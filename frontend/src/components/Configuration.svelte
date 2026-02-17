<script>
  import { systemStatus, showToast, addActivity } from '../stores/index.js';
  
  let status = {};
  let config = {
    deviceName: 'ESP32',
    refreshInterval: 5000,
    autoReconnect: true,
    notifications: true,
    darkMode: true
  };
  
  systemStatus.subscribe(value => status = value);
  
  function saveConfig() {
    localStorage.setItem('esp32Config', JSON.stringify(config));
    showToast('Configuration saved', 'success');
    addActivity('Configuration saved');
  }
  
  function loadConfig() {
    const saved = localStorage.getItem('esp32Config');
    if (saved) {
      config = { ...config, ...JSON.parse(saved) };
    }
  }
  
  loadConfig();
</script>

<div class="config-page">
  <div class="page-header">
    <h2><i class="fas fa-sliders-h"></i> Configuration</h2>
    <button class="btn btn-primary" on:click={saveConfig}>
      <i class="fas fa-save"></i> Save Settings
    </button>
  </div>
  
  <div class="config-grid">
    <!-- Device Settings -->
    <div class="config-card">
      <div class="card-header">
        <h3><i class="fas fa-microchip"></i> Device Settings</h3>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label>Device Name</label>
          <input type="text" bind:value={config.deviceName}>
        </div>
      </div>
    </div>
    
    <!-- Connection Settings -->
    <div class="config-card">
      <div class="card-header">
        <h3><i class="fas fa-plug"></i> Connection</h3>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label>Refresh Interval (ms)</label>
          <input type="number" bind:value={config.refreshInterval}>
        </div>
        <div class="form-group checkbox">
          <label>
            <input type="checkbox" bind:checked={config.autoReconnect}>
            <span>Auto Reconnect</span>
          </label>
        </div>
      </div>
    </div>
    
    <!-- UI Settings -->
    <div class="config-card">
      <div class="card-header">
        <h3><i class="fas fa-palette"></i> User Interface</h3>
      </div>
      <div class="card-body">
        <div class="form-group checkbox">
          <label>
            <input type="checkbox" bind:checked={config.darkMode}>
            <span>Dark Mode</span>
          </label>
        </div>
        <div class="form-group checkbox">
          <label>
            <input type="checkbox" bind:checked={config.notifications}>
            <span>Enable Notifications</span>
          </label>
        </div>
      </div>
    </div>
    
    <!-- System Info -->
    <div class="config-card">
      <div class="card-header">
        <h3><i class="fas fa-info-circle"></i> System Information</h3>
      </div>
      <div class="card-body">
        <div class="info-row">
          <span class="info-label">Device</span>
          <span class="info-value">{status.device || 'ESP32'}</span>
        </div>
        <div class="info-row">
          <span class="info-label">Firmware</span>
          <span class="info-value">{status.version || 'v4.0'}</span>
        </div>
        <div class="info-row">
          <span class="info-label">Free Heap</span>
          <span class="info-value">{status.freeHeap ? (status.freeHeap / 1024).toFixed(1) + ' KB' : '--'}</span>
        </div>
        <div class="info-row">
          <span class="info-label">Uptime</span>
          <span class="info-value">{status.uptime ? (status.uptime / 3600).toFixed(1) + ' hours' : '--'}</span>
        </div>
      </div>
    </div>
  </div>
</div>

<style>
  .config-page { display: flex; flex-direction: column; gap: 1.5rem; }
  .page-header { display: flex; justify-content: space-between; align-items: center; }
  .page-header h2 { display: flex; align-items: center; gap: 0.75rem; font-family: var(--font-display); font-size: 1.5rem; color: var(--primary); text-transform: uppercase; }
  
  .btn { display: flex; align-items: center; gap: 0.5rem; padding: 0.6rem 1rem; border: 1px solid var(--primary); border-radius: 0.5rem; background: rgba(0, 255, 255, 0.1); color: var(--primary); font-family: var(--font-display); font-size: 0.75rem; font-weight: 600; text-transform: uppercase; cursor: pointer; transition: all 0.3s ease; }
  .btn:hover { background: var(--primary); color: var(--bg-primary); }
  
  .config-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 1.25rem; }
  .config-card { background: var(--glass-bg); border: 1px solid var(--glass-border); border-radius: 1rem; overflow: hidden; }
  .card-header { padding: 1rem; background: rgba(255,255,255,0.03); border-bottom: 1px solid var(--glass-border); }
  .card-header h3 { display: flex; align-items: center; gap: 0.5rem; font-family: var(--font-display); font-size: 1rem; color: var(--primary); text-transform: uppercase; }
  .card-body { padding: 1.25rem; display: flex; flex-direction: column; gap: 1rem; }
  
  .form-group { display: flex; flex-direction: column; gap: 0.5rem; }
  .form-group label { font-size: 0.85rem; color: var(--text-secondary); }
  .form-group input[type="text"], .form-group input[type="number"] { padding: 0.75rem; background: rgba(255,255,255,0.05); border: 1px solid var(--glass-border); border-radius: 0.5rem; color: var(--text-primary); font-size: 0.95rem; }
  .form-group input:focus { outline: none; border-color: var(--primary); }
  .form-group.checkbox { flex-direction: row; align-items: center; }
  .form-group.checkbox label { display: flex; align-items: center; gap: 0.75rem; cursor: pointer; }
  .form-group.checkbox input[type="checkbox"] { width: 20px; height: 20px; accent-color: var(--primary); }
  
  .info-row { display: flex; justify-content: space-between; padding: 0.5rem 0; border-bottom: 1px solid var(--glass-border); }
  .info-row:last-child { border-bottom: none; }
  .info-label { font-size: 0.85rem; color: var(--text-muted); }
  .info-value { font-family: var(--font-mono); font-size: 0.9rem; color: var(--primary); }
</style>
