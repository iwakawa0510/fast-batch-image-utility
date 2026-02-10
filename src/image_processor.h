﻿#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace fbiu {

// ITU-R BT.601 standard luminance coefficients
constexpr float LUMA_COEF_R = 0.299f;
constexpr float LUMA_COEF_G = 0.587f;
constexpr float LUMA_COEF_B = 0.114f;

// Default threshold for luma-to-alpha conversion
constexpr uint8_t DEFAULT_LUMA_THRESHOLD = 200;

// Custom luminance parameters
struct CustomLumaParams {
    float coef_r = LUMA_COEF_R;
    float coef_g = LUMA_COEF_G;
    float coef_b = LUMA_COEF_B;
    uint8_t threshold = DEFAULT_LUMA_THRESHOLD;
};

enum class ImageFormat {
    PNG,
    TIFF,
    TGA,
    JPEG,
    BMP,
    UNKNOWN
};

enum class ProcessFunction {
    LUMA_TO_ALPHA,        // Luminance → Transparency (alpha) mask
    LUMA_TO_ALPHA_CUSTOM, // Custom luminance → Transparency with adjustable parameters
    CONVERT_TO_PNG        // Simple format conversion to PNG
};

struct ImageData {
    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;
    
    bool is_valid() const {
        return width > 0 && height > 0 && channels > 0 && 
               pixels.size() == static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
    }
};

class ImageProcessor {
public:
    ImageProcessor() = default;
    ~ImageProcessor() = default;

    // Load image from file
    static ImageData load_image(const std::string& path);
    
    // Save image as PNG
    static bool save_png(const std::string& path, const ImageData& image);
    
    // Detect image format from file extension
    static ImageFormat detect_format(const std::string& path);
    
    // Process functions
    static ImageData luma_to_alpha(const ImageData& input, uint8_t threshold = DEFAULT_LUMA_THRESHOLD);
    static ImageData luma_to_alpha_custom(const ImageData& input, const CustomLumaParams& params);
    static ImageData convert_to_png(const ImageData& input);
    
    // Apply processing function
    static ImageData process(const ImageData& input, ProcessFunction func);
    
    // Batch processing
    struct BatchOptions {
        std::string input_dir;
        std::string output_dir;
        ProcessFunction function = ProcessFunction::LUMA_TO_ALPHA;
        int num_threads = 0;  // 0 = auto-detect
        uint8_t luma_threshold = DEFAULT_LUMA_THRESHOLD;  // Threshold for standard luma_to_alpha
        CustomLumaParams custom_params;  // Parameters for LUMA_TO_ALPHA_CUSTOM
        std::function<void(int, int, const std::string&)> progress_callback;
    };
    
    static bool batch_process(const BatchOptions& options);
    
private:
    // Luminance calculation: L = 0.299*R + 0.587*G + 0.114*B
    static uint8_t calculate_luminance(uint8_t r, uint8_t g, uint8_t b);
    
    // Custom luminance calculation with custom coefficients
    static uint8_t calculate_luminance_custom(uint8_t r, uint8_t g, uint8_t b, 
                                             float coef_r, float coef_g, float coef_b);
};

} // namespace fbiu
