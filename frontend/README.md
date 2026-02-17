# ESP32 DualComm Web Interface

A modern, reactive web interface for the ESP32 DualComm System built with **Svelte**. This frontend provides real-time monitoring and control of sensors, actuators, WiFi, and ESP-NOW communication.

## Why Svelte?

Svelte was chosen over React or Vue for this ESP32 application because:

1. **Smaller Bundle Size**: Svelte compiles away to tiny vanilla JS, perfect for embedded systems with limited resources
2. **No Virtual DOM**: Direct DOM updates mean better performance on resource-constrained devices
3. **Simpler State Management**: Built-in reactive stores eliminate the need for complex state libraries
4. **Easier Learning Curve**: Less boilerplate code than React, faster development than Vue
5. **Better SSR Support**: Can be rendered on the server if needed

## Features

- **Dashboard**: Real-time system overview with sensor readings and activity logs
- **Sensors**: Monitor temperature, humidity, pressure, light, motion, distance, air quality
- **Actuators**: Control LED, RGB LED, motors, servos, relays, buzzers
- **WiFi Manager**: Connect to networks, scan available networks, AP mode
- **ESP-NOW Communication**: View peers and message logs
- **OTA Updates**: Firmware upload and updates
- **Logs**: Activity logging with filtering and export
- **Configuration**: Device settings and preferences

## Project Structure

```
frontend/
├── src/
│   ├── components/       # Reusable UI components
│   │   ├── Header.svelte      # Top header with status
│   │   ├── Navigation.svelte  # Tab navigation
│   │   ├── Controls.svelte    # Global control buttons
│   │   ├── Dashboard.svelte   # Main dashboard view
│   │   ├── Sensors.svelte     # Sensor monitoring
│   │   ├── Actuators.svelte   # Actuator control
│   │   ├── WiFiManager.svelte # WiFi management
│   │   ├── Communication.svelte # ESP-NOW comm
│   │   ├── OTAUpdate.svelte   # OTA firmware updates
│   │   ├── Logs.svelte        # Activity logs
│   │   ├── Configuration.svelte # Settings
│   │   └── ToastContainer.svelte # Toast notifications
│   ├── services/         # API and WebSocket services
│   │   └── api.js
│   ├── stores/           # Svelte stores for state
│   │   └── index.js
│   ├── App.svelte        # Main application component
│   └── main.js          # Application entry point
├── public/               # Static assets
├── index.html           # HTML template
├── package.json         # Dependencies
├── vite.config.js       # Vite configuration
└── svelte.config.js    # Svelte configuration
```

## Getting Started

### Prerequisites

- Node.js 16+
- npm or yarn

### Installation

```bash
cd frontend
npm install
```

### Development

```bash
npm run dev
```

### Production Build

```bash
npm run build
```

### Preview Production Build

```bash
npm run preview
```

## API Integration

The frontend communicates with the ESP32 via REST API and WebSocket:

- **REST API**: For configuration, WiFi, OTA updates
- **WebSocket**: For real-time sensor data and status updates

### API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/status` | GET | Get system status |
| `/api/sensors` | GET | Get sensor readings |
| `/api/actuators` | GET/POST | Get/set actuator state |
| `/api/wifi/scan` | POST | Scan for networks |
| `/api/wifi/connect` | POST | Connect to network |
| `/api/ota/update` | POST | Upload firmware |
| `/api/restart` | POST | Restart device |

## WebSocket Events

| Event | Direction | Description |
|-------|-----------|-------------|
| `sensorData` | ESP32 → Frontend | Real-time sensor readings |
| `actuatorStatus` | ESP32 → Frontend | Actuator state changes |
| `espnowStatus` | ESP32 → Frontend | ESP-NOW peer updates |
| `getSensorData` | Frontend → ESP32 | Request sensor data |
| `setActuator` | Frontend → ESP32 | Control actuator |

## Customization

### Theme

The color scheme can be customized in `src/app.css`:

```css
:root {
  --primary: #00ffff;
  --secondary: #ff00ff;
  --bg-primary: #0d0d1a;
  --glass-bg: rgba(26, 26, 46, 0.8);
}
```

### Adding New Sensors/Actuators

1. Add API endpoints in ESP32 firmware
2. Create component or extend existing in `src/components/`
3. Add store for state management in `src/stores/`
4. Wire up in `App.svelte`

## Browser Support

- Chrome 90+
- Firefox 88+
- Safari 14+
- Edge 90+

## License

MIT License - See LICENSE file for details
