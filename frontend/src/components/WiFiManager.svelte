<script>
  import { onMount } from 'svelte';
  import { wifiStatus, wifiNetworks, addActivity, showToast } from '../stores/index.js';
  import { fetchWiFiStatus, scanWiFiNetworks, connectToWiFi, disconnectWiFi, startAccessPoint, stopAccessPoint } from '../services/api.js';
  
  let wifi = {};
  let networks = [];
  let ssid = '';
  let password = '';
  let apSSID = 'ESP32-AP';
  let apPassword = '';
  
  wifiStatus.subscribe(value => wifi = value);
  wifiNetworks.subscribe(value => networks = value);
  
  onMount(async () => {
    await fetchWiFiStatus();
    await scanWiFiNetworks();
  });
  
  async function handleConnect() {
    if (!ssid) {
      showToast('Please enter SSID', 'warning');
      return;
    }
    await connectToWiFi(ssid, password);
  }
  
  async function handleDisconnect() {
    await disconnectWiFi();
  }
  
  async function handleScan() {
    await scanWiFiNetworks();
  }
  
  async function handleStartAP() {
    await startAccessPoint(apSSID, apPassword);
  }
  
  async function handleStopAP() {
    await stopAccessPoint();
  }
  
  function selectNetwork(network) {
    ssid = network.ssid;
  }
</script>

<div class="wifi-page">
  <div class="page-header">
    <h2><i class="fas fa-wifi"></i> WiFi Manager</h2>
    <button class="btn btn-primary" on:click={handleScan}>
      <i class="fas fa-sync-alt"></i> Scan Networks
    </button>
  </div>
  
  <!-- Connection Status -->
  <div class="status-card">
    <div class="card-header">
      <h3><i class="fas fa-plug"></i> Current Connection</h3>
      <div class="connection-status" class:connected={wifi.connected}>
        <span class="status-dot"></span>
        {wifi.connected ? 'Connected' : 'Disconnected'}
      </div>
    </div>
    <div class="status-grid">
      <div class="status-item">
        <span class="label">SSID</span>
        <span class="value">{wifi.ssid || '---'}</span>
      </div>
      <div class="status-item">
        <span class="label">IP Address</span>
        <span class="value">{wifi.ip || '---'}</span>
      </div>
      <div class="status-item">
        <span class="label">Signal</span>
        <span class="value">{wifi.rssi || 0} dBm</span>
      </div>
      <div class="status-item">
        <span class="label">MAC</span>
        <span class="value">{wifi.mac || '---'}</span>
      </div>
    </div>
    {#if wifi.connected}
      <button class="btn btn-danger" on:click={handleDisconnect}>
        <i class="fas fa-unlink"></i> Disconnect
      </button>
    {/if}
  </div>
  
  <!-- Network List -->
  <div class="networks-card">
    <div class="card-header">
      <h3><i class="fas fa-list"></i> Available Networks</h3>
    </div>
    <div class="network-list">
      {#if networks.length === 0}
        <p class="empty">No networks found. Click "Scan Networks" to search.</p>
      {:else}
        {#each networks as network}
          <div class="network-item" on:click={() => selectNetwork(network)}>
            <div class="network-info">
              <i class="fas fa-wifi"></i>
              <span class="ssid">{network.ssid}</span>
            </div>
            <div class="network-details">
              <span class="signal">{network.rssi} dBm</span>
              <span class="channel">Ch {network.channel}</span>
            </div>
          </div>
        {/each}
      {/if}
    </div>
  </div>
  
  <!-- Connect Form -->
  <div class="connect-card">
    <div class="card-header">
      <h3><i class="fas fa-sign-in-alt"></i> Connect to Network</h3>
    </div>
    <div class="form-group">
      <label>SSID</label>
      <input type="text" bind:value={ssid} placeholder="Enter network SSID">
    </div>
    <div class="form-group">
      <label>Password</label>
      <input type="password" bind:value={password} placeholder="Enter password">
    </div>
    <button class="btn btn-success" on:click={handleConnect}>
      <i class="fas fa-plug"></i> Connect
    </button>
  </div>
  
  <!-- Access Point -->
  <div class="ap-card">
    <div class="card-header">
      <h3><i class="fas fa-broadcast-tower"></i> Access Point Mode</h3>
      <div class="ap-status" class:connected={wifi.apMode}>
        {wifi.apMode ? 'Active' : 'Inactive'}
      </div>
    </div>
    <div class="form-group">
      <label>AP SSID</label>
      <input type="text" bind:value={apSSID} placeholder="ESP32-AP">
    </div>
    <div class="form-group">
      <label>Password (min 8 chars)</label>
      <input type="password" bind:value={apPassword} placeholder="Leave empty for open">
    </div>
    <div class="ap-buttons">
      {#if wifi.apMode}
        <button class="btn btn-danger" on:click={handleStopAP}>
          <i class="fas fa-stop"></i> Stop AP
        </button>
      {:else}
        <button class="btn btn-success" on:click={handleStartAP}>
          <i class="fas fa-play"></i> Start AP
        </button>
      {/if}
    </div>
  </div>
</div>

<style>
  .wifi-page { display: flex; flex-direction: column; gap: 1.5rem; }
  .page-header { display: flex; justify-content: space-between; align-items: center; }
  .page-header h2 { display: flex; align-items: center; gap: 0.75rem; font-family: var(--font-display); font-size: 1.5rem; color: var(--primary); text-transform: uppercase; }
  
  .btn { display: flex; align-items: center; gap: 0.5rem; padding: 0.6rem 1rem; border: 1px solid; border-radius: 0.5rem; font-family: var(--font-display); font-size: 0.75rem; font-weight: 600; text-transform: uppercase; cursor: pointer; transition: all 0.3s ease; }
  .btn-primary { background: rgba(0, 255, 255, 0.1); border-color: var(--primary); color: var(--primary); }
  .btn-primary:hover { background: var(--primary); color: var(--bg-primary); }
  .btn-success { background: rgba(0, 255, 136, 0.1); border-color: var(--success); color: var(--success); }
  .btn-success:hover { background: var(--success); color: var(--bg-primary); }
  .btn-danger { background: rgba(255, 0, 85, 0.1); border-color: var(--danger); color: var(--danger); }
  .btn-danger:hover { background: var(--danger); color: white; }
  
  .status-card, .networks-card, .connect-card, .ap-card { background: var(--glass-bg); backdrop-filter: blur(10px); border: 1px solid var(--glass-border); border-radius: 1rem; overflow: hidden; }
  .card-header { display: flex; justify-content: space-between; align-items: center; padding: 1rem; background: rgba(255, 255, 255, 0.03); border-bottom: 1px solid var(--glass-border); }
  .card-header h3 { display: flex; align-items: center; gap: 0.5rem; font-family: var(--font-display); font-size: 1rem; color: var(--primary); text-transform: uppercase; }
  
  .connection-status, .ap-status { display: flex; align-items: center; gap: 0.5rem; font-size: 0.85rem; color: var(--text-muted); }
  .connection-status.connected, .ap-status.connected { color: var(--success); }
  .status-dot { width: 8px; height: 8px; border-radius: 50%; background: currentColor; }
  
  .status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 1rem; padding: 1.25rem; }
  .status-item { display: flex; flex-direction: column; gap: 0.25rem; }
  .status-item .label { font-size: 0.7rem; color: var(--text-muted); text-transform: uppercase; }
  .status-item .value { font-family: var(--font-mono); font-size: 0.95rem; color: var(--primary); }
  
  .status-card .btn { margin: 0 1rem 1rem; }
  
  .network-list { padding: 1rem; max-height: 250px; overflow-y: auto; }
  .network-list .empty { text-align: center; color: var(--text-muted); padding: 2rem; }
  .network-item { display: flex; justify-content: space-between; align-items: center; padding: 0.75rem; background: rgba(255, 255, 255, 0.03); border: 1px solid var(--glass-border); border-radius: 0.5rem; margin-bottom: 0.5rem; cursor: pointer; transition: all 0.3s ease; }
  .network-item:hover { background: rgba(0, 255, 255, 0.05); border-color: var(--primary); }
  .network-info { display: flex; align-items: center; gap: 0.5rem; }
  .network-info .ssid { font-weight: 600; }
  .network-details { display: flex; gap: 1rem; font-size: 0.8rem; color: var(--text-muted); }
  
  .connect-card, .ap-card { padding: 1.25rem; }
  .form-group { margin-bottom: 1rem; }
  .form-group label { display: block; margin-bottom: 0.5rem; font-size: 0.85rem; color: var(--text-secondary); }
  .form-group input { width: 100%; padding: 0.75rem; background: rgba(255, 255, 255, 0.05); border: 1px solid var(--glass-border); border-radius: 0.5rem; color: var(--text-primary); font-size: 0.95rem; }
  .form-group input:focus { outline: none; border-color: var(--primary); }
  
  .ap-buttons { display: flex; gap: 0.5rem; }
</style>
