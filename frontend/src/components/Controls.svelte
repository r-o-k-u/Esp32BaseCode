<script>
  import { 
    refreshPeers, 
    scanWiFiNetworks, 
    emergencyStop, 
    resetAllActuators 
  } from '../services/api.js';
  import { showToast, addActivity, systemStatus } from '../stores/index.js';
  
  let restarting = false;
  
  async function handleRefreshAll() {
    showToast('Refreshing all data...', 'info');
    addActivity('Full system refresh initiated', 'info');
    // Trigger WebSocket refresh
    window.location.reload();
  }
  
  async function handleScanPeers() {
    await refreshPeers();
  }
  
  async function handleScanWiFi() {
    await scanWiFiNetworks();
  }
  
  function handleEmergencyStop() {
    if (confirm('⚠️ EMERGENCY STOP: This will immediately stop all actuators. Continue?')) {
      emergencyStop();
    }
  }
  
  async function handleRestart() {
    if (confirm('Are you sure you want to restart the device?')) {
      restarting = true;
      showToast('Device restarting...', 'warning');
      addActivity('Device restart initiated', 'warning');
      
      try {
        const response = await fetch('/api/restart', { method: 'POST' });
        if (response.ok) {
          setTimeout(() => {
            window.location.reload();
          }, 5000);
        }
      } catch (error) {
        showToast('Failed to restart device', 'error');
        restarting = false;
      }
    }
  }
</script>

<div class="controls">
  <div class="control-group">
    <button class="btn btn-primary" on:click={handleRefreshAll}>
      <i class="fas fa-sync-alt"></i>
      <span>Refresh All</span>
    </button>
    
    <button class="btn btn-success" on:click={handleScanPeers}>
      <i class="fas fa-search"></i>
      <span>Scan ESP-NOW</span>
    </button>
    
    <button class="btn btn-info" on:click={handleScanWiFi}>
      <i class="fas fa-wifi"></i>
      <span>Scan WiFi</span>
    </button>
    
    <button class="btn btn-danger" on:click={handleEmergencyStop}>
      <i class="fas fa-skull-crossbones"></i>
      <span>Emergency Stop</span>
    </button>
  </div>
  
  <div class="control-group">
    <button class="btn btn-secondary" on:click={handleRestart} disabled={restarting}>
      <i class="fas fa-redo" class:spinning={restarting}></i>
      <span>{restarting ? 'Restarting...' : 'Restart'}</span>
    </button>
  </div>
</div>

<style>
  .controls {
    display: flex;
    justify-content: space-between;
    align-items: center;
    gap: 1rem;
    padding: 1rem 2rem;
    background: linear-gradient(90deg, rgba(26, 26, 46, 0.9), rgba(13, 13, 26, 0.9));
    border-bottom: 1px solid var(--glass-border);
    flex-wrap: wrap;
  }
  
  .control-group {
    display: flex;
    gap: 0.75rem;
    flex-wrap: wrap;
  }
  
  .btn {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.6rem 1.25rem;
    border: 1px solid;
    border-radius: 0.5rem;
    font-family: var(--font-display);
    font-size: 0.75rem;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    cursor: pointer;
    transition: all 0.3s ease;
  }
  
  .btn:disabled {
    opacity: 0.6;
    cursor: not-allowed;
  }
  
  .btn-primary {
    background: rgba(0, 255, 255, 0.1);
    border-color: var(--primary);
    color: var(--primary);
  }
  
  .btn-primary:hover:not(:disabled) {
    background: var(--primary);
    color: var(--bg-primary);
    box-shadow: 0 0 20px rgba(0, 255, 255, 0.5);
    transform: translateY(-2px);
  }
  
  .btn-success {
    background: rgba(0, 255, 136, 0.1);
    border-color: var(--success);
    color: var(--success);
  }
  
  .btn-success:hover:not(:disabled) {
    background: var(--success);
    color: var(--bg-primary);
    box-shadow: 0 0 20px rgba(0, 255, 136, 0.5);
    transform: translateY(-2px);
  }
  
  .btn-info {
    background: rgba(0, 170, 255, 0.1);
    border-color: var(--info);
    color: var(--info);
  }
  
  .btn-info:hover:not(:disabled) {
    background: var(--info);
    color: var(--bg-primary);
    box-shadow: 0 0 20px rgba(0, 170, 255, 0.5);
    transform: translateY(-2px);
  }
  
  .btn-danger {
    background: rgba(255, 0, 85, 0.1);
    border-color: var(--danger);
    color: var(--danger);
  }
  
  .btn-danger:hover:not(:disabled) {
    background: var(--danger);
    color: white;
    box-shadow: 0 0 20px rgba(255, 0, 85, 0.5);
    transform: translateY(-2px);
  }
  
  .btn-secondary {
    background: rgba(255, 255, 255, 0.05);
    border-color: var(--glass-border);
    color: var(--text-secondary);
  }
  
  .btn-secondary:hover:not(:disabled) {
    background: rgba(255, 255, 255, 0.1);
    color: var(--text-primary);
    border-color: var(--primary);
    transform: translateY(-2px);
  }
  
  .spinning {
    animation: spin 1s linear infinite;
  }
  
  @keyframes spin {
    from { transform: rotate(0deg); }
    to { transform: rotate(360deg); }
  }
  
  @media (max-width: 768px) {
    .controls {
      flex-direction: column;
      align-items: stretch;
    }
    
    .control-group {
      justify-content: center;
    }
    
    .btn {
      flex: 1;
      justify-content: center;
      min-width: 120px;
    }
  }
</style>
