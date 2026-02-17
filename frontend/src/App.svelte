<script>
  import { onMount, onDestroy } from 'svelte';
  import { currentTab, connectionStatus, toasts, activityLog } from './stores/index.js';
  import { initWebSocket, disconnectWebSocket } from './services/api.js';
  
  // Components
  import Header from './components/Header.svelte';
  import Navigation from './components/Navigation.svelte';
  import Controls from './components/Controls.svelte';
  import Dashboard from './components/Dashboard.svelte';
  import Sensors from './components/Sensors.svelte';
  import Actuators from './components/Actuators.svelte';
  import Communication from './components/Communication.svelte';
  import WiFiManager from './components/WiFiManager.svelte';
  import OTAUpdate from './components/OTAUpdate.svelte';
  import Logs from './components/Logs.svelte';
  import Configuration from './components/Configuration.svelte';
  import ToastContainer from './components/ToastContainer.svelte';
  
  let activeTab = 'dashboard';
  let connected = false;
  let clockInterval;
  let currentTime = new Date().toLocaleTimeString();
  
  // Subscribe to stores
  currentTab.subscribe(value => {
    activeTab = value;
  });
  
  connectionStatus.subscribe(status => {
    connected = status.connected;
  });
  
  onMount(() => {
    console.log('🚀 ESP32 Control Panel v4.0 Svelte initializing...');
    
    // Initialize WebSocket connection
    initWebSocket();
    
    // Update clock
    clockInterval = setInterval(() => {
      currentTime = new Date().toLocaleTimeString();
    }, 1000);
    
    console.log('✅ App initialized');
  });
  
  onDestroy(() => {
    if (clockInterval) {
      clearInterval(clockInterval);
    }
    disconnectWebSocket();
  });
  
  function handleTabChange(event) {
    const tab = event.detail;
    currentTab.set(tab);
  }
</script>

<main class="app">
  <Header {currentTime} {connected} />
  
  <Navigation {activeTab} on:tabChange={handleTabChange} />
  
  <Controls />
  
  <div class="content">
    {#if activeTab === 'dashboard'}
      <Dashboard />
    {:else if activeTab === 'sensors'}
      <Sensors />
    {:else if activeTab === 'actuators'}
      <Actuators />
    {:else if activeTab === 'communication'}
      <Communication />
    {:else if activeTab === 'wifi'}
      <WiFiManager />
    {:else if activeTab === 'ota'}
      <OTAUpdate />
    {:else if activeTab === 'logs'}
      <Logs />
    {:else if activeTab === 'config'}
      <Configuration />
    {/if}
  </div>
  
  <footer class="footer">
    <div class="footer-content">
      <span>ESP32 Control Panel v4.0</span>
      <span class="separator">|</span>
      <span>{currentTime}</span>
      <span class="separator">|</span>
      <span class:connected>
        <i class="fas fa-circle"></i>
        {connected ? 'Connected' : 'Disconnected'}
      </span>
    </div>
  </footer>
  
  <ToastContainer />
</main>

<style>
  :global(*) {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
  }
  
  :global(body) {
    font-family: 'Rajdhani', sans-serif;
    background: linear-gradient(135deg, #0D0D1A 0%, #1A1A2E 50%, #16213E 100%);
    color: #FFFFFF;
    min-height: 100vh;
    overflow-x: hidden;
  }
  
  :global(:root) {
    --primary: #00FFFF;
    --primary-dark: #00CCCC;
    --primary-light: #66FFFF;
    --primary-glow: rgba(0, 255, 255, 0.4);
    --secondary: #FF00FF;
    --success: #00FF88;
    --warning: #FFAA00;
    --danger: #FF0055;
    --info: #00AAFF;
    --bg-primary: #0D0D1A;
    --bg-secondary: #1A1A2E;
    --bg-tertiary: #16213E;
    --glass-bg: rgba(26, 26, 46, 0.7);
    --glass-border: rgba(0, 255, 255, 0.2);
    --text-primary: #FFFFFF;
    --text-secondary: #B8C5D6;
    --text-muted: #7A8AA3;
    --font-display: 'Orbitron', monospace;
    --font-mono: 'JetBrains Mono', monospace;
    --font-body: 'Rajdhani', sans-serif;
  }
  
  .app {
    min-height: 100vh;
    display: flex;
    flex-direction: column;
  }
  
  .content {
    flex: 1;
    padding: 1.5rem;
    overflow-y: auto;
  }
  
  .footer {
    padding: 1rem 2rem;
    background: linear-gradient(90deg, rgba(13, 13, 26, 0.9), rgba(26, 26, 46, 0.9));
    border-top: 1px solid var(--glass-border);
  }
  
  .footer-content {
    display: flex;
    justify-content: center;
    align-items: center;
    gap: 1rem;
    font-family: var(--font-mono);
    font-size: 0.8rem;
    color: var(--text-muted);
  }
  
  .separator {
    opacity: 0.5;
  }
  
  .connected {
    color: var(--success);
    display: flex;
    align-items: center;
    gap: 0.5rem;
  }
  
  .connected i {
    font-size: 0.6rem;
    animation: pulse 2s infinite;
  }
  
  @keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.5; }
  }
  
  @media (max-width: 768px) {
    .content {
      padding: 1rem;
    }
    
    .footer-content {
      flex-wrap: wrap;
      text-align: center;
    }
  }
</style>
