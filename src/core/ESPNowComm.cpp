/**
 * @file ESPNowComm.cpp
 * @brief Implementation of ESP-NOW communication manager
 */

#include "ESPNowComm.h"
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>

// Static member initialization
ESPNowComm espnowComm;
static ESPNowComm *s_instance = nullptr;

/**
 * @brief Constructor
 */
ESPNowComm::ESPNowComm()
{
    peerCount = 0;
    totalSent = 0;
    totalReceived = 0;
    totalFailed = 0;
    recvCallback = nullptr;
    sentCallback = nullptr;
    s_instance = this;

    // Initialize peer list
    for (int i = 0; i < MAX_ESPNOW_PEERS; i++)
    {
        peers[i].active = false;
    }
}

/**
 * @brief Initialize ESP-NOW
 * @return true if successful
 */
bool ESPNowComm::begin()
{
    DEBUG_PRINTLN("Initializing ESP-NOW...");

    // Set device in STA mode
    WiFi.mode(WIFI_STA);

    // Print MAC address
    uint8_t mac[6];
    WiFi.macAddress(mac);
    DEBUG_PRINTF("Device MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        DEBUG_PRINTLN("ERROR: ESP-NOW init failed");
        return false;
    }

    DEBUG_PRINTLN("ESP-NOW initialized successfully");

    // Register callbacks
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);

    return true;
}

/**
 * @brief Deinitialize ESP-NOW
 */
void ESPNowComm::end()
{
    esp_now_deinit();
    DEBUG_PRINTLN("ESP-NOW deinitialized");
}

/**
 * @brief Add a peer device
 * @param mac MAC address of peer
 * @param name Optional name for the peer
 * @return true if successful
 */
bool ESPNowComm::addPeer(const uint8_t *mac, const char *name)
{
    if (peerCount >= MAX_ESPNOW_PEERS)
    {
        DEBUG_PRINTLN("ERROR: Max peers reached");
        return false;
    }

    // Check if already registered
    if (isPeerRegistered(mac))
    {
        DEBUG_PRINTLN("Peer already registered");
        return true;
    }

    // Add peer to ESP-NOW
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = ESPNOW_CHANNEL;
    peerInfo.encrypt = false; // No encryption for simplicity

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        DEBUG_PRINTLN("ERROR: Failed to add peer");
        return false;
    }

    // Add to our peer list
    memcpy(peers[peerCount].mac, mac, 6);
    strncpy(peers[peerCount].name, name, 31);
    peers[peerCount].name[31] = '\0';
    peers[peerCount].active = true;
    peers[peerCount].lastSeen = millis();
    peers[peerCount].messagesSent = 0;
    peers[peerCount].messagesReceived = 0;

    peerCount++;

    DEBUG_PRINTF("Peer added: %s (%s)\n", name, getMacString(mac).c_str());
    return true;
}

/**
 * @brief Remove a peer
 * @param mac MAC address of peer
 * @return true if successful
 */
bool ESPNowComm::removePeer(const uint8_t *mac)
{
    // Find peer
    int index = -1;
    for (int i = 0; i < peerCount; i++)
    {
        if (memcmp(peers[i].mac, mac, 6) == 0)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
        return false;

    // Remove from ESP-NOW
    esp_now_del_peer(mac);

    // Remove from our list (shift array)
    for (int i = index; i < peerCount - 1; i++)
    {
        peers[i] = peers[i + 1];
    }
    peerCount--;

    DEBUG_PRINTLN("Peer removed");
    return true;
}

/**
 * @brief Check if peer is registered
 */
bool ESPNowComm::isPeerRegistered(const uint8_t *mac)
{
    for (int i = 0; i < peerCount; i++)
    {
        if (memcmp(peers[i].mac, mac, 6) == 0 && peers[i].active)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Get peer information
 */
PeerInfo *ESPNowComm::getPeerInfo(uint8_t index)
{
    if (index >= peerCount)
        return nullptr;
    return &peers[index];
}

/**
 * @brief Update peer activity timestamp
 */
void ESPNowComm::updatePeerActivity(const uint8_t *mac)
{
    for (int i = 0; i < peerCount; i++)
    {
        if (memcmp(peers[i].mac, mac, 6) == 0)
        {
            peers[i].lastSeen = millis();
            peers[i].messagesReceived++;
            break;
        }
    }
}

/**
 * @brief Send message to specific peer
 * @param mac Destination MAC address
 * @param type Message type
 * @param data Data to send (JSON string)
 * @return true if successful
 */
bool ESPNowComm::sendMessage(const uint8_t *mac, uint8_t type, const char *data)
{
    ESPNowMessage msg;

    // Fill message structure
    msg.type = type;
    WiFi.macAddress(msg.sender);
    msg.timestamp = millis();

    // Copy data
    size_t dataLen = strlen(data);
    if (dataLen > sizeof(msg.data) - 1)
    {
        dataLen = sizeof(msg.data) - 1;
        DEBUG_PRINTLN("WARNING: Data truncated");
    }
    memcpy(msg.data, data, dataLen);
    msg.data[dataLen] = '\0';
    msg.dataLen = dataLen;

    // Calculate checksum
    msg.checksum = calculateChecksum(&msg);

    // Send message
    esp_err_t result = esp_now_send(mac, (uint8_t *)&msg, sizeof(msg));

    if (result == ESP_OK)
    {
        totalSent++;

        // Update peer statistics
        for (int i = 0; i < peerCount; i++)
        {
            if (memcmp(peers[i].mac, mac, 6) == 0)
            {
                peers[i].messagesSent++;
                break;
            }
        }

#if DEBUG_ESPNOW
        DEBUG_PRINTF("Message sent to %s (type:%d)\n", getMacString(mac).c_str(), type);
#endif
        return true;
    }
    else
    {
        totalFailed++;
        DEBUG_PRINTLN("ERROR: Message send failed");
        return false;
    }
}

/**
 * @brief Send message to all peers
 */
bool ESPNowComm::sendToAllPeers(uint8_t type, const char *data)
{
    bool success = true;
    for (int i = 0; i < peerCount; i++)
    {
        if (peers[i].active)
        {
            if (!sendMessage(peers[i].mac, type, data))
            {
                success = false;
            }
        }
    }
    return success;
}

/**
 * @brief Send sensor data
 */
bool ESPNowComm::sendSensorData(const uint8_t *mac, const char *jsonData)
{
    return sendMessage(mac, MSG_SENSOR_DATA, jsonData);
}

/**
 * @brief Send actuator command
 */
bool ESPNowComm::sendActuatorCommand(const uint8_t *mac, const char *command)
{
    return sendMessage(mac, MSG_ACTUATOR_CMD, command);
}

/**
 * @brief Send status update
 */
bool ESPNowComm::sendStatus(const uint8_t *mac)
{
    StaticJsonDocument<200> doc;
    doc["device"] = DEVICE_NAME;
    doc["version"] = FIRMWARE_VERSION;
    doc["uptime"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();

    char buffer[200];
    serializeJson(doc, buffer);

    return sendMessage(mac, MSG_STATUS, buffer);
}

/**
 * @brief Send alert message
 */
bool ESPNowComm::sendAlert(const uint8_t *mac, const char *alertMsg)
{
    return sendMessage(mac, MSG_ALERT, alertMsg);
}

/**
 * @brief Calculate simple checksum
 */
uint8_t ESPNowComm::calculateChecksum(const ESPNowMessage *msg)
{
    uint8_t sum = 0;
    sum += msg->type;
    for (int i = 0; i < 6; i++)
        sum += msg->sender[i];
    sum += (msg->timestamp & 0xFF);
    sum += msg->dataLen;
    for (int i = 0; i < msg->dataLen; i++)
        sum += msg->data[i];
    return sum;
}

/**
 * @brief Validate checksum
 */
bool ESPNowComm::validateChecksum(const ESPNowMessage *msg)
{
    return (calculateChecksum(msg) == msg->checksum);
}

/**
 * @brief Callback when data is sent
 */
void ESPNowComm::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
#if DEBUG_ESPNOW
    DEBUG_PRINTF("Send status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
#endif

    if (s_instance && s_instance->sentCallback)
    {
        s_instance->sentCallback(mac_addr, status == ESP_NOW_SEND_SUCCESS);
    }
}

/**
 * @brief Callback when data is received
 */
void ESPNowComm::onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    if (!s_instance)
        return;

    // Parse message
    ESPNowMessage *msg = (ESPNowMessage *)data;

    // Validate checksum
    if (!validateChecksum(msg))
    {
        DEBUG_PRINTLN("ERROR: Checksum validation failed");
        return;
    }

    // Update statistics
    s_instance->totalReceived++;
    s_instance->updatePeerActivity(mac_addr);

#if DEBUG_ESPNOW
    DEBUG_PRINTF("Message received from %s (type:%d)\n",
                 s_instance->getMacString(mac_addr).c_str(), msg->type);
    DEBUG_PRINTF("Data: %s\n", msg->data);
#endif

    // Call user callback
    if (s_instance->recvCallback)
    {
        s_instance->recvCallback(mac_addr, msg->data, msg->type);
    }
}

/**
 * @brief Get MAC address as string
 */
String ESPNowComm::getMacString(const uint8_t *mac)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

/**
 * @brief Get own MAC address
 */
void ESPNowComm::getOwnMac(uint8_t *mac)
{
    WiFi.macAddress(mac);
}

/**
 * @brief Print list of all peers
 */
void ESPNowComm::printPeerList()
{
    DEBUG_PRINTLN("\n=== Registered Peers ===");
    for (int i = 0; i < peerCount; i++)
    {
        DEBUG_PRINTF("%d. %s - %s (Sent:%d, Recv:%d, LastSeen:%lu)\n",
                     i + 1,
                     peers[i].name,
                     getMacString(peers[i].mac).c_str(),
                     peers[i].messagesSent,
                     peers[i].messagesReceived,
                     peers[i].lastSeen);
    }
    DEBUG_PRINTLN("=======================\n");
}

/**
 * @brief Get communication statistics
 */
void ESPNowComm::getStatistics(uint32_t &sent, uint32_t &received, uint32_t &failed)
{
    sent = totalSent;
    received = totalReceived;
    failed = totalFailed;
}

/**
 * @brief Reset statistics
 */
void ESPNowComm::resetStatistics()
{
    totalSent = 0;
    totalReceived = 0;
    totalFailed = 0;

    for (int i = 0; i < peerCount; i++)
    {
        peers[i].messagesSent = 0;
        peers[i].messagesReceived = 0;
    }
}

/**
 * @brief Check peer activity and mark inactive peers
 * @param timeout Timeout in milliseconds
 */
void ESPNowComm::checkPeerActivity(uint32_t timeout)
{
    uint32_t now = millis();
    for (int i = 0; i < peerCount; i++)
    {
        if (now - peers[i].lastSeen > timeout)
        {
            peers[i].active = false;
            DEBUG_PRINTF("Peer %s marked inactive\n", peers[i].name);
        }
    }
}
