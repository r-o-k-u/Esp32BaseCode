/**
 * @file CameraManager.h
 * @brief Camera management and control system (ESP32-CAM only)
 * @author Your Name
 * @version 2.0
 */

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "../config.h"
#include <Arduino.h>

#if ENABLE_CAMERA

#include <esp_camera.h>
#include <FS.h>
#include <SPIFFS.h>

class CameraManager
{
private:
    bool initialized;
    bool cameraReady;
    camera_config_t config;

    // Camera settings
    int imageQuality;
    int frameSize;
    int brightness;
    int contrast;
    int saturation;
    int sharpness;
    int specialEffect;
    int whiteBalance;
    int aeLevel;

public:
    CameraManager();
    ~CameraManager();

    bool begin();
    bool captureImage(uint8_t **buffer, size_t *bufferSize);
    bool captureImageToFile(const char *filename);
    bool captureJPEG(uint8_t **buffer, size_t *bufferSize);

    // Camera control
    bool setResolution(int width, int height);
    bool setFrameSize(framesize_t size);
    bool setImageQuality(int quality);
    bool setBrightness(int level);
    bool setContrast(int level);
    bool setSaturation(int level);
    bool setSharpness(int level);
    bool setSpecialEffect(int effect);
    bool setWhiteBalance(int wb);
    bool setAeLevel(int level);

    // Flash control
    bool enableFlash();
    bool disableFlash();
    bool setFlashMode(int mode);

    // Stream control
    bool startStream();
    bool stopStream();
    bool isStreaming();

    // Status and information
    String getCameraInfo();
    bool isCameraReady();
    size_t getFreeHeap();

    // Utilities
    void dumpCameraConfig();
    bool testCamera();

private:
    bool initializeCamera();
    bool configureCamera();
    bool validateCameraConfig();
    void applyCameraSettings();
    bool saveImageToFile(const uint8_t *buffer, size_t size, const char *filename);
    void logCameraError(const char *operation, int error);
};

#endif // ENABLE_CAMERA

#endif // CAMERA_MANAGER_H
