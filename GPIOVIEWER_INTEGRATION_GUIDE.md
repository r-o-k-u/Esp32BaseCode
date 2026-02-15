# GPIOViewer Integration Guide

## Overview

This guide explains how to integrate the GPIOViewer library into your ESP32 Dual Communication System without breaking existing functionality.

## What is GPIOViewer?

GPIOViewer is an Arduino library that provides real-time GPIO pin monitoring and visualization for ESP32 boards. It creates a web interface that shows live pin states, functions, and board layouts.

## Features

- Real-time GPIO pin monitoring
- Live pin state visualization
- Pin function detection (ADC, Touch, etc.)
- Board-specific pin layouts
- mDNS support (gpioviewer.local:8080)
- Web-based interface accessible via browser

## Integration Steps

### 1. Update platformio.ini

Add the GPIOViewer library to your `platformio.ini` file:

```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
board_build.filesystem = spiffs

; Build flags
build_flags = 
    -D DEVICE_TYPE=0          ; 0 = ESP32, 1 = ESP32-CAM
    -D CORE_DEBUG_LEVEL=3     ; Debug level
    -D CONFIG_ASYNC_TCP_RUNNING_CORE=1
    -D CONFIG_ASYNC_TCP_USE_WDT=1

; Library dependencies
lib_deps = 
    ESP Async WebServer@^1.2.3
    ESPAsyncTCP@^1.2.2
    ArduinoJson@^6.21.3
    DHT sensor library@^1.4.4
    Adafruit Unified Sensor@^1.1.9
    Adafruit BMP280 Library@^2.6.6
    MPU6050@^1.0.0
    ESP32Servo@^0.13.0
    PubSubClient@^2.8          ; For future MQTT support
    ESP32Time@^2.0.0
    https://github.com/thelastoutpostworkshop/gpio_viewer.git
```

### 2. Modify main.cpp

Add the following to your `src/main.cpp` file:

#### A. Include the library (add after other includes):

```cpp
// GPIOViewer library - Must be included after other libraries
#include <gpio_viewer.h>
```

#### B. Add global instance declaration:

```cpp
// Global object instances
GPIOViewer gpioViewer; // GPIO monitoring instance
```

#### C. Add initialization function:

```cpp
void initGPIOViewer()
{
    DEBUG_PRINTLN("\n[GPIO] Initializing GPIOViewer...");

    try
    {
        // Configure GPIOViewer settings for optimal performance
        gpioViewer.setPort(8080);               // HTTP port for GPIOViewer
        gpioViewer.setSamplingInterval(100);    // Sampling interval in ms (default: 100ms)
        gpioViewer.setSkipPeripheralPins(true); // Skip I2C/SPI/UART pins owned by peripherals

        // Initialize GPIOViewer - this will start the web server on port 8080
        gpioViewer.begin();

        DEBUG_PRINTLN("✓ GPIOViewer initialized successfully");
        DEBUG_PRINTLN("┌───────────────────────────────────────────────────┐");
        DEBUG_PRINTLN("│              GPIOVIEWER ACCESS                    │");
        DEBUG_PRINTLN("├───────────────────────────────────────────────────┤");
        DEBUG_PRINTF("│ URL:  http://%-35s │\n", WiFi.localIP().toString().c_str());
        DEBUG_PRINTF("│ Port: %-35s │\n", "8080");
        DEBUG_PRINTLN("│                                                   │");
        DEBUG_PRINTLN("│ Features:                                         │");
        DEBUG_PRINTLN("│ • Real-time GPIO pin monitoring                   │");
        DEBUG_PRINTLN("│ • Live pin state visualization                    │");
        DEBUG_PRINTLN("│ • Pin function detection                          │");
        DEBUG_PRINTLN("│ • Board-specific pin layouts                      │");
        DEBUG_PRINTLN("│ • mDNS support (gpioviewer.local:8080)            │");
        DEBUG_PRINTLN("└───────────────────────────────────────────────────┘");
    }
    catch (const std::exception &e)
    {
        DEBUG_PRINTLN("⚠️ GPIOViewer initialization failed!");
        DEBUG_PRINTF("   Error: %s\n", e.what());
        DEBUG_PRINTLN("   Continuing without GPIO monitoring...");
    }
    catch (...)
    {
        DEBUG_PRINTLN("⚠️ GPIOViewer initialization failed with unknown error!");
        DEBUG_PRINTLN("   Continuing without GPIO monitoring...");
    }
}
```

#### D. Add update function:

```cpp
void updateGPIOViewer()
{
    // GPIOViewer handles its own timing and web server requests
    // This function can be called frequently from the main loop
    // without blocking other operations
}
```

#### E. Call initialization in setup() (add after WiFi connection):

```cpp
// ─────────────────────────────────────────────────────────────────────
// 10. INITIALIZE GPIOVIEWER
// ─────────────────────────────────────────────────────────────────────
DEBUG_PRINTLN("\n[9/10] Initializing GPIOViewer...");
initGPIOViewer();
```

#### F. Call update in loop():

```cpp
// ─────────────────────────────────────────────────────────────────────
// 10. UPDATE GPIOVIEWER
// ─────────────────────────────────────────────────────────────────────
updateGPIOViewer();
```

### 3. Build and Upload

1. Save all changes
2. Build the project: `pio run`
3. Upload to ESP32: `pio run -t upload`

## Accessing GPIOViewer

Once the ESP32 boots up:

1. **Via IP Address**: Open your browser and go to `http://<ESP32_IP>:8080`
2. **Via mDNS**: If supported, use `http://gpioviewer.local:8080`

## Configuration Options

### Port Configuration
```cpp
gpioViewer.setPort(8080);  // Change default port (default: 8080)
```

### Sampling Interval
```cpp
gpioViewer.setSamplingInterval(100);  // Change sampling rate in ms (default: 100ms)
```

### Peripheral Pin Handling
```cpp
gpioViewer.setSkipPeripheralPins(true);  // Skip I2C/SPI/UART pins (default: true)
gpioViewer.setSkipPeripheralPins(false); // Monitor all pins
```

## Troubleshooting

### GPIOViewer won't start:
- Check WiFi is connected (not in AP mode)
- Verify port 8080 is not blocked by firewall
- Ensure sufficient free heap memory
- Check Serial Monitor for error messages

### Web interface not accessible:
- Try `http://<ip>:8080`
- Check if mDNS works: `http://gpioviewer.local:8080`
- Verify WiFi connection is stable
- Ensure no other service is using port 8080

### Pins not showing correctly:
- GPIOViewer skips pins used by I2C/SPI/UART by default
- Use `setSkipPeripheralPins(false)` to monitor all pins
- Check if pins are configured as outputs elsewhere

### Performance issues:
- Reduce sampling interval if system is overloaded
- Consider disabling GPIOViewer if not needed
- Monitor memory usage in Serial Monitor

## Compatibility Notes

- **WiFi Mode**: GPIOViewer requires WiFi_STA mode (not AP mode)
- **Port Usage**: Uses port 8080 by default (can be changed)
- **Memory**: Adds ~50KB to project size (assets loaded from GitHub Pages)
- **Coexistence**: Works alongside existing web server (different port)
- **Pin Detection**: Automatically detects pin functions (ADC, Touch, etc.)

## Safety Features

- **Error Handling**: Graceful failure if GPIOViewer initialization fails
- **Non-blocking**: Doesn't interfere with main system operations
- **Optional**: System continues to work if GPIOViewer fails
- **Resource Management**: Proper cleanup on errors

## Example Output

When GPIOViewer initializes successfully, you'll see:

```
[GPIO] Initializing GPIOViewer...
✓ GPIOViewer initialized successfully
┌───────────────────────────────────────────────────┐
│              GPIOVIEWER ACCESS                    │
├───────────────────────────────────────────────────┤
│ URL:  http://192.168.1.100                         │
│ Port: 8080                                        │
│                                                   │
│ Features:                                         │
│ • Real-time GPIO pin monitoring                   │
│ • Live pin state visualization                    │
│ • Pin function detection                          │
│ • Board-specific pin layouts                      │
│ • mDNS support (gpioviewer.local:8080)            │
└───────────────────────────────────────────────────┘
```

## Integration Benefits

1. **Non-invasive**: Doesn't modify existing functionality
2. **Optional**: System works fine without GPIOViewer
3. **Real-time**: Live monitoring of all GPIO pins
4. **Visual**: Easy-to-understand web interface
5. **Comprehensive**: Shows pin functions and board layout
6. **Accessible**: Works from any device with a browser

## Next Steps

After successful integration:

1. Test the web interface
2. Verify all pins are detected correctly
3. Adjust sampling interval if needed
4. Consider adding GPIOViewer to your monitoring workflow
5. Explore advanced features like pin function detection

## Support

For issues with GPIOViewer:
- Check the [GPIOViewer GitHub repository](https://github.com/thelastoutpostworkshop/gpio_viewer)
- Review the [GPIOViewer documentation](https://github.com/thelastoutpostworkshop/gpio_viewer/blob/main/README.md)
- Check the Serial Monitor for detailed error messages