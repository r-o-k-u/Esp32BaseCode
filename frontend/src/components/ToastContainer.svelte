<script>
  import { toasts } from '../stores/index.js';
  
  function dismissToast(id) {
    toasts.update(t => t.filter(toast => toast.id !== id));
  }
</script>

<div class="toast-container">
  {#each $toasts as toast (toast.id)}
    <div 
      class="toast {toast.type}"
      on:click={() => dismissToast(toast.id)}
      on:keydown={(e) => e.key === 'Enter' && dismissToast(toast.id)}
      role="alert"
      tabindex="0"
    >
      <div class="toast-icon">
        {#if toast.type === 'success'}
          <i class="fas fa-check-circle"></i>
        {:else if toast.type === 'error'}
          <i class="fas fa-times-circle"></i>
        {:else if toast.type === 'warning'}
          <i class="fas fa-exclamation-triangle"></i>
        {:else}
          <i class="fas fa-info-circle"></i>
        {/if}
      </div>
      <div class="toast-message">
        {toast.message}
      </div>
      <button class="toast-close" on:click|stopPropagation={() => dismissToast(toast.id)}>
        <i class="fas fa-times"></i>
      </button>
    </div>
  {/each}
</div>

<style>
  .toast-container {
    position: fixed;
    bottom: 1.5rem;
    right: 1.5rem;
    z-index: 9999;
    display: flex;
    flex-direction: column;
    gap: 0.75rem;
    max-width: 400px;
  }
  
  .toast {
    display: flex;
    align-items: center;
    gap: 0.75rem;
    padding: 1rem 1.25rem;
    background: linear-gradient(135deg, rgba(26, 26, 46, 0.95), rgba(13, 13, 26, 0.95));
    backdrop-filter: blur(20px);
    border: 1px solid var(--glass-border);
    border-left: 4px solid var(--primary);
    border-radius: 0.75rem;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
    cursor: pointer;
    animation: slideIn 0.3s ease;
    transform: translateX(120%);
    opacity: 0;
  }
  
  .toast.show {
    transform: translateX(0);
    opacity: 1;
  }
  
  .toast.success {
    border-left-color: var(--success);
  }
  
  .toast.error {
    border-left-color: var(--danger);
  }
  
  .toast.warning {
    border-left-color: var(--warning);
  }
  
  .toast.info {
    border-left-color: var(--info);
  }
  
  .toast-icon {
    font-size: 1.25rem;
  }
  
  .toast.success .toast-icon {
    color: var(--success);
  }
  
  .toast.error .toast-icon {
    color: var(--danger);
  }
  
  .toast.warning .toast-icon {
    color: var(--warning);
  }
  
  .toast.info .toast-icon {
    color: var(--info);
  }
  
  .toast-message {
    flex: 1;
    font-size: 0.9rem;
    color: var(--text-primary);
  }
  
  .toast-close {
    background: none;
    border: none;
    color: var(--text-muted);
    cursor: pointer;
    padding: 0.25rem;
    font-size: 0.9rem;
    transition: color 0.2s;
  }
  
  .toast-close:hover {
    color: var(--text-primary);
  }
  
  @keyframes slideIn {
    from {
      transform: translateX(120%);
      opacity: 0;
    }
    to {
      transform: translateX(0);
      opacity: 1;
    }
  }
  
  @media (max-width: 480px) {
    .toast-container {
      left: 1rem;
      right: 1rem;
      max-width: none;
    }
  }
</style>
