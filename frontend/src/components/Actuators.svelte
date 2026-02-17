<script>
  import { actuatorStatus, showToast, addActivity } from '../stores/index.js';
  import { 
    toggleActuator, 
    setLEDBrightness, 
    setRGBColor, 
    triggerBuzzer,
    controlMotor,
    controlServo,
    toggleRelay,
    emergencyStop,
    resetAllActuators
  } from '../services/api.js';
  
  let actuators = {};
  let ledBrightness = 0;
  let rgbR = 255, rgbG = 0, rgbB = 0;
  let motorSpeed = 0;
  let servoAngle = 90;
  let buzzerFreq = 1000;
  let buzzerDur = 500;
  
  actuatorStatus.subscribe(value => actuators = value);
  
  function handleLEDToggle() {
    toggleActuator('led', !actuators.led?.on);
  }
  
  function handleLEDBrightness(e) {
    ledBrightness = parseInt(e.target.value);
    setLEDBrightness(ledBrightness);
  }
  
  function handleRGBChange() {
    setRGBColor(rgbR, rgbG, rgbB);
  }
  
  function handleMotorForward() {
    controlMotor('forward', motorSpeed);
  }
  
  function handleMotorBackward() {
    controlMotor('backward', motorSpeed);
  }
  
  function handleMotorStop() {
    controlMotor('stop');
    motorSpeed = 0;
  }
  
  function handleServoChange(e) {
    servoAngle = parseInt(e.target.value);
    controlServo(servoAngle);
  }
  
  function handleRelayToggle(num) {
    const newState = !actuators.relays?.[num - 1];
    toggleRelay(num, newState);
  }
  
  function handleBuzzerTest() {
    triggerBuzzer(buzzerDur, buzzerFreq);
  }
</script>

<div class="actuators-page">
  <div class="page-header">
    <h2><i class="fas fa-cogs"></i> Actuator Control</h2>
    <div class="header-actions">
      <button class="btn btn-danger" on:click={emergencyStop}>
        <i class="fas fa-skull-crossbones"></i> Emergency Stop
      </button>
      <button class="btn btn-warning" on:click={resetAllActuators}>
        <i class="fas fa-power-off"></i> Reset All
      </button>
    </div>
  </div>
  
  <div class="actuator-grid">
    <!-- LED Control -->
    <div class="actuator-card">
      <div class="actuator-header">
        <div class="actuator-title">
          <i class="fas fa-lightbulb" style="color: #FFCE56;"></i>
          <span>LED Control</span>
        </div>
        <div class="actuator-status" class:online={actuators.led?.on}>
          {actuators.led?.on ? 'ON' : 'OFF'}
        </div>
      </div>
      <div class="actuator-body">
        <div class="control-buttons">
          <button class="btn" class:btn-success={actuators.led?.on} on:click={handleLEDToggle}>
            <i class="fas fa-toggle-on"></i> Toggle
          </button>
        </div>
        <div class="slider-control">
          <label>Brightness: {ledBrightness}</label>
          <input type="range" min="0" max="255" bind:value={ledBrightness} on:change={handleLEDBrightness}>
        </div>
      </div>
    </div>
    
    <!-- RGB LED -->
    <div class="actuator-card">
      <div class="actuator-header">
        <div class="actuator-title">
          <i class="fas fa-palette" style="color: #9966FF;"></i>
          <span>RGB LED</span>
        </div>
        <div class="actuator-status" class:online={actuators.rgb?.on}>
          {actuators.rgb?.on ? 'ON' : 'OFF'}
        </div>
      </div>
      <div class="actuator-body">
        <div class="color-sliders">
          <div class="color-slider">
            <label><span style="color:#FF0000">●</span> R: {rgbR}</label>
            <input type="range" min="0" max="255" bind:value={rgbR} on:change={handleRGBChange}>
          </div>
          <div class="color-slider">
            <label><span style="color:#00FF00">●</span> G: {rgbG}</label>
            <input type="range" min="0" max="255" bind:value={rgbG} on:change={handleRGBChange}>
          </div>
          <div class="color-slider">
            <label><span style="color:#0000FF">●</span> B: {rgbB}</label>
            <input type="range" min="0" max="255" bind:value={rgbB} on:change={handleRGBChange}>
          </div>
        </div>
      </div>
    </div>
    
    <!-- Motor Control -->
    <div class="actuator-card">
      <div class="actuator-header">
        <div class="actuator-title">
          <i class="fas fa-motorcycle" style="color: #36A2EB;"></i>
          <span>Motor Control</span>
        </div>
        <div class="actuator-status" class:online={actuators.motor?.on}>
          {actuators.motor?.direction?.toUpperCase() || 'STOPPED'}
        </div>
      </div>
      <div class="actuator-body">
        <div class="control-buttons">
          <button class="btn btn-success" on:click={handleMotorForward}>
            <i class="fas fa-arrow-up"></i> Forward
          </button>
          <button class="btn btn-warning" on:click={handleMotorBackward}>
            <i class="fas fa-arrow-down"></i> Backward
          </button>
          <button class="btn btn-danger" on:click={handleMotorStop}>
            <i class="fas fa-stop"></i> Stop
          </button>
        </div>
        <div class="slider-control">
          <label>Speed: {motorSpeed}%</label>
          <input type="range" min="0" max="100" bind:value={motorSpeed}>
        </div>
      </div>
    </div>
    
    <!-- Servo Control -->
    <div class="actuator-card">
      <div class="actuator-header">
        <div class="actuator-title">
          <i class="fas fa-sliders-h" style="color: #FF9F40;"></i>
          <span>Servo Control</span>
        </div>
        <div class="actuator-status" class:online={actuators.servo?.on}>
          {actuators.servo?.angle || 0}°
        </div>
      </div>
      <div class="actuator-body">
        <div class="slider-control">
          <label>Angle: {servoAngle}°</label>
          <input type="range" min="0" max="180" bind:value={servoAngle} on:change={handleServoChange}>
        </div>
      </div>
    </div>
    
    <!-- Relay Control -->
    <div class="actuator-card">
      <div class="actuator-header">
        <div class="actuator-title">
          <i class="fas fa-plug" style="color: #4BC0C0;"></i>
          <span>Relay Control</span>
        </div>
      </div>
      <div class="actuator-body">
        <div class="relay-list">
          {#each [1, 2, 3] as num}
            <div class="relay-item">
              <span>Relay {num}</span>
              <button 
                class="btn btn-small" 
                class:btn-success={actuators.relays?.[num - 1]}
                on:click={() => handleRelayToggle(num)}
              >
                {actuators.relays?.[num - 1] ? 'ON' : 'OFF'}
              </button>
            </div>
          {/each}
        </div>
      </div>
    </div>
    
    <!-- Buzzer -->
    <div class="actuator-card">
      <div class="actuator-header">
        <div class="actuator-title">
          <i class="fas fa-volume-up" style="color: #FF6384;"></i>
          <span>Buzzer</span>
        </div>
        <div class="actuator-status" class:online={actuators.buzzer?.on}>
          {actuators.buzzer?.on ? 'PLAYING' : 'IDLE'}
        </div>
      </div>
      <div class="actuator-body">
        <div class="slider-control">
          <label>Frequency: {buzzerFreq} Hz</label>
          <input type="range" min="100" max="5000" bind:value={buzzerFreq}>
        </div>
        <button class="btn btn-primary" on:click={handleBuzzerTest}>
          <i class="fas fa-bell"></i> Test Buzzer
        </button>
      </div>
    </div>
  </div>
</div>

<style>
  .actuators-page {
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
    gap: 0.75rem;
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
  
  .btn-success { border-color: var(--success); color: var(--success); }
  .btn-success:hover { background: var(--success); color: var(--bg-primary); }
  .btn-warning { border-color: var(--warning); color: var(--warning); }
  .btn-warning:hover { background: var(--warning); color: var(--bg-primary); }
  .btn-danger { border-color: var(--danger); color: var(--danger); }
  .btn-danger:hover { background: var(--danger); color: white; }
  .btn-small { padding: 0.4rem 0.75rem; font-size: 0.7rem; }
  
  .actuator-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 1.25rem;
  }
  
  .actuator-card {
    background: var(--glass-bg);
    backdrop-filter: blur(10px);
    border: 1px solid var(--glass-border);
    border-radius: 1rem;
    overflow: hidden;
    transition: all 0.3s ease;
  }
  
  .actuator-card:hover {
    border-color: var(--primary);
    transform: translateY(-4px);
  }
  
  .actuator-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 1rem;
    background: rgba(255, 255, 255, 0.03);
    border-bottom: 1px solid var(--glass-border);
  }
  
  .actuator-title {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    font-weight: 600;
  }
  
  .actuator-status {
    font-size: 0.8rem;
    color: var(--text-muted);
  }
  
  .actuator-status.online {
    color: var(--success);
  }
  
  .actuator-body {
    padding: 1.25rem;
    display: flex;
    flex-direction: column;
    gap: 1rem;
  }
  
  .control-buttons {
    display: flex;
    gap: 0.5rem;
    flex-wrap: wrap;
  }
  
  .slider-control {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
  }
  
  .slider-control label {
    font-size: 0.85rem;
    color: var(--text-secondary);
  }
  
  .slider-control input[type="range"] {
    width: 100%;
    accent-color: var(--primary);
  }
  
  .color-sliders {
    display: flex;
    flex-direction: column;
    gap: 0.75rem;
  }
  
  .color-slider {
    display: flex;
    flex-direction: column;
    gap: 0.25rem;
  }
  
  .color-slider label {
    font-size: 0.8rem;
    color: var(--text-secondary);
  }
  
  .color-slider input[type="range"] {
    width: 100%;
  }
  
  .relay-list {
    display: flex;
    flex-direction: column;
    gap: 0.75rem;
  }
  
  .relay-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0.5rem;
    background: rgba(255, 255, 255, 0.03);
    border-radius: 0.5rem;
  }
</style>
