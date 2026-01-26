/**
 * @file ESPNowComm.h
 * @brief ESP-NOW communication manager for device-to-device communication
 *
 * Handles bi-directional communication between ESP32 devices using ESP-NOW protocol.
 * Supports multiple peers, callbacks, and automatic retry logic.
 */

#ifndef ESPNOW_COMM_H
#define ESPNOW_COMM_H
#include "../config.h"
#include <esp_now.h>

// Message types
enum MessageType
{
    MSG_SENSOR_DATA = 0,
    MSG_ACTUATOR_CMD = 1,
    MSG_STATUS = 2,
    MSG_CONFIG = 3,
    MSG_ACK = 4,
    MSG_ALERT = 5,
    MSG_SYNC = 6,
    MSG_CUSTOM = 99
};

// Message structure (max 250 bytes for ESP-NOW)
struct ESPNowMessage
{
    uint8_t type;       // Message type
    uint8_t sender[6];  // Sender MAC address
    uint32_t timestamp; // Message timestamp
    uint8_t dataLen;    // Data length
    char data[230];     // Actual data (JSON string)
    uint8_t checksum;   // Simple checksum for validation
};

// Peer information
struct PeerInfo
{
    uint8_t mac[6];
    char name[32];
    bool active;
    uint32_t lastSeen;
    uint16_t messagesSent;
    uint16_t messagesReceived;
};

// Callback function types
typedef void (*OnDataRecvCallback)(const uint8_t *mac, const char *data, uint8_t type);
typedef void (*OnDataSentCallback)(const uint8_t *mac, bool success);

class ESPNowComm
{
private:
    PeerInfo peers[MAX_ESPNOW_PEERS];
    uint8_t peerCount;
    OnDataRecvCallback recvCallback;
    OnDataSentCallback sentCallback;

    // Statistics
    uint32_t totalSent;
    uint32_t totalReceived;
    uint32_t totalFailed;

    // Internal methods
    static uint8_t calculateChecksum(const ESPNowMessage *msg);
    static bool validateChecksum(const ESPNowMessage *msg);

    // Static callbacks for ESP-NOW
    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

public:
    ESPNowComm();

    // Initialization
    bool begin();
    void end();

    // Peer management
    bool addPeer(const uint8_t *mac, const char *name = "");
    bool removePeer(const uint8_t *mac);
    bool isPeerRegistered(const uint8_t *mac);
    uint8_t getPeerCount() { return peerCount; }
    PeerInfo *getPeerInfo(uint8_t index);
    void updatePeerActivity(const uint8_t *mac);

    // Sending data
    bool sendMessage(const uint8_t *mac, uint8_t type, const char *data);
    bool sendToAllPeers(uint8_t type, const char *data);
    bool sendSensorData(const uint8_t *mac, const char *jsonData);
    bool sendActuatorCommand(const uint8_t *mac, const char *command);
    bool sendStatus(const uint8_t *mac);
    bool sendAlert(const uint8_t *mac, const char *alertMsg);

    // Callbacks
    void setOnDataRecv(OnDataRecvCallback callback) { recvCallback = callback; }
    void setOnDataSent(OnDataSentCallback callback) { sentCallback = callback; }

    // Utility
    String getMacString(const uint8_t *mac);
    void getOwnMac(uint8_t *mac);
    void printPeerList();
    void getStatistics(uint32_t &sent, uint32_t &received, uint32_t &failed);
    void resetStatistics();

    // Keep alive
    void checkPeerActivity(uint32_t timeout = 60000); // 60 seconds timeout
};

extern ESPNowComm espnowComm; // Global instance

#endif // ESPNOW_COMM_H
