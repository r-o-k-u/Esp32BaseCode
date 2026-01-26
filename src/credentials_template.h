/**
 * ═══════════════════════════════════════════════════════════════════════════
 * ESP32 DUAL COMMUNICATION SYSTEM - CREDENTIALS FILE
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file credentials.h
 * @brief WiFi credentials and sensitive configuration
 * @version 2.0.0
 * @date 2024
 * @author Your Name
 * @license MIT
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * ⚠️ SECURITY WARNING - READ THIS CAREFULLY!
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * This file contains SENSITIVE INFORMATION including:
 * - WiFi passwords
 * - API keys
 * - Web interface passwords
 * - MQTT credentials
 * - Device MAC addresses
 *
 * DO NOT COMMIT THIS FILE TO PUBLIC REPOSITORIES!
 *
 * STEPS TO SECURE THIS FILE:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. ADD TO .gitignore:
 *    Add this line to your .gitignore file:
 *    src/credentials.h
 *
 * 2. CREATE A TEMPLATE:
 *    Create credentials_template.h with placeholder values:
 *    #define WIFI_SSID_1 "YOUR_WIFI_SSID"
 *    #define WIFI_PASS_1 "YOUR_WIFI_PASSWORD"
 *
 * 3. DOCUMENT IN README:
 *    Tell users to copy credentials_template.h to credentials.h
 *    and fill in their actual values
 *
 * 4. USE ENVIRONMENT VARIABLES (Advanced):
 *    In platformio.ini:
 *    build_flags =
 *      -D WIFI_SSID=\"${sysenv.WIFI_SSID}\"
 *      -D WIFI_PASSWORD=\"${sysenv.WIFI_PASSWORD}\"
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * USAGE INSTRUCTIONS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. Replace "YourWiFiSSID" with your actual WiFi network name
 * 2. Replace "YourWiFiPassword" with your actual WiFi password
 * 3. Change all default passwords (admin123, etc.)
 * 4. Get MAC addresses from Serial Monitor after first boot
 * 5. Update PEER_MAC_1, PEER_MAC_2 with actual device MAC addresses
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// ═══════════════════════════════════════════════════════════════════════════
// WIFI NETWORK CREDENTIALS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Primary WiFi Network
 *
 * SSID: Your WiFi network name (case-sensitive!)
 * PASSWORD: Your WiFi password (minimum 8 characters for WPA2)
 *
 * TROUBLESHOOTING:
 * - Ensure SSID is exactly correct (case matters!)
 * - Check password has no leading/trailing spaces
 * - ESP32 only supports 2.4GHz networks (not 5GHz)
 * - ESP32 only supports WPA/WPA2 (not WPA3 in most cases)
 * - Some special characters in passwords may cause issues
 *
 * EXAMPLE:
 * #define WIFI_SSID_1     "MyHomeNetwork"
 * #define WIFI_PASS_1     "MySecurePassword123"
 */
#define WIFI_SSID_1 "YourWiFiSSID"
#define WIFI_PASS_1 "YourWiFiPassword"

/**
 * Secondary WiFi Network (Optional Backup)
 *
 * If primary network fails, system will try this one.
 * Useful for:
 * - Mobile hotspot backup
 * - Different location networks
 * - Failover connectivity
 *
 * To use single network: Set both to same values
 * To disable backup: Leave as empty strings ""
 */
#define WIFI_SSID_2 "BackupWiFiSSID"
#define WIFI_PASS_2 "BackupWiFiPassword"

// ═══════════════════════════════════════════════════════════════════════════
// WEB INTERFACE CREDENTIALS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Web Interface Login
 *
 * Used for HTTP Basic Authentication on web dashboard
 * Browser will prompt for username and password
 *
 * ⚠️ IMPORTANT: CHANGE THESE FROM DEFAULTS!
 * Default credentials are a SECURITY RISK
 *
 * SECURITY NOTES:
 * - This is HTTP Basic Auth (not very secure over HTTP)
 * - Use HTTPS in production (requires SSL certificates)
 * - Or keep device on isolated network
 * - Don't use same password as WiFi
 *
 * EXAMPLE:
 * #define WEB_USERNAME    "myadmin"
 * #define WEB_PASSWORD    "MyStr0ngP@ssw0rd!"
 */
#define WEB_USERNAME "admin"
#define WEB_PASSWORD "admin123" // ⚠️ CHANGE THIS!

// ═══════════════════════════════════════════════════════════════════════════
// MQTT BROKER CREDENTIALS (if using MQTT in future)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * MQTT Broker Configuration
 *
 * MQTT is used for cloud connectivity and IoT platforms
 * Currently not implemented but included for future expansion
 *
 * Popular MQTT brokers:
 * - Mosquitto (local server)
 * - HiveMQ Cloud (cloud service)
 * - AWS IoT Core (Amazon)
 * - Azure IoT Hub (Microsoft)
 * - CloudMQTT (cloud service)
 *
 * MQTT_SERVER: Broker address (IP or domain)
 * MQTT_PORT: Usually 1883 (unencrypted) or 8883 (SSL)
 * MQTT_USER: Username (if broker requires authentication)
 * MQTT_PASS: Password
 * MQTT_CLIENT_ID: Unique identifier for this device
 *
 * EXAMPLE:
 * #define MQTT_SERVER     "mqtt.example.com"
 * #define MQTT_PORT       1883
 * #define MQTT_USER       "myuser"
 * #define MQTT_PASS       "mypassword"
 * #define MQTT_CLIENT_ID  "ESP32_Device_1"
 */
#define MQTT_SERVER "mqtt.example.com"
#define MQTT_PORT 1883
#define MQTT_USER "mqtt_username"
#define MQTT_PASS "mqtt_password"
#define MQTT_CLIENT_ID "ESP32_Device_1"

// ═══════════════════════════════════════════════════════════════════════════
// API KEYS (if using cloud services)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Third-Party API Keys
 *
 * Add API keys for services you're using:
 * - Weather APIs (OpenWeatherMap, etc.)
 * - IoT platforms (ThingSpeak, Blynk, etc.)
 * - Cloud storage (AWS, Azure, etc.)
 * - Notification services (Pushover, Telegram, etc.)
 *
 * SECURITY:
 * - Never commit API keys to public repositories
 * - Rotate keys regularly
 * - Use read-only keys when possible
 * - Monitor API usage for unauthorized access
 *
 * EXAMPLE:
 * #define OPENWEATHER_API_KEY "abc123def456ghi789"
 * #define THINGSPEAK_API_KEY  "ABCDEFGHIJKLMNOP"
 * #define BLYNK_AUTH_TOKEN    "abcdef1234567890"
 */
#define API_KEY "your_api_key_here"

// ═══════════════════════════════════════════════════════════════════════════
// ESP-NOW PEER DEVICE MAC ADDRESSES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Peer Device MAC Addresses
 *
 * HOW TO GET MAC ADDRESS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. Upload code to your ESP32 device
 * 2. Open Serial Monitor (115200 baud)
 * 3. Look for this line on boot:
 *    "MAC: AA:BB:CC:DD:EE:FF"
 * 4. Copy the MAC address
 * 5. Convert to array format: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
 * 6. Paste below as PEER_MAC_1, PEER_MAC_2, etc.
 *
 * EXAMPLE:
 * If Serial Monitor shows: "MAC: 24:6F:28:12:34:56"
 * Use: #define PEER_MAC_1 {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56}
 *
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * PEER_MAC_1: First peer device (e.g., Kitchen sensor)
 * PEER_MAC_2: Second peer device (e.g., Garage controller)
 * PEER_MAC_3: Third peer device (e.g., Garden sensor)
 *
 * You can add up to MAX_ESPNOW_PEERS devices (default: 5)
 *
 * BROADCAST ADDRESS:
 * {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} sends to all nearby ESP32 devices
 * Use for testing or when you don't know peer MAC addresses
 *
 * NAMING CONVENTION:
 * Give meaningful names to help identify devices:
 * - Location-based: Kitchen, Garage, Garden
 * - Function-based: TempSensor, Relay_Controller, Camera_Monitor
 * - Number-based: Device_1, Device_2, Device_3
 */

/**
 * Device 1 - Change this to actual MAC address
 * Example: Kitchen Temperature Sensor
 *
 * To find MAC, upload to device and check Serial Monitor
 */
#define PEER_MAC_1 {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC}

/**
 * Device 2 - Change this to actual MAC address
 * Example: Garage Door Controller
 */
#define PEER_MAC_2 {0x24, 0x6F, 0x28, 0xDD, 0xEE, 0xFF}

/**
 * Device 3 - Change this to actual MAC address
 * Example: Garden Irrigation System
 */
#define PEER_MAC_3 {0x24, 0x6F, 0x28, 0x11, 0x22, 0x33}

/**
 * Device 4 - Change this to actual MAC address
 * Example: Living Room Display
 */
#define PEER_MAC_4 {0x24, 0x6F, 0x28, 0x44, 0x55, 0x66}

/**
 * Device 5 - Change this to actual MAC address
 * Example: Backup Sensor Node
 */
#define PEER_MAC_5 {0x24, 0x6F, 0x28, 0x77, 0x88, 0x99}

// ═══════════════════════════════════════════════════════════════════════════
// TELEGRAM BOT (Optional - for notifications)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Telegram Bot Configuration
 *
 * Telegram bots can send notifications to your phone:
 * - Sensor alerts (high temperature, motion detected, etc.)
 * - System status (device online/offline, errors)
 * - Photos from ESP32-CAM
 * - Command responses
 *
 * HOW TO CREATE A TELEGRAM BOT:
 * ═══════════════════════════════════════════════════════════════════════════
 * 1. Open Telegram app
 * 2. Search for @BotFather
 * 3. Send /newbot command
 * 4. Follow instructions to create bot
 * 5. Copy the Bot Token (looks like: 123456789:ABCdefGHIjklMNOpqrsTUVwxyz)
 * 6. To get CHAT_ID:
 *    a. Send a message to your bot
 *    b. Visit: https://api.telegram.org/bot<TOKEN>/getUpdates
 *    c. Look for "chat":{"id":123456789}
 *    d. Copy the chat ID number
 *
 * EXAMPLE:
 * #define TELEGRAM_BOT_TOKEN "123456789:ABCdefGHIjklMNOpqrsTUVwxyz"
 * #define TELEGRAM_CHAT_ID   "123456789"
 */
#define TELEGRAM_BOT_TOKEN "your_bot_token_here"
#define TELEGRAM_CHAT_ID "your_chat_id_here"

// ═══════════════════════════════════════════════════════════════════════════
// DEVICE-SPECIFIC SETTINGS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Device Location (for identification)
 *
 * Used in:
 * - Web interface display
 * - MQTT topic names
 * - Log file naming
 * - Status messages
 *
 * EXAMPLES:
 * - "Kitchen"
 * - "Garage"
 * - "Living Room"
 * - "Garden"
 * - "Office"
 */
#define DEVICE_LOCATION "Kitchen"

/**
 * Time Zone Offset (hours from UTC)
 *
 * Used for accurate timestamps in logs and displays
 *
 * EXAMPLES:
 * - PST (Pacific): -8
 * - EST (Eastern): -5
 * - GMT (London): 0
 * - CET (Central Europe): +1
 * - IST (India): +5.5
 * - JST (Japan): +9
 * - AEST (Sydney): +10
 *
 * Adjust for daylight saving time as needed
 */
#define TIMEZONE_OFFSET 0

/**
 * NTP Server (for time synchronization)
 *
 * Network Time Protocol servers provide accurate time
 *
 * POPULAR NTP SERVERS:
 * - pool.ntp.org (general purpose)
 * - time.google.com (Google)
 * - time.windows.com (Microsoft)
 * - time.nist.gov (US government)
 *
 * Use servers geographically close to you for best accuracy
 */
#define NTP_SERVER "pool.ntp.org"

// ═══════════════════════════════════════════════════════════════════════════
// ENCRYPTION KEYS (Advanced - for secure communication)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * AES Encryption Key
 *
 * Used for encrypting sensitive data in:
 * - ESP-NOW messages
 * - Stored configurations
 * - Cloud communications
 *
 * SECURITY REQUIREMENTS:
 * - Must be 16, 24, or 32 bytes (128, 192, or 256-bit AES)
 * - Use random bytes (not text passwords)
 * - Each device should have unique key
 * - Store securely (not in source code ideally)
 *
 * GENERATING RANDOM KEY:
 * Use online tools or:
 *   Python: import os; print(os.urandom(16).hex())
 *   Linux: openssl rand -hex 16
 *
 * EXAMPLE (16-byte key):
 * uint8_t encryptionKey[16] = {
 *     0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
 *     0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
 * };
 */
uint8_t encryptionKey[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // ⚠️ CHANGE THIS! All zeros is not secure!

// ═══════════════════════════════════════════════════════════════════════════
// VALIDATION AND COMPILE-TIME CHECKS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Check if credentials have been updated from defaults
 * Warns during compilation if default values are still being used
 */

// Check WiFi credentials
#if !defined(WIFI_SSID_1) || !defined(WIFI_PASS_1)
#error "WiFi credentials not defined! Please set WIFI_SSID_1 and WIFI_PASS_1"
#endif

// Warn if using default passwords
// Note: Compile-time checks for string comparison not supported in C++
// These checks would need to be implemented at runtime

// Check password length
// Note: Compile-time checks for string length not supported in C++
// These checks would need to be implemented at runtime

#endif // CREDENTIALS_H

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * SECURITY CHECKLIST
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Before deploying, verify:
 *
 * ☐ Changed all default passwords
 * ☐ Added credentials.h to .gitignore
 * ☐ Using strong passwords (12+ characters, mixed case, numbers, symbols)
 * ☐ Different passwords for WiFi, web interface, and MQTT
 * ☐ Updated MAC addresses with actual device addresses
 * ☐ Generated random encryption key (not all zeros)
 * ☐ Removed or secured any API keys
 * ☐ Set appropriate timezone offset
 * ☐ Tested connectivity with new credentials
 * ☐ Documented credentials securely (password manager)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * RECOVERY PROCEDURES
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * IF YOU FORGET CREDENTIALS:
 *
 * WiFi Password:
 * - Device will start in AP mode if can't connect
 * - Connect to AP_SSID (from config.h)
 * - Access web interface to reconfigure
 *
 * Web Interface Password:
 * - Re-upload code with new password
 * - Or use serial commands (if implemented)
 * - Or reset to factory defaults
 *
 * MAC Address:
 * - Always printed on Serial Monitor at boot
 * - Can't be changed (hardware address)
 * - Use Serial Monitor to retrieve
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */
