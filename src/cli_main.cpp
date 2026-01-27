#include "image_processor.h"
#include <iostream>
#include <string>
#include <map>

void print_usage() {
    std::cout << "Fast Batch Image Utility - CLI Mode\n";
    std::cout << "Usage: fbiu_cli --input <dir> --output <dir> --function <func> [--threads <n>]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --input <dir>      Input directory containing images\n";
    std::cout << "  --output <dir>     Output directory for processed images\n";
    std::cout << "  --function <func>  Processing function:\n";
    std::cout << "                     luma2alpha  - Convert luminance to transparency (alpha)\n";
    std::cout << "                     png         - Convert to PNG format\n";
    std::cout << "  --threads <n>      Number of threads (default: auto)\n";
    std::cout << "  --help             Show this help message\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::map<std::string, std::string> args;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        
        if (arg.substr(0, 2) == "--" && i + 1 < argc) {
            std::string key = arg.substr(2);
            std::string value = argv[i + 1];
            args[key] = value;
            ++i;
        }
    }
    
    // Validate required arguments
    if (args.find("input") == args.end() || 
        args.find("output") == args.end() || 
        args.find("function") == args.end()) {
        std::cerr << "Error: Missing required arguments\n\n";
        print_usage();
        return 1;
    }
    
    // Parse function
    fbiu::ProcessFunction func;
    std::string func_str = args["function"];
    
    if (func_str == "luma2alpha") {
        func = fbiu::ProcessFunction::LUMA_TO_ALPHA;
    } else if (func_str == "png") {
        func = fbiu::ProcessFunction::CONVERT_TO_PNG;
    } else {
        std::cerr << "Error: Unknown function '" << func_str << "'\n";
        return 1;
    }
    
    // Parse threads
    int threads = 0;
    if (args.find("threads") != args.end()) {
        try {
            threads = std::stoi(args["threads"]);
        } catch (...) {
            std::cerr << "Warning: Invalid thread count, using auto\n";
            threads = 0;
        }
    }
    
    // Setup batch options
    fbiu::ImageProcessor::BatchOptions options;
    options.input_dir = args["input"];
    options.output_dir = args["output"];
    options.function = func;
    options.num_threads = threads;
    
    options.progress_callback = [](int completed, int total, const std::string& filename) {
        std::cout << "[" << completed << "/" << total << "] Processing: " 
                  << filename << std::endl;
    };
    
    // Execute
    std::cout << "Starting batch processing...\n";
    std::cout << "Input:  " << options.input_dir << "\n";
    std::cout << "Output: " << options.output_dir << "\n";
    std::cout << "Function: " << func_str << "\n";
    std::cout << "Threads: " << (threads > 0 ? std::to_string(threads) : "auto") << "\n\n";
    
    bool success = fbiu::ImageProcessor::batch_process(options);
    
    if (success) {
        std::cout << "\nBatch processing completed successfully\n";
        return 0;
    } else {
        std::cerr << "\nBatch processing failed\n";
        return 1;
    }
}
