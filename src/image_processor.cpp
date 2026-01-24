#include "image_processor.h"
#include "thread_pool.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <filesystem>
#include <algorithm>
#include <iostream>
#include <atomic>
#include <thread>

namespace fs = std::filesystem;

namespace fbiu {

ImageData ImageProcessor::load_image(const std::string& path) {
    ImageData result;
    
    int w, h, c;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 0);
    
    if (!data) {
        std::cerr << "Failed to load image: " << path << std::endl;
        return result;
    }
    
    result.width = w;
    result.height = h;
    result.channels = c;
    result.pixels.assign(data, data + (w * h * c));
    
    stbi_image_free(data);
    return result;
}

bool ImageProcessor::save_png(const std::string& path, const ImageData& image) {
    if (!image.is_valid()) {
        std::cerr << "Invalid image data" << std::endl;
        return false;
    }
    
    int result = stbi_write_png(
        path.c_str(),
        image.width,
        image.height,
        image.channels,
        image.pixels.data(),
        image.width * image.channels
    );
    
    return result != 0;
}

ImageFormat ImageProcessor::detect_format(const std::string& path) {
    fs::path p(path);
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".png") return ImageFormat::PNG;
    if (ext == ".tif" || ext == ".tiff") return ImageFormat::TIFF;
    if (ext == ".tga") return ImageFormat::TGA;
    if (ext == ".jpg" || ext == ".jpeg") return ImageFormat::JPEG;
    if (ext == ".bmp") return ImageFormat::BMP;
    
    return ImageFormat::UNKNOWN;
}

uint8_t ImageProcessor::calculate_luminance(uint8_t r, uint8_t g, uint8_t b) {
    // L = 0.299*R + 0.587*G + 0.114*B
    float luma = 0.299f * r + 0.587f * g + 0.114f * b;
    return static_cast<uint8_t>(std::clamp(luma, 0.0f, 255.0f));
}

ImageData ImageProcessor::luma_to_alpha(const ImageData& input, uint8_t threshold) {
    if (!input.is_valid()) {
        return ImageData{};
    }
    
    // First, convert input to RGBA format (normalize to have alpha channel)
    ImageData rgba_input;
    rgba_input.width = input.width;
    rgba_input.height = input.height;
    rgba_input.channels = 4; // RGBA
    rgba_input.pixels.resize(input.width * input.height * 4);
    
    const int pixel_count = input.width * input.height;
    
    // Convert input to RGBA first
    for (int i = 0; i < pixel_count; ++i) {
        uint8_t r, g, b, a = 255; // Default alpha to opaque
        
        if (input.channels == 1) {
            // Grayscale input -> RGB
            r = g = b = input.pixels[i];
        } else if (input.channels == 2) {
            // Grayscale + Alpha input
            r = g = b = input.pixels[i * 2 + 0];
            a = input.pixels[i * 2 + 1];
        } else if (input.channels == 3) {
            // RGB input
            r = input.pixels[i * 3 + 0];
            g = input.pixels[i * 3 + 1];
            b = input.pixels[i * 3 + 2];
        } else if (input.channels == 4) {
            // RGBA input
            r = input.pixels[i * 4 + 0];
            g = input.pixels[i * 4 + 1];
            b = input.pixels[i * 4 + 2];
            a = input.pixels[i * 4 + 3];
        } else {
            // Invalid channel count, skip
            continue;
        }
        
        // Store as RGBA
        rgba_input.pixels[i * 4 + 0] = r;
        rgba_input.pixels[i * 4 + 1] = g;
        rgba_input.pixels[i * 4 + 2] = b;
        rgba_input.pixels[i * 4 + 3] = a;
    }
    
    // Now convert luminance to alpha channel
    ImageData output;
    output.width = rgba_input.width;
    output.height = rgba_input.height;
    output.channels = 4; // RGBA
    output.pixels.resize(rgba_input.width * rgba_input.height * 4);
    
    for (int i = 0; i < pixel_count; ++i) {
        uint8_t r = rgba_input.pixels[i * 4 + 0];
        uint8_t g = rgba_input.pixels[i * 4 + 1];
        uint8_t b = rgba_input.pixels[i * 4 + 2];
        
        // Calculate luminance from RGB.
        // User intent: "luminance -> transparency" (brighter = more transparent),
        // so alpha should be inverted: alpha = 255 - luminance.
        uint8_t luma = calculate_luminance(r, g, b);
        
        // Threshold logic: if luminance is below threshold, preserve original color fully (alpha = 255)
        // Otherwise, apply transparency based on luminance
        uint8_t alpha;
        if (luma < threshold) {
            // Below threshold: keep original color (fully opaque)
            alpha = 255;
        } else {
            // Above threshold: apply transparency (brighter = more transparent)
            // Map threshold..255 to 255..0
            uint8_t mapped_luma = static_cast<uint8_t>(
                255 - ((static_cast<uint16_t>(luma - threshold) * 255) / (255 - threshold))
            );
            alpha = mapped_luma;
        }
        
        // If the source already had alpha, preserve it by multiplying:
        // final_alpha = alpha * src_alpha / 255
        uint8_t src_a = rgba_input.pixels[i * 4 + 3];
        uint8_t final_alpha = static_cast<uint8_t>(
            (static_cast<uint16_t>(alpha) * static_cast<uint16_t>(src_a)) / 255
        );

        output.pixels[i * 4 + 0] = r;
        output.pixels[i * 4 + 1] = g;
        output.pixels[i * 4 + 2] = b;
        output.pixels[i * 4 + 3] = final_alpha;
    }
    
    return output;
}

ImageData ImageProcessor::convert_to_png(const ImageData& input) {
    // Simply return a copy - conversion happens during save
    return input;
}

ImageData ImageProcessor::process(const ImageData& input, ProcessFunction func) {
    switch (func) {
        case ProcessFunction::LUMA_TO_ALPHA:
            return luma_to_alpha(input, 200); // Default threshold
        case ProcessFunction::CONVERT_TO_PNG:
            return convert_to_png(input);
        default:
            return ImageData{};
    }
}

bool ImageProcessor::batch_process(const BatchOptions& options) {
    if (!fs::exists(options.input_dir) || !fs::is_directory(options.input_dir)) {
        std::cerr << "Input directory does not exist: " << options.input_dir << std::endl;
        return false;
    }
    
    // Create output directory if it doesn't exist
    if (!fs::exists(options.output_dir)) {
        fs::create_directories(options.output_dir);
    }
    
    // Collect all valid image files
    std::vector<fs::path> image_files;
    for (const auto& entry : fs::directory_iterator(options.input_dir)) {
        if (!entry.is_regular_file()) continue;
        
        ImageFormat fmt = detect_format(entry.path().string());
        if (fmt != ImageFormat::UNKNOWN) {
            image_files.push_back(entry.path());
        }
    }
    
    if (image_files.empty()) {
        std::cerr << "No valid image files found in input directory" << std::endl;
        return false;
    }
    
    // Determine thread count
    int num_threads = options.num_threads;
    if (num_threads <= 0) {
        num_threads = static_cast<int>(std::thread::hardware_concurrency());
        if (num_threads <= 0) num_threads = 4;
    }
    
    // Create thread pool
    ThreadPool pool(num_threads);
    
    std::atomic<int> completed{0};
    const int total = static_cast<int>(image_files.size());
    
    // Process each file
    for (const auto& input_path : image_files) {
        pool.enqueue([&, input_path]() {
            // Load image
            ImageData input_image = load_image(input_path.string());
            if (!input_image.is_valid()) {
                completed++;
                if (options.progress_callback) {
                    options.progress_callback(completed, total, input_path.filename().string());
                }
                return;
            }
            
            // Process image
            ImageData output_image;
            if (options.function == ProcessFunction::LUMA_TO_ALPHA) {
                output_image = luma_to_alpha(input_image, options.luma_threshold);
            } else {
                output_image = process(input_image, options.function);
            }
            if (!output_image.is_valid()) {
                completed++;
                if (options.progress_callback) {
                    options.progress_callback(completed, total, input_path.filename().string());
                }
                return;
            }
            
            // Save as PNG
            fs::path output_path = fs::path(options.output_dir) / input_path.filename();
            output_path.replace_extension(".png");
            
            save_png(output_path.string(), output_image);
            
            completed++;
            if (options.progress_callback) {
                options.progress_callback(completed, total, input_path.filename().string());
            }
        });
    }
    
    // Wait for all tasks to complete
    pool.wait();
    
    return true;
}

} // namespace fbiu
