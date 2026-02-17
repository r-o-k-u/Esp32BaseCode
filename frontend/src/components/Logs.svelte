<script>
  import { activityLog } from '../stores/index.js';
  
  let logs = [];
  let filterType = 'all';
  
  activityLog.subscribe(value => logs = value);
  
  $: filteredLogs = filterType === 'all' ? logs : logs.filter(l => l.type === filterType);
  
  function clearLogs() {
    activityLog.set([]);
  }
  
  function exportLogs() {
    const data = JSON.stringify(logs, null, 2);
    const blob = new Blob([data], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `esp32_logs_${new Date().toISOString().slice(0,10)}.json`;
    a.click();
  }
</script>

<div class="logs-page">
  <div class="page-header">
    <h2><i class="fas fa-clipboard-list"></i> System Logs</h2>
    <div class="header-actions">
      <select bind:value={filterType}>
        <option value="all">All</option>
        <option value="info">Info</option>
        <option value="success">Success</option>
        <option value="warning">Warning</option>
        <option value="error">Error</option>
      </select>
      <button class="btn btn-secondary" on:click={exportLogs}>
        <i class="fas fa-download"></i> Export
      </button>
      <button class="btn btn-danger" on:click={clearLogs}>
        <i class="fas fa-trash"></i> Clear
      </button>
    </div>
  </div>
  
  <div class="logs-container">
    {#if filteredLogs.length === 0}
      <div class="empty-state">
        <i class="fas fa-inbox"></i>
        <p>No log entries</p>
      </div>
    {:else}
      {#each filteredLogs as log}
        <div class="log-entry {log.type}">
          <div class="log-icon">
            {#if log.type === 'success'}
              <i class="fas fa-check-circle"></i>
            {:else if log.type === 'warning'}
              <i class="fas fa-exclamation-triangle"></i>
            {:else if log.type === 'error'}
              <i class="fas fa-times-circle"></i>
            {:else}
              <i class="fas fa-info-circle"></i>
            {/if}
          </div>
          <div class="log-content">
            <span class="log-message">{log.message}</span>
            <span class="log-time">{log.timestamp}</span>
          </div>
        </div>
      {/each}
    {/if}
  </div>
</div>

<style>
  .logs-page { display: flex; flex-direction: column; gap: 1.5rem; }
  .page-header { display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 1rem; }
  .page-header h2 { display: flex; align-items: center; gap: 0.75rem; font-family: var(--font-display); font-size: 1.5rem; color: var(--primary); }
  .header-actions { display: flex; gap: 0.75rem; align-items: center; }
  .header-actions select { padding: 0.5rem 1rem; background: rgba(255,255,255,0.05); border: 1px solid var(--glass-border); border-radius: 0.5rem; color: var(--text-primary); }
  
  .btn { display: flex; align-items: center; gap: 0.5rem; padding: 0.5rem 1rem; border: 1px solid; border-radius: 0.5rem; font-family: var(--font-display); font-size: 0.75rem; font-weight: 600; text-transform: uppercase; cursor: pointer; transition: all 0.3s ease; }
  .btn-secondary { background: rgba(255,255,255,0.05); border-color: var(--glass-border); color: var(--text-secondary); }
  .btn-secondary:hover { background: rgba(255,255,255,0.1); color: var(--text-primary); }
  .btn-danger { background: rgba(255,0,85,0.1); border-color: var(--danger); color: var(--danger); }
  .btn-danger:hover { background: var(--danger); color: white; }
  
  .logs-container { background: var(--glass-bg); border: 1px solid var(--glass-border); border-radius: 1rem; padding: 1rem; max-height: 500px; overflow-y: auto; display: flex; flex-direction: column; gap: 0.5rem; }
  .empty-state { display: flex; flex-direction: column; align-items: center; gap: 0.5rem; padding: 3rem; color: var(--text-muted); }
  .empty-state i { font-size: 3rem; }
  
  .log-entry { display: flex; align-items: center; gap: 0.75rem; padding: 0.75rem; background: rgba(255,255,255,0.02); border-radius: 0.5rem; border-left: 3px solid var(--info); }
  .log-entry.success { border-left-color: var(--success); }
  .log-entry.warning { border-left-color: var(--warning); }
  .log-entry.error { border-left-color: var(--danger); }
  
  .log-icon { font-size: 1rem; }
  .log-entry.success .log-icon { color: var(--success); }
  .log-entry.warning .log-icon { color: var(--warning); }
  .log-entry.error .log-icon { color: var(--danger); }
  .log-entry:not(.success):not(.warning):not(.error) .log-icon { color: var(--info); }
  
  .log-content { flex: 1; display: flex; justify-content: space-between; align-items: center; }
  .log-message { font-size: 0.9rem; }
  .log-time { font-family: var(--font-mono); font-size: 0.75rem; color: var(--text-muted); }
</style>
