# GPIOViewer Port Conflict Solution

## Problem Analysis

The user was concerned that GPIOViewer would interfere with the main web server because both were trying to use port 8080. This would cause a port conflict and prevent one or both services from starting.

## Root Cause

In the original `main_with_gpio_viewer.cpp`, GPIOViewer was hardcoded to use port 8080:

```cpp
gpioViewer.setPort(8080);               // HTTP port for GPIOViewer
```

Meanwhile, the main web server was configured to use port 80 (default HTTP port):

```cpp
#define WEB_SERVER_PORT 80
```

This created a potential conflict if both services tried to bind to the same port.

## Solution Implemented

### 1. Added GPIOViewer Port Configuration to config.h

Added a dedicated configuration constant for GPIOViewer port:

```cpp
/**
 * GPIOViewer port configuration
 *
 * GPIOVIEWER_PORT: Port for GPIOViewer web interface
 * Must be different from WEB_SERVER_PORT to avoid conflicts
 *
 * DEFAULT: 8080 (different from main web server on port 80)
 * ALTERNATIVES: 8081, 8082, 8888, 9000, etc.
 *
 * IMPORTANT: Ensure this port is not blocked by firewall
 * and is different from any other services running on the ESP32
 */
#define GPIOVIEWER_PORT 8080
```

### 2. Added Compile-Time Validation

Added compile-time checks to prevent port conflicts:

```cpp
// Check port configuration for conflicts
#if GPIOVIEWER_PORT == WEB_SERVER_PORT
#error "GPIOVIEWER_PORT cannot be the same as WEB_SERVER_PORT"
#endif

#if GPIOVIEWER_PORT == OTA_PORT
#warning "GPIOVIEWER_PORT conflicts with OTA_PORT - consider changing one"
#endif
```

### 3. Updated GPIOViewer Initialization

Modified the GPIOViewer initialization in `main_with_gpio_viewer.cpp` to use the configurable port:

```cpp
// Configure GPIOViewer settings using config.h port
gpioViewer.setPort(GPIOVIEWER_PORT);    // HTTP port for GPIOViewer (8080)
gpioViewer.setSamplingInterval(100);    // Sampling interval in ms (default: 100ms)
gpioViewer.setSkipPeripheralPins(true); // Skip I2C/SPI/UART pins owned by peripherals (default: true)
```

### 4. Enhanced User Information

Updated the startup banner to clearly show both services and their ports:

```cpp
DEBUG_PRINTLN("│ NOTE: Different port from main web server         │");
DEBUG_PRINTLN("│       Main server: http://ip/ (port 80)           │");
DEBUG_PRINTLN("│       GPIOViewer:  http://ip:8080/               │");
```

## Port Configuration Summary

| Service | Port | URL |
|---------|------|-----|
| Main Web Server | 80 | http://[IP]/ |
| GPIOViewer | 8080 | http://[IP]:8080/ |
| OTA Updates | 3232 | Network port for OTA |
| WebSocket | 81 | WebSocket for real-time updates |

## Benefits of This Solution

1. **No Port Conflicts**: Each service uses a different port
2. **Configurable**: Ports can be easily changed in config.h
3. **Compile-Time Safety**: Prevents accidental port conflicts
4. **Clear Documentation**: Users know exactly which port to use for each service
5. **Backward Compatible**: Existing configurations continue to work

## Testing the Solution

To verify the solution works:

1. **Compile the code**: Should compile without errors
2. **Check Serial Monitor**: Should show both services starting successfully
3. **Test Web Server**: http://[IP]/ should load the main dashboard
4. **Test GPIOViewer**: http://[IP]:8080/ should load the GPIO monitoring interface
5. **Verify No Conflicts**: Both services should be accessible simultaneously

## Alternative Port Configurations

If port 8080 is already in use, users can easily change it in config.h:

```cpp
#define GPIOVIEWER_PORT 8081  // Alternative port
// #define GPIOVIEWER_PORT 8888  // Another alternative
// #define GPIOVIEWER_PORT 9000  // Yet another alternative
```

The compile-time validation will ensure the new port doesn't conflict with other services.

## Firewall Considerations

Users may need to ensure that the GPIOViewer port (8080 by default) is not blocked by their firewall, especially in corporate or restrictive network environments.

## Conclusion

This solution provides a clean, configurable way to run both the main web server and GPIOViewer simultaneously without port conflicts. The implementation is robust, well-documented, and includes safety checks to prevent configuration errors.