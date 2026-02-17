<script>
  import { createEventDispatcher } from 'svelte';
  
  export let activeTab = 'dashboard';
  
  const dispatch = createEventDispatcher();
  
  const tabs = [
    { id: 'dashboard', label: 'Dashboard', icon: 'fa-tachometer-alt' },
    { id: 'sensors', label: 'Sensors', icon: 'fa-thermometer-half' },
    { id: 'actuators', label: 'Actuators', icon: 'fa-cogs' },
    { id: 'communication', label: 'Communication', icon: 'fa-satellite-dish' },
    { id: 'wifi', label: 'WiFi Manager', icon: 'fa-wifi' },
    { id: 'ota', label: 'OTA Updates', icon: 'fa-upload' },
    { id: 'logs', label: 'Logs', icon: 'fa-clipboard-list' },
    { id: 'config', label: 'Configuration', icon: 'fa-sliders-h' }
  ];
  
  function selectTab(tabId) {
    dispatch('tabChange', tabId);
  }
</script>

<nav class="navigation">
  <div class="nav-container">
    {#each tabs as tab}
      <button 
        class="nav-item"
        class:active={activeTab === tab.id}
        on:click={() => selectTab(tab.id)}
      >
        <i class="fas {tab.icon}"></i>
        <span>{tab.label}</span>
      </button>
    {/each}
  </div>
</nav>

<style>
  .navigation {
    background: var(--glass-bg);
    backdrop-filter: blur(10px);
    border-bottom: 1px solid var(--glass-border);
    padding: 0.75rem 1rem;
    overflow-x: auto;
  }
  
  .nav-container {
    display: flex;
    gap: 0.5rem;
    max-width: 1400px;
    margin: 0 auto;
    justify-content: center;
    flex-wrap: wrap;
  }
  
  .nav-item {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.75rem 1.25rem;
    background: transparent;
    border: 1px solid transparent;
    border-radius: 2rem;
    color: var(--text-secondary);
    font-family: var(--font-display);
    font-size: 0.8rem;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    cursor: pointer;
    transition: all 0.3s ease;
    white-space: nowrap;
  }
  
  .nav-item:hover {
    color: var(--primary);
    background: rgba(0, 255, 255, 0.05);
    border-color: var(--primary);
    transform: translateY(-2px);
    box-shadow: 0 5px 15px rgba(0, 255, 255, 0.2);
  }
  
  .nav-item.active {
    color: var(--primary);
    background: linear-gradient(135deg, rgba(0, 255, 255, 0.15), rgba(255, 0, 255, 0.1));
    border: 1px solid var(--primary);
    box-shadow: 0 0 20px rgba(0, 255, 255, 0.3), inset 0 0 20px rgba(0, 255, 255, 0.1);
  }
  
  .nav-item i {
    font-size: 1rem;
  }
  
  .nav-item.active i {
    animation: iconPulse 2s ease-in-out infinite;
  }
  
  @keyframes iconPulse {
    0%, 100% { transform: scale(1); }
    50% { transform: scale(1.1); }
  }
  
  @media (max-width: 768px) {
    .nav-container {
      justify-content: flex-start;
      padding-bottom: 0.5rem;
    }
    
    .nav-item {
      padding: 0.5rem 1rem;
      font-size: 0.7rem;
    }
    
    .nav-item i {
      font-size: 0.9rem;
    }
  }
</style>
