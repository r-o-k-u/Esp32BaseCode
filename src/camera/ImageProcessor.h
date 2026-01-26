/**
 * @file ImageProcessor.h
 * @brief Image processing and analysis utilities
 * @author Your Name
 * @version 2.0
 */

#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include "../config.h"
#include <Arduino.h>

#if ENABLE_CAMERA

#include <esp_camera.h>
#include <FS.h>
#include <SPIFFS.h>

class ImageProcessor
{
private:
    bool initialized;

    // Processing settings
    int threshold;
    int blurRadius;
    int edgeThreshold;

    // Analysis results
    struct MotionDetection
    {
        bool motionDetected;
        int motionPixels;
        int totalPixels;
        float motionPercentage;
        unsigned long timestamp;
    };

    struct FaceDetection
    {
        bool faceDetected;
        int faceCount;
        int centerX;
        int centerY;
        int width;
        int height;
        unsigned long timestamp;
    };

    MotionDetection lastMotion;
    FaceDetection lastFace;

public:
    ImageProcessor();
    ~ImageProcessor();

    bool begin();

    // Basic image operations
    bool resizeImage(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize, int newWidth, int newHeight);
    bool convertToGrayscale(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize);
    bool applyFilter(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize, const char *filterType);

    // Image analysis
    bool detectMotion(const uint8_t *currentFrame, size_t currentSize, const uint8_t *previousFrame, size_t previousSize, int threshold = 30);
    bool detectFaces(const uint8_t *image, size_t imageSize);
    bool detectObjects(const uint8_t *image, size_t imageSize);
    bool analyzeBrightness(const uint8_t *image, size_t imageSize, float &averageBrightness, float &contrast);

    // Image enhancement
    bool enhanceImage(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize);
    bool adjustBrightness(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize, int brightness);
    bool adjustContrast(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize, float contrast);

    // Compression and format conversion
    bool compressJPEG(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize, int quality = 80);
    bool convertToPNG(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize);
    bool convertToBMP(const uint8_t *input, size_t inputSize, uint8_t **output, size_t *outputSize);

    // File operations
    bool saveProcessedImage(const uint8_t *image, size_t size, const char *filename);
    bool loadImageFromFile(const char *filename, uint8_t **buffer, size_t *size);
    bool deleteImage(const char *filename);

    // Batch processing
    bool processImageDirectory(const char *directory, const char *outputDirectory);
    bool batchConvertFormat(const char *inputDir, const char *outputDir, const char *format);

    // Status and results
    String getMotionStatus();
    String getFaceStatus();
    bool hasMotion();
    bool hasFaces();
    void clearResults();

    // Configuration
    void setThreshold(int value);
    void setBlurRadius(int radius);
    void setEdgeThreshold(int threshold);

private:
    bool validateImage(const uint8_t *image, size_t size);
    bool parseJPEGHeader(const uint8_t *data, size_t size, int &width, int &height, int &components);
    bool applyGaussianBlur(uint8_t *image, int width, int height, int radius);
    bool applyEdgeDetection(uint8_t *image, int width, int height, int threshold);
    bool applyHistogramEqualization(uint8_t *image, int width, int height);
    bool calculateImageHash(const uint8_t *image, size_t size, uint64_t &hash);
    bool compareImageHashes(uint64_t hash1, uint64_t hash2, float &similarity);
    void logProcessingError(const char *operation, const char *error);
};

#endif // ENABLE_CAMERA

#endif // IMAGE_PROCESSOR_H
