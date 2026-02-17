<script>
  import { onMount } from 'svelte';
  import { espNowStatus, espNowPeers, messageLog, addActivity, showToast } from '../stores/index.js';
  import { refreshPeers } from '../services/api.js';
  
  let espnow = {};
  let peers = [];
  let messages = [];
  
  espNowStatus.subscribe(value => espnow = value);
  espNowPeers.subscribe(value => peers = value);
  messageLog.subscribe(value => messages = value.slice(0, 50));
  
  async function handleRefreshPeers() {
    await refreshPeers();
  }
</script>

<div class="comm-page">
  <div class="page-header">
    <h2><i class="fas fa-satellite-dish"></i> ESP-NOW Communication</h2>
    <button class="btn btn-primary" on:click={handleRefreshPeers}>
      <i class="fas fa-sync-alt"></i> Refresh Peers
    </button>
  </div>
  
  <!-- Stats -->
  <div class="stats-grid">
    <div class="stat-card">
      <div class="stat-header">ESP-NOW Status</div>
      <div class="stat-value" class:online={espnow.peers > 0}>{espnow.peers || 0} Peers</div>
    </div>
    <div class="stat-card">
      <div class="stat-header">Messages Sent</div>
      <div class="stat-value">{espnow.sent || 0}</div>
    </div>
    <div class="stat-card">
      <div class="stat-header">Messages Received</div>
      <div class="stat-value">{espnow.received || 0}</div>
    </div>
    <div class="stat-card">
      <div class="stat-header">Failed</div>
      <div class="stat-value error">{espnow.failed || 0}</div>
    </div>
  </div>
  
  <!-- Peers List -->
  <div class="card">
    <div class="card-header">
      <h3><i class="fas fa-users"></i> Connected Peers</h3>
    </div>
    <div class="peers-list">
      {#if peers.length === 0}
        <p class="empty">No peers connected. ESP-NOW devices will appear here when discovered.</p>
      {:else}
        {#each peers as peer}
          <div class="peer-item">
            <div class="peer-info">
              <i class="fas fa-microchip"></i>
              <span class="peer-mac">{peer.mac}</span>
            </div>
            <div class="peer-status online">Connected</div>
          </div>
        {/each}
      {/if}
    </div>
  </div>
  
  <!-- Message Log -->
  <div class="card">
    <div class="card-header">
      <h3><i class="fas fa-envelope"></i> Message Log</h3>
    </div>
    <div class="message-list">
      {#if messages.length === 0}
        <p class="empty">No messages yet.</p>
      {:else}
        {#each messages as msg}
          <div class="message-item">
            <span class="msg-time">{msg.timestamp || '--:--:--'}</span>
            <span class="msg-data">{JSON.stringify(msg.data || {})}</span>
          </div>
        {/each}
      {/if}
    </div>
  </div>
</div>

<style>
  .comm-page { display: flex; flex-direction: column; gap: 1.5rem; }
  .page-header { display: flex; justify-content: space-between; align-items: center; }
  .page-header h2 { display: flex; align-items: center; gap: 0.75rem; font-family: var(--font-display); font-size: 1.5rem; color: var(--primary); text-transform: uppercase; }
  
  .btn { display: flex; align-items: center; gap: 0.5rem; padding: 0.6rem 1rem; border: 1px solid var(--primary); border-radius: 0.5rem; background: rgba(0, 255, 255, 0.1); color: var(--primary); font-family: var(--font-display); font-size: 0.75rem; font-weight: 600; text-transform: uppercase; cursor: pointer; transition: all 0.3s ease; }
  .btn:hover { background: var(--primary); color: var(--bg-primary); }
  
  .stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 1rem; }
  .stat-card { background: var(--glass-bg); backdrop-filter: blur(10px); border: 1px solid var(--glass-border); border-radius: 1rem; padding: 1.25rem; }
  .stat-header { font-size: 0.8rem; color: var(--text-muted); text-transform: uppercase; margin-bottom: 0.5rem; }
  .stat-value { font-family: var(--font-mono); font-size: 1.8rem; font-weight: 700; color: var(--primary); }
  .stat-value.online { color: var(--success); }
  .stat-value.error { color: var(--danger); }
  
  .card { background: var(--glass-bg); backdrop-filter: blur(10px); border: 1px solid var(--glass-border); border-radius: 1rem; overflow: hidden; }
  .card-header { padding: 1rem; background: rgba(255, 255, 255, 0.03); border-bottom: 1px solid var(--glass-border); }
  .card-header h3 { display: flex; align-items: center; gap: 0.5rem; font-family: var(--font-display); font-size: 1rem; color: var(--primary); text-transform: uppercase; }
  
  .peers-list, .message-list { padding: 1rem; max-height: 250px; overflow-y: auto; }
  .empty { text-align: center; color: var(--text-muted); padding: 2rem; }
  
  .peer-item { display: flex; justify-content: space-between; align-items: center; padding: 0.75rem; background: rgba(255, 255, 255, 0.03); border: 1px solid var(--glass-border); border-radius: 0.5rem; margin-bottom: 0.5rem; }
  .peer-info { display: flex; align-items: center; gap: 0.5rem; }
  .peer-mac { font-family: var(--font-mono); font-size: 0.9rem; }
  .peer-status { font-size: 0.8rem; }
  .peer-status.online { color: var(--success); }
  
  .message-item { display: flex; gap: 1rem; padding: 0.5rem; border-bottom: 1px solid var(--glass-border); }
  .msg-time { font-family: var(--font-mono); font-size: 0.75rem; color: var(--text-muted); }
  .msg-data { font-family: var(--font-mono); font-size: 0.85rem; color: var(--text-primary); }
</style>
