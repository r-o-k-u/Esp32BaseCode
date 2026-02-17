<script>
  import { otaStatus, showToast } from '../stores/index.js';
  import { uploadFirmware } from '../services/api.js';
  
  let ota = {};
  let selectedFile = null;
  
  otaStatus.subscribe(value => ota = value);
  
  function handleFileSelect(event) {
    selectedFile = event.target.files[0];
  }
  
  async function handleUpload() {
    if (!selectedFile) {
      showToast('Please select a firmware file', 'warning');
      return;
    }
    
    if (!selectedFile.name.endsWith('.bin')) {
      showToast('Please select a .bin firmware file', 'warning');
      return;
    }
    
    try {
      await uploadFirmware(selectedFile, () => {});
    } catch (error) {
      showToast('Firmware upload failed', 'error');
    }
  }
</script>

<div class="ota-page">
  <div class="page-header">
    <h2><i class="fas fa-upload"></i> OTA Updates</h2>
  </div>
  
  <div class="status-card">
    <div class="card-header">
      <h3>OTA Status</h3>
      <div class="status-badge" class:ready={ota.initialized}>
        {ota.initialized ? 'Ready' : 'Not Initialized'}
      </div>
    </div>
    <div class="status-grid">
      <div class="status-item">
        <span class="label">Hostname</span>
        <span class="value">{ota.hostname || 'esp32'}</span>
      </div>
      <div class="status-item">
        <span class="label">Port</span>
        <span class="value">{ota.port || 3232}</span>
      </div>
      <div class="status-item">
        <span class="label">Total Updates</span>
        <span class="value">{ota.totalUpdates || 0}</span>
      </div>
    </div>
  </div>
  
  <div class="upload-card">
    <div class="card-header">
      <h3>Upload Firmware</h3>
    </div>
    <div class="upload-content">
      <input type="file" accept=".bin" on:change={handleFileSelect}>
      {#if selectedFile}
        <div class="file-info">
          <i class="fas fa-file"></i>
          <span>{selectedFile.name}</span>
        </div>
      {/if}
      
      {#if ota.updating}
        <div class="progress-bar">
          <div class="progress-fill" style="width: {ota.progress || 0}%"></div>
        </div>
        <span>{ota.progress || 0}%</span>
      {/if}
      
      <button class="btn btn-success" on:click={handleUpload} disabled={ota.updating || !selectedFile}>
        <i class="fas fa-upload"></i>
        {ota.updating ? 'Uploading...' : 'Upload & Update'}
      </button>
    </div>
  </div>
</div>

<style>
  .ota-page { display: flex; flex-direction: column; gap: 1.5rem; }
  .page-header h2 { display: flex; align-items: center; gap: 0.75rem; font-family: var(--font-display); font-size: 1.5rem; color: var(--primary); }
  
  .status-card, .upload-card { background: var(--glass-bg); border: 1px solid var(--glass-border); border-radius: 1rem; overflow: hidden; }
  .card-header { display: flex; justify-content: space-between; align-items: center; padding: 1rem; background: rgba(255,255,255,0.03); border-bottom: 1px solid var(--glass-border); }
  .card-header h3 { font-family: var(--font-display); font-size: 1rem; color: var(--primary); }
  
  .status-badge { font-size: 0.8rem; padding: 0.25rem 0.75rem; background: rgba(255,0,85,0.1); border: 1px solid var(--danger); border-radius: 1rem; color: var(--danger); }
  .status-badge.ready { background: rgba(0,255,136,0.1); border-color: var(--success); color: var(--success); }
  
  .status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 1rem; padding: 1.25rem; }
  .status-item { display: flex; flex-direction: column; }
  .status-item .label { font-size: 0.7rem; color: var(--text-muted); text-transform: uppercase; }
  .status-item .value { font-family: var(--font-mono); font-size: 1rem; color: var(--primary); }
  
  .upload-content { padding: 1.25rem; display: flex; flex-direction: column; gap: 1rem; }
  .upload-content input[type="file"] { padding: 0.5rem; background: rgba(255,255,255,0.05); border: 1px dashed var(--glass-border); border-radius: 0.5rem; color: var(--text-primary); }
  .file-info { display: flex; align-items: center; gap: 0.5rem; padding: 0.5rem; background: rgba(0,255,136,0.1); border-radius: 0.5rem; }
  
  .progress-bar { height: 20px; background: rgba(255,255,255,0.1); border-radius: 10px; overflow: hidden; }
  .progress-fill { height: 100%; background: var(--primary); transition: width 0.3s; }
  
  .btn { display: flex; align-items: center; justify-content: center; gap: 0.5rem; padding: 0.75rem 1.5rem; border: 1px solid; border-radius: 0.5rem; font-family: var(--font-display); font-size: 0.85rem; cursor: pointer; }
  .btn:disabled { opacity: 0.5; cursor: not-allowed; }
  .btn-success { background: rgba(0,255,136,0.1); border-color: var(--success); color: var(--success); }
  .btn-success:hover:not(:disabled) { background: var(--success); color: var(--bg-primary); }
</style>
