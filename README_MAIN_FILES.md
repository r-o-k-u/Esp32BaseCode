# Main Application Files Guide

## File Structure Overview

The ESP32 Dual Communication System provides two main application files:

### 1. `src/main.cpp` - Standard Application
- **Purpose**: Complete ESP32 system without GPIOViewer
- **Features**: WiFi, ESP-NOW, Web Server, OTA, Sensors, Actuators, Camera
- **Use Case**: Standard deployment without GPIO monitoring

### 2. `src/main_with_gpio_viewer.cpp` - GPIOViewer Enhanced Application
- **Purpose**: Complete ESP32 system WITH GPIOViewer integration
- **Features**: All features from main.cpp PLUS GPIO monitoring
- **Use Case**: Development, debugging, and GPIO monitoring scenarios

## Key Differences

| Feature | main.cpp | main_with_gpio_viewer.cpp |
|---------|----------|---------------------------|
| GPIOViewer | ❌ Not included | ✅ Full integration |
| Port Usage | Port 80 only | Port 80 + Port 8080 |
| Memory Usage | Lower | Slightly higher |
| GPIO Monitoring | Manual only | Real-time web interface |
| Debugging | Serial only | Serial + Web interface |

## Port Configuration

### main.cpp (Standard)
- **Web Server**: Port 80
- **URL**: http://[IP]/

### main_with_gpio_viewer.cpp (Enhanced)
- **Web Server**: Port 80
- **GPIOViewer**: Port 8080
- **Main URL**: http://[IP]/
- **GPIO URL**: http://[IP]:8080/

## How to Choose

### Use `main.cpp` when:
- You don't need GPIO monitoring
- Memory is constrained
- You want the standard deployment
- GPIO monitoring is not required for your application

### Use `main_with_gpio_viewer.cpp` when:
- You need real-time GPIO monitoring
- You're developing/debugging hardware
- You want to visualize pin states
- You need to monitor GPIO conflicts
- You're troubleshooting sensor/actuator issues

## GPIOViewer Benefits

The GPIOViewer integration provides:

1. **Real-time Monitoring**: Live pin state visualization
2. **Pin Function Detection**: Automatic detection of pin usage
3. **Conflict Detection**: Identifies pin conflicts between components
4. **Web Interface**: Easy-to-use web-based monitoring
5. **Development Aid**: Essential for hardware debugging

## Port Conflict Resolution

The enhanced version includes automatic port conflict prevention:

```cpp
// Compile-time validation prevents conflicts
#if GPIOVIEWER_PORT == WEB_SERVER_PORT
#error "GPIOVIEWER_PORT cannot be the same as WEB_SERVER_PORT"
#endif
```

## Memory Considerations

- **main.cpp**: ~50KB program size
- **main_with_gpio_viewer.cpp**: ~100KB program size (includes GPIOViewer assets)

## Switching Between Versions

To switch between versions:

1. **Rename files**:
   - For standard: `main.cpp` (keep this name)
   - For GPIOViewer: `main_with_gpio_viewer.cpp` → `main.cpp`

2. **Or use build flags**:
   - Add `-D ENABLE_GPIOVIEWER` to enable GPIOViewer
   - Modify the main.cpp to conditionally include GPIOViewer

## Recommended Usage

### Development Phase
- Use `main_with_gpio_viewer.cpp` for hardware development
- Monitor GPIO conflicts during prototyping
- Debug sensor/actuator wiring issues

### Production Deployment
- Use `main.cpp` for final deployment
- Lower memory usage
- No unnecessary GPIO monitoring overhead

### Testing Phase
- Use `main_with_gpio_viewer.cpp` to validate hardware
- Ensure no GPIO conflicts
- Verify all components work correctly

## GPIOViewer Access

When using `main_with_gpio_viewer.cpp`:

1. **Main Dashboard**: http://[IP]/
2. **GPIO Monitoring**: http://[IP]:8080/
3. **Both services run simultaneously without conflicts**

## Troubleshooting

### Port 8080 Blocked
If GPIOViewer port is blocked by firewall:
1. Change `GPIOVIEWER_PORT` in config.h
2. Use alternative port (8081, 8888, 9000, etc.)
3. Update firewall settings

### Memory Issues
If experiencing memory problems:
1. Use `main.cpp` instead
2. Reduce sensor read intervals
3. Disable unused features in config.h

## Conclusion

Both files provide the same core functionality. Choose based on your GPIO monitoring needs:

- **Standard operation**: Use `main.cpp`
- **Development/debugging**: Use `main_with_gpio_viewer.cpp`

The GPIOViewer integration is designed to be non-intrusive and only adds value when needed.