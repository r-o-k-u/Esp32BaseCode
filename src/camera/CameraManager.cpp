/**
 * @file CameraManager.cpp
 * @brief Camera management and control system implementation
 * @author Your Name
 * @version 2.0
 */

#include "CameraManager.h"
#include "../utils/Logger.h"

#if ENABLE_CAMERA

CameraManager::CameraManager()
    : initialized(false), cameraReady(false), imageQuality(10), frameSize(FRAME_240X240),
      brightness(0), contrast(0), saturation(0), sharpness(0), specialEffect(0),
      whiteBalance(0), aeLevel(0)
{
    // Initialize camera configuration
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = (framesize_t)frameSize;
    config.jpeg_quality = imageQuality;
    config.fb_count = 1;
}

CameraManager::~CameraManager()
{
    if (initialized)
    {
        DEBUG_PRINTLN("[CAMERA] Camera Manager deinitialized");
    }
}

bool CameraManager::begin()
{
    DEBUG_PRINTLN("[CAMERA] Initializing Camera Manager...");

    if (!initializeCamera())
    {
        DEBUG_PRINTLN("[CAMERA] Failed to initialize camera");
        return false;
    }

    if (!configureCamera())
    {
        DEBUG_PRINTLN("[CAMERA] Failed to configure camera");
        return false;
    }

    initialized = true;
    cameraReady = true;

    DEBUG_PRINTLN("[CAMERA] Camera Manager initialized successfully");
    return true;
}

bool CameraManager::initializeCamera()
{
    // Initialize SPIFFS for image storage
    if (!SPIFFS.begin(true))
    {
        DEBUG_PRINTLN("[CAMERA] Failed to mount SPIFFS");
        return false;
    }

    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        logCameraError("Camera initialization", err);
        return false;
    }

    // Test camera
    if (!testCamera())
    {
        DEBUG_PRINTLN("[CAMERA] Camera test failed");
        return false;
    }

    return true;
}

bool CameraManager::configureCamera()
{
    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
    {
        DEBUG_PRINTLN("[CAMERA] Camera sensor not found");
        return false;
    }

    // Apply initial settings
    applyCameraSettings();

    // Set JPEG quality
    s->set_quality(s, imageQuality);

    // Set frame size
    s->set_framesize(s, (framesize_t)frameSize);

    DEBUG_PRINTLN("[CAMERA] Camera configured successfully");
    return true;
}

bool CameraManager::captureImage(uint8_t **buffer, size_t *bufferSize)
{
    if (!cameraReady)
    {
        DEBUG_PRINTLN("[CAMERA] Camera not ready");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        DEBUG_PRINTLN("[CAMERA] Failed to capture image");
        return false;
    }

    *buffer = (uint8_t *)malloc(fb->len);
    if (*buffer == nullptr)
    {
        esp_camera_fb_return(fb);
        DEBUG_PRINTLN("[CAMERA] Failed to allocate buffer");
        return false;
    }

    memcpy(*buffer, fb->buf, fb->len);
    *bufferSize = fb->len;

    esp_camera_fb_return(fb);
    return true;
}

bool CameraManager::captureImageToFile(const char *filename)
{
    uint8_t *buffer;
    size_t bufferSize;

    if (!captureImage(&buffer, &bufferSize))
    {
        return false;
    }

    bool success = saveImageToFile(buffer, bufferSize, filename);
    free(buffer);

    return success;
}

bool CameraManager::captureJPEG(uint8_t **buffer, size_t *bufferSize)
{
    return captureImage(buffer, bufferSize);
}

bool CameraManager::setResolution(int width, int height)
{
    // Find appropriate frame size
    framesize_t size = FRAMESIZE_96X96;

    if (width >= 1600 && height >= 1200)
        size = FRAMESIZE_1600X1200;
    else if (width >= 1280 && height >= 1024)
        size = FRAMESIZE_1280X1024;
    else if (width >= 1024 && height >= 768)
        size = FRAMESIZE_1024X768;
    else if (width >= 800 && height >= 600)
        size = FRAMESIZE_800X600;
    else if (width >= 640 && height >= 480)
        size = FRAMESIZE_QVGA;
    else if (width >= 320 && height >= 240)
        size = FRAMESIZE_QQVGA;
    else if (width >= 240 && height >= 240)
        size = FRAMESIZE_240X240;
    else if (width >= 176 && height >= 144)
        size = FRAMESIZE_QCIF;
    else if (width >= 160 && height >= 120)
        size = FRAMESIZE_QQVGA;
    else
        size = FRAMESIZE_96X96;

    return setFrameSize(size);
}

bool CameraManager::setFrameSize(framesize_t size)
{
    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_framesize(s, size);
    if (err == ESP_OK)
    {
        frameSize = size;
        DEBUG_PRINTLN("[CAMERA] Frame size set to: " + String(frameSize));
        return true;
    }

    logCameraError("Set frame size", err);
    return false;
}

bool CameraManager::setImageQuality(int quality)
{
    if (quality < 0 || quality > 63)
        return false;

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_quality(s, quality);
    if (err == ESP_OK)
    {
        imageQuality = quality;
        DEBUG_PRINTLN("[CAMERA] Image quality set to: " + String(quality));
        return true;
    }

    logCameraError("Set image quality", err);
    return false;
}

bool CameraManager::setBrightness(int level)
{
    if (level < -2 || level > 2)
        return false;

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_brightness(s, level);
    if (err == ESP_OK)
    {
        brightness = level;
        DEBUG_PRINTLN("[CAMERA] Brightness set to: " + String(level));
        return true;
    }

    logCameraError("Set brightness", err);
    return false;
}

bool CameraManager::setContrast(int level)
{
    if (level < -2 || level > 2)
        return false;

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_contrast(s, level);
    if (err == ESP_OK)
    {
        contrast = level;
        DEBUG_PRINTLN("[CAMERA] Contrast set to: " + String(level));
        return true;
    }

    logCameraError("Set contrast", err);
    return false;
}

bool CameraManager::setSaturation(int level)
{
    if (level < -2 || level > 2)
        return false;

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_saturation(s, level);
    if (err == ESP_OK)
    {
        saturation = level;
        DEBUG_PRINTLN("[CAMERA] Saturation set to: " + String(level));
        return true;
    }

    logCameraError("Set saturation", err);
    return false;
}

bool CameraManager::setSharpness(int level)
{
    if (level < -2 || level > 2)
        return false;

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_sharpness(s, level);
    if (err == ESP_OK)
    {
        sharpness = level;
        DEBUG_PRINTLN("[CAMERA] Sharpness set to: " + String(level));
        return true;
    }

    logCameraError("Set sharpness", err);
    return false;
}

bool CameraManager::setSpecialEffect(int effect)
{
    if (effect < 0 || effect > 6)
        return false;

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_special_effect(s, effect);
    if (err == ESP_OK)
    {
        specialEffect = effect;
        DEBUG_PRINTLN("[CAMERA] Special effect set to: " + String(effect));
        return true;
    }

    logCameraError("Set special effect", err);
    return false;
}

bool CameraManager::setWhiteBalance(int wb)
{
    if (wb < 0 || wb > 4)
        return false;

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_wb_mode(s, wb);
    if (err == ESP_OK)
    {
        whiteBalance = wb;
        DEBUG_PRINTLN("[CAMERA] White balance set to: " + String(wb));
        return true;
    }

    logCameraError("Set white balance", err);
    return false;
}

bool CameraManager::setAeLevel(int level)
{
    if (level < -2 || level > 2)
        return false;

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
        return false;

    esp_err_t err = s->set_ae_level(s, level);
    if (err == ESP_OK)
    {
        aeLevel = level;
        DEBUG_PRINTLN("[CAMERA] AE level set to: " + String(level));
        return true;
    }

    logCameraError("Set AE level", err);
    return false;
}

bool CameraManager::enableFlash()
{
    if (FLASH_PIN != -1)
    {
        pinMode(FLASH_PIN, OUTPUT);
        digitalWrite(FLASH_PIN, HIGH);
        DEBUG_PRINTLN("[CAMERA] Flash enabled");
        return true;
    }
    return false;
}

bool CameraManager::disableFlash()
{
    if (FLASH_PIN != -1)
    {
        pinMode(FLASH_PIN, OUTPUT);
        digitalWrite(FLASH_PIN, LOW);
        DEBUG_PRINTLN("[CAMERA] Flash disabled");
        return true;
    }
    return false;
}

bool CameraManager::setFlashMode(int mode)
{
    // Mode 0: Off, Mode 1: On, Mode 2: Auto
    switch (mode)
    {
    case 0:
        return disableFlash();
    case 1:
        return enableFlash();
    case 2:
        // Auto mode would require light sensor integration
        return enableFlash();
    default:
        return false;
    }
}

bool CameraManager::startStream()
{
    // Stream start would be handled by web server
    DEBUG_PRINTLN("[CAMERA] Stream started");
    return true;
}

bool CameraManager::stopStream()
{
    // Stream stop would be handled by web server
    DEBUG_PRINTLN("[CAMERA] Stream stopped");
    return true;
}

bool CameraManager::isStreaming()
{
    // Check stream status
    return false; // Placeholder
}

String CameraManager::getCameraInfo()
{
    String info = "{\"camera\":{";
    info += "\"ready\":" + String(cameraReady ? "true" : "false") + ",";
    info += "\"quality\":" + String(imageQuality) + ",";
    info += "\"frameSize\":" + String(frameSize) + ",";
    info += "\"brightness\":" + String(brightness) + ",";
    info += "\"contrast\":" + String(contrast) + ",";
    info += "\"saturation\":" + String(saturation) + ",";
    info += "\"sharpness\":" + String(sharpness) + ",";
    info += "\"specialEffect\":" + String(specialEffect) + ",";
    info += "\"whiteBalance\":" + String(whiteBalance) + ",";
    info += "\"aeLevel\":" + String(aeLevel) + ",";
    info += "\"freeHeap\":" + String(getFreeHeap());
    info += "}}";
    return info;
}

bool CameraManager::isCameraReady()
{
    return cameraReady;
}

size_t CameraManager::getFreeHeap()
{
    return ESP.getFreeHeap();
}

void CameraManager::dumpCameraConfig()
{
    DEBUG_PRINTLN("[CAMERA] Camera Configuration:");
    DEBUG_PRINTLN("  Quality: " + String(imageQuality));
    DEBUG_PRINTLN("  Frame Size: " + String(frameSize));
    DEBUG_PRINTLN("  Brightness: " + String(brightness));
    DEBUG_PRINTLN("  Contrast: " + String(contrast));
    DEBUG_PRINTLN("  Saturation: " + String(saturation));
    DEBUG_PRINTLN("  Sharpness: " + String(sharpness));
    DEBUG_PRINTLN("  Special Effect: " + String(specialEffect));
    DEBUG_PRINTLN("  White Balance: " + String(whiteBalance));
    DEBUG_PRINTLN("  AE Level: " + String(aeLevel));
}

bool CameraManager::testCamera()
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        DEBUG_PRINTLN("[CAMERA] Camera test failed - no frame buffer");
        return false;
    }

    DEBUG_PRINTLN("[CAMERA] Camera test successful - frame size: " + String(fb->len) + " bytes");
    esp_camera_fb_return(fb);
    return true;
}

bool CameraManager::saveImageToFile(const uint8_t *buffer, size_t size, const char *filename)
{
    File file = SPIFFS.open(filename, FILE_WRITE);
    if (!file)
    {
        DEBUG_PRINTLN("[CAMERA] Failed to open file for writing: " + String(filename));
        return false;
    }

    size_t written = file.write(buffer, size);
    file.close();

    if (written == size)
    {
        DEBUG_PRINTLN("[CAMERA] Image saved to: " + String(filename));
        return true;
    }
    else
    {
        DEBUG_PRINTLN("[CAMERA] Failed to write complete image to: " + String(filename));
        return false;
    }
}

void CameraManager::logCameraError(const char *operation, int error)
{
    DEBUG_PRINTLN("[CAMERA] Error in " + String(operation) + ": " + String(error));
}

#endif // ENABLE_CAMERA
