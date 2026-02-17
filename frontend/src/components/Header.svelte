<script>
  export let currentTime = '';
  export let connected = false;
</script>

<header class="header">
  <div class="header-content">
    <div class="title-section">
      <h1>ESP32 DualComm System</h1>
      <h2>Control Center</h2>
    </div>
    <div class="status-section">
      <div class="status-indicator" class:connected>
        <span class="dot"></span>
        <span>{connected ? 'Connected' : 'Disconnected'}</span>
      </div>
    </div>
  </div>
</header>

<style>
  .header {
    background: linear-gradient(90deg, rgba(13, 13, 26, 0.95), rgba(26, 26, 46, 0.95));
    backdrop-filter: blur(20px);
    border-bottom: 1px solid var(--glass-border);
    padding: 1.5rem 2rem;
    position: sticky;
    top: 0;
    z-index: 100;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.5);
  }
  
  .header::after {
    content: '';
    position: absolute;
    bottom: 0;
    left: 0;
    right: 0;
    height: 2px;
    background: linear-gradient(90deg, transparent, var(--primary), var(--secondary), var(--primary), transparent);
  }
  
  .header-content {
    display: flex;
    justify-content: space-between;
    align-items: center;
    max-width: 1400px;
    margin: 0 auto;
  }
  
  .title-section h1 {
    font-family: var(--font-display);
    font-size: 1.8rem;
    font-weight: 900;
    background: linear-gradient(135deg, var(--primary) 0%, var(--secondary) 100%);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    text-transform: uppercase;
    letter-spacing: 0.1em;
    text-shadow: 0 0 30px rgba(0, 255, 255, 0.5);
  }
  
  .title-section h2 {
    font-size: 0.9rem;
    color: var(--text-secondary);
    text-transform: uppercase;
    letter-spacing: 0.05em;
    margin-top: 0.25rem;
  }
  
  .status-indicator {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.5rem 1rem;
    background: rgba(255, 0, 85, 0.1);
    border: 1px solid var(--danger);
    border-radius: 2rem;
    font-family: var(--font-mono);
    font-size: 0.8rem;
    color: var(--danger);
  }
  
  .status-indicator.connected {
    background: rgba(0, 255, 136, 0.1);
    border-color: var(--success);
    color: var(--success);
  }
  
  .dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: currentColor;
    animation: blink 2s infinite;
  }
  
  .status-indicator.connected .dot {
    animation: none;
    box-shadow: 0 0 10px var(--success);
  }
  
  @keyframes blink {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.3; }
  }
  
  @media (max-width: 768px) {
    .header-content {
      flex-direction: column;
      gap: 1rem;
    }
    
    .title-section h1 {
      font-size: 1.4rem;
      text-align: center;
    }
    
    .title-section h2 {
      text-align: center;
    }
  }
</style>
