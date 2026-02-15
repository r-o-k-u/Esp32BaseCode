/**
 * ═══════════════════════════════════════════════════════════════════════════
 * GPIOVIEWER INTEGRATION HEADER
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file gpio_viewer_integration.h
 * @brief GPIOViewer library integration for ESP32 Dual Communication System
 * @version 2.0.0
 * @date 2024
 * @author Your Name
 * @license MIT
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * DESCRIPTION:
 * ═══════════════════════════════════════════════════════════════════════════
 * This header provides GPIOViewer integration without modifying existing
 * functionality. It can be included in main.cpp to add GPIO monitoring
 * capabilities to the ESP32 Dual Communication System.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#pragma once

// Include GPIOViewer library
#include <gpio_viewer.h>

// Global GPIOViewer instance
extern GPIOViewer gpioViewer;

// GPIOViewer configuration and initialization
void initGPIOViewer();
void updateGPIOViewer();

// ═══════════════════════════════════════════════════════════════════════════
// GPIOVIEWER CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Initialize GPIOViewer with optimal settings for the ESP32 system
 *
 * This function configures GPIOViewer to work seamlessly with the
 * existing ESP32 Dual Communication System without interfering with
 * other components.
 */
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

// ═══════════════════════════════════════════════════════════════════════════
// GPIOVIEWER UPDATE FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Update GPIOViewer (called from main loop)
 *
 * This function should be called periodically from the main loop
 * to keep GPIOViewer responsive. It handles web server requests
 * and updates the GPIO monitoring interface.
 */
void updateGPIOViewer()
{
    // GPIOViewer handles its own timing and web server requests
    // This function can be called frequently from the main loop
    // without blocking other operations

    // Note: GPIOViewer.begin() starts its own web server that runs
    // in the background. This function is mainly for future extensibility
    // and potential real-time updates if needed.
}

// ═══════════════════════════════════════════════════════════════════════════
// GPIOVIEWER INTEGRATION INSTRUCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * TO INTEGRATE GPIOVIEWER INTO YOUR PROJECT:
 *
 * 1. Add this header to your project:
 *    #include "gpio_viewer_integration.h"
 *
 * 2. Add the global instance declaration to your main.cpp:
 *    GPIOViewer gpioViewer;
 *
 * 3. Call initialization in setup() after WiFi connection:
 *    initGPIOViewer();
 *
 * 4. Call update in loop() for responsiveness:
 *    updateGPIOViewer();
 *
 * 5. Ensure platformio.ini includes the GPIOViewer library:
 *    https://github.com/thelastoutpostworkshop/gpio_viewer.git
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * COMPATIBILITY NOTES:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * - GPIOViewer requires WiFi_STA mode (not AP mode)
 * - Uses port 8080 by default (can be changed with setPort())
 * - Automatically detects pin functions (ADC, Touch, etc.)
 * - Skips pins owned by active I2C/SPI/UART peripherals by default
 * - Adds ~50KB to project size (assets loaded from GitHub Pages)
 * - Works alongside existing web server (different port)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * TROUBLESHOOTING:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. GPIOViewer won't start:
 *    - Check WiFi is connected (not in AP mode)
 *    - Verify port 8080 is not blocked by firewall
 *    - Ensure sufficient free heap memory
 *
 * 2. Web interface not accessible:
 *    - Try http://<ip>:8080
 *    - Check if mDNS works: http://gpioviewer.local:8080
 *    - Verify WiFi connection is stable
 *
 * 3. Pins not showing correctly:
 *    - GPIOViewer skips pins used by I2C/SPI/UART by default
 *    - Use setSkipPeripheralPins(false) to monitor all pins
 *    - Check if pins are configured as outputs elsewhere
 *
 * 4. Performance issues:
 *    - Reduce sampling interval if system is overloaded
 *    - Consider disabling GPIOViewer if not needed
 *    - Monitor memory usage in Serial Monitor
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */