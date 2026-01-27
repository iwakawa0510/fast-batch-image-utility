#include "image_processor.h"
#include "thread_pool.h"

// Suppress MSVC warnings
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <filesystem>
#include <algorithm>
#include <iostream>
#include <atomic>
#include <thread>
#include <fstream>
#include <vector>

#ifdef ENABLE_SIMD
#include <immintrin.h>
#endif

namespace fs = std::filesystem;

namespace fbiu {

ImageData ImageProcessor::load_image(const std::string& path) {
    ImageData result;
    
    // Use char8_t for C++20 compliant UTF-8 path handling
    fs::path file_path(reinterpret_cast<const char8_t*>(path.c_str()));
    
    // Open file using standard stream
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return result;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(static_cast<size_t>(size));
    if (!file.read(buffer.data(), size)) return result;

    int w, h, c;
    unsigned char* data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(buffer.data()), static_cast<int>(size), &w, &h, &c, 0);
    
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

// stb_image_write 用のコールバック関数
static void write_to_stream(void* context, void* data, int size) {
    std::ofstream* ofs = static_cast<std::ofstream*>(context);
    ofs->write(static_cast<const char*>(data), size);
}

bool ImageProcessor::save_png(const std::string& path, const ImageData& image) {
    if (!image.is_valid()) {
        std::cerr << "Invalid image data" << std::endl;
        return false;
    }
    
    fs::path file_path(reinterpret_cast<const char8_t*>(path.c_str()));
    std::ofstream file(file_path, std::ios::binary);
    if (!file) return false;

    int result = stbi_write_png_to_func(
        write_to_stream,
        &file,
        image.width,
        image.height,
        image.channels,
        image.pixels.data(),
        image.width * image.channels
    );
    
    return result != 0;
}

ImageFormat ImageProcessor::detect_format(const std::string& path) {
    fs::path p(reinterpret_cast<const char8_t*>(path.c_str()));
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
    
    int scalar_start_index = 0;

#ifdef ENABLE_SIMD
    // 将来的にここにSIMD実装を追加し、処理済みピクセル数を scalar_start_index に設定します
    // scalar_start_index = (pixel_count / 8) * 8;
    // ... SIMD loop ...
#endif

    for (int i = scalar_start_index; i < pixel_count; ++i) {
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
            int range = 255 - threshold;
            if (range == 0) range = 1; // Avoid division by zero
            uint8_t mapped_luma = static_cast<uint8_t>(
                255 - ((static_cast<uint16_t>(luma - threshold) * 255) / range)
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
    fs::path input_dir(reinterpret_cast<const char8_t*>(options.input_dir.c_str()));
    fs::path output_dir(reinterpret_cast<const char8_t*>(options.output_dir.c_str()));

    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cerr << "Input directory does not exist: " << options.input_dir << std::endl;
        return false;
    }
    
    // Create output directory if it doesn't exist
    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir);
    }
    
    // Collect all valid image files
    std::vector<fs::path> image_files;
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (!entry.is_regular_file()) continue;
        
        // fs::path -> u8string -> std::string (UTF-8)
        std::u8string u8path = entry.path().u8string();
        std::string path_str(reinterpret_cast<const char*>(u8path.c_str()));

        ImageFormat fmt = detect_format(path_str);
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
            std::u8string u8_input_path = input_path.u8string();
            std::string input_path_str(reinterpret_cast<const char*>(u8_input_path.c_str()));
            ImageData input_image = load_image(input_path_str);
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
            fs::path output_path = output_dir / input_path.filename();
            output_path.replace_extension(".png");
            
            std::u8string u8_output_path = output_path.u8string();
            std::string output_path_str(reinterpret_cast<const char*>(u8_output_path.c_str()));
            save_png(output_path_str, output_image);
            
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
