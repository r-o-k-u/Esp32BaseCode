/**
 * ═══════════════════════════════════════════════════════════════════════════
 * NON-BLOCKING TIMER - IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file Timer.cpp
 * @brief Implementation of non-blocking timer utility
 * @version 2.0.0
 * @date 2024
 *
 * This utility provides simple non-blocking timers for periodic tasks.
 * No delay() needed - keeps your loop responsive!
 *
 * WHY USE TIMERS INSTEAD OF delay()?
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Problem with delay():
 * @code
 * void loop() {
 *     readSensor();
 *     delay(1000);  // ❌ BLOCKS everything for 1 second!
 *     // Can't process buttons, network, OTA during delay
 * }
 * @endcode
 *
 * Solution with Timer:
 * @code
 * Timer sensorTimer(1000);
 *
 * void loop() {
 *     if (sensorTimer.isReady()) {
 *         readSensor();  // Only runs every 1000ms
 *     }
 *     // Loop continues immediately
 *     processButtons();  // ✓ Always responsive
 *     handleNetwork();   // ✓ Always responsive
 *     handleOTA();       // ✓ Always responsive
 * }
 * @endcode
 *
 * BENEFITS:
 * - Multiple timers can run simultaneously
 * - No blocking - system stays responsive
 * - Easy to adjust intervals
 * - Works with interrupts and WiFi
 * - OTA updates won't fail
 * - Button presses won't be missed
 *
 * USAGE EXAMPLES:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Example 1: Simple periodic task
 * @code
 * Timer ledTimer(500);  // Blink every 500ms
 *
 * void loop() {
 *     if (ledTimer.isReady()) {
 *         digitalWrite(LED, !digitalRead(LED));
 *     }
 * }
 * @endcode
 *
 * Example 2: Multiple timers
 * @code
 * Timer fastTimer(100);    // 10Hz - fast updates
 * Timer mediumTimer(1000); // 1Hz  - medium updates
 * Timer slowTimer(10000);  // 0.1Hz - slow updates
 *
 * void loop() {
 *     if (fastTimer.isReady()) {
 *         updateDisplay();  // Update display 10 times/sec
 *     }
 *
 *     if (mediumTimer.isReady()) {
 *         readSensors();    // Read sensors once/sec
 *     }
 *
 *     if (slowTimer.isReady()) {
 *         logData();        // Log data every 10 sec
 *     }
 * }
 * @endcode
 *
 * Example 3: Dynamic interval
 * @code
 * Timer adaptiveTimer(1000);
 *
 * void loop() {
 *     if (adaptiveTimer.isReady()) {
 *         int temp = readTemperature();
 *
 *         if (temp > 30) {
 *             adaptiveTimer.setInterval(500);  // Read faster when hot
 *         } else {
 *             adaptiveTimer.setInterval(2000); // Read slower when cool
 *         }
 *     }
 * }
 * @endcode
 *
 * Example 4: One-shot timer (run once)
 * @code
 * Timer delayTimer(5000);
 * bool taskDone = false;
 *
 * void loop() {
 *     if (delayTimer.isReady() && !taskDone) {
 *         performTask();  // Runs once after 5 seconds
 *         taskDone = true;
 *     }
 * }
 * @endcode
 */

#include "Timer.h"

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * TIMER PATTERNS AND BEST PRACTICES
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * PATTERN 1: Rate Limiting
 * ═══════════════════════════════════════════════════════════════════════════
 * Prevent functions from running too frequently
 *
 * @code
 * Timer rateLimit(100);  // Max 10 times per second
 *
 * void handleButtonPress() {
 *     if (buttonPressed() && rateLimit.isReady()) {
 *         doAction();  // Only triggers max 10x/sec
 *     }
 * }
 * @endcode
 *
 * PATTERN 2: Debouncing
 * ═══════════════════════════════════════════════════════════════════════════
 * Ignore rapid state changes (button bounce, sensor noise)
 *
 * @code
 * Timer debounceTimer(50);  // 50ms debounce
 * bool lastState = false;
 *
 * void readButton() {
 *     bool currentState = digitalRead(BUTTON_PIN);
 *
 *     if (currentState != lastState && debounceTimer.isReady()) {
 *         if (currentState == HIGH) {
 *             handleButtonPress();
 *         }
 *         lastState = currentState;
 *     }
 * }
 * @endcode
 *
 * PATTERN 3: State Machines
 * ═══════════════════════════════════════════════════════════════════════════
 * Time-based state transitions
 *
 * @code
 * enum State { IDLE, HEATING, COOLING, DONE };
 * State currentState = IDLE;
 * Timer stateTimer(5000);
 *
 * void stateMachine() {
 *     if (stateTimer.isReady()) {
 *         switch(currentState) {
 *             case IDLE:
 *                 currentState = HEATING;
 *                 stateTimer.setInterval(10000);
 *                 break;
 *             case HEATING:
 *                 currentState = COOLING;
 *                 stateTimer.setInterval(10000);
 *                 break;
 *             case COOLING:
 *                 currentState = DONE;
 *                 break;
 *         }
 *     }
 * }
 * @endcode
 *
 * PATTERN 4: Periodic Maintenance
 * ═══════════════════════════════════════════════════════════════════════════
 * Regular system maintenance tasks
 *
 * @code
 * Timer maintenanceTimer(3600000);  // Every hour
 *
 * void loop() {
 *     if (maintenanceTimer.isReady()) {
 *         cleanupMemory();
 *         checkSystemHealth();
 *         logStatistics();
 *         compactDatabase();
 *     }
 * }
 * @endcode
 *
 * PERFORMANCE NOTES:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * - isReady() is very fast (~1μs)
 * - Safe to call isReady() every loop iteration
 * - No memory allocation (everything on stack)
 * - Works in interrupts (if millis() does)
 * - Thread-safe for single-core operations
 *
 * MEMORY USAGE:
 * Each Timer object uses 8 bytes:
 * - 4 bytes for interval
 * - 4 bytes for lastTime
 *
 * You can have hundreds of timers without memory issues.
 *
 * COMMON MISTAKES:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * ❌ WRONG: Creating timer inside loop
 * @code
 * void loop() {
 *     Timer t(1000);  // Creates new timer every loop!
 *     if (t.isReady()) { ... }
 * }
 * @endcode
 *
 * ✓ CORRECT: Create timer outside loop
 * @code
 * Timer t(1000);  // Create once
 * void loop() {
 *     if (t.isReady()) { ... }
 * }
 * @endcode
 *
 * ❌ WRONG: Not calling isReady() regularly
 * @code
 * Timer t(1000);
 * void loop() {
 *     delay(5000);    // Timer missed its trigger!
 *     if (t.isReady()) { ... }
 * }
 * @endcode
 *
 * ✓ CORRECT: Call isReady() frequently
 * @code
 * Timer t(1000);
 * void loop() {
 *     if (t.isReady()) { ... }
 *     // No delay() calls that would block
 * }
 * @endcode
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * END OF TIMER IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 */
