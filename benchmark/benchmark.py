#!/usr/bin/env python3
"""
Fast Batch Image Utility - Benchmark Script

This script generates test images and measures processing performance.
"""

import os
import sys
import time
import subprocess
import shutil
from pathlib import Path

try:
    from PIL import Image
    import numpy as np
except ImportError:
    print("Error: This script requires PIL (Pillow) and numpy")
    print("Install with: pip install Pillow numpy")
    sys.exit(1)


def generate_test_images(output_dir, count=1000, width=1000, height=1000):
    """Generate random test images for benchmarking."""
    print(f"Generating {count} test images ({width}x{height})...")
    
    os.makedirs(output_dir, exist_ok=True)
    
    for i in range(count):
        # Generate random RGB image
        data = np.random.randint(0, 256, (height, width, 3), dtype=np.uint8)
        img = Image.fromarray(data, 'RGB')
        
        output_path = os.path.join(output_dir, f"test_{i:04d}.png")
        img.save(output_path)
        
        if (i + 1) % 100 == 0:
            print(f"  Generated {i + 1}/{count} images...")
    
    print(f"Generated {count} test images in {output_dir}")


def run_benchmark(cli_executable, input_dir, output_dir, function="luma2alpha"):
    """Run the CLI tool and measure execution time."""
    print(f"\nRunning benchmark...")
    print(f"  Executable: {cli_executable}")
    print(f"  Input: {input_dir}")
    print(f"  Output: {output_dir}")
    print(f"  Function: {function}")
    
    # Clear output directory
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)
    
    # Build command
    cmd = [
        cli_executable,
        "--input", input_dir,
        "--output", output_dir,
        "--function", function
    ]
    
    # Measure execution time
    start_time = time.time()
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print(f"Error running benchmark: {e}")
        print(f"stderr: {e.stderr}")
        return None
    
    end_time = time.time()
    elapsed = end_time - start_time
    
    return elapsed


def main():
    # Configuration
    SCRIPT_DIR = Path(__file__).parent
    PROJECT_ROOT = SCRIPT_DIR.parent
    BUILD_DIR = PROJECT_ROOT / "build" / "bin"
    
    TEST_INPUT_DIR = SCRIPT_DIR / "test_input"
    TEST_OUTPUT_DIR = SCRIPT_DIR / "test_output"
    
    # Determine CLI executable name
    if sys.platform == "win32":
        CLI_EXECUTABLE = BUILD_DIR / "fbiu_cli.exe"
    else:
        CLI_EXECUTABLE = BUILD_DIR / "fbiu_cli"
    
    # Check if executable exists
    if not CLI_EXECUTABLE.exists():
        print(f"Error: CLI executable not found at {CLI_EXECUTABLE}")
        print("Please build the project first:")
        print("  mkdir build && cd build")
        print("  cmake .. && cmake --build . --config Release")
        sys.exit(1)
    
    print("=" * 60)
    print("Fast Batch Image Utility - Benchmark")
    print("=" * 60)
    
    # Generate test images
    image_count = 1000
    image_size = (1000, 1000)
    
    print(f"\nTest configuration:")
    print(f"  Image count: {image_count}")
    print(f"  Image size: {image_size[0]}x{image_size[1]}")
    
    if not TEST_INPUT_DIR.exists() or len(list(TEST_INPUT_DIR.glob("*.png"))) < image_count:
        generate_test_images(str(TEST_INPUT_DIR), image_count, *image_size)
    else:
        print(f"\nUsing existing test images in {TEST_INPUT_DIR}")
    
    # Run benchmark
    elapsed = run_benchmark(
        str(CLI_EXECUTABLE),
        str(TEST_INPUT_DIR),
        str(TEST_OUTPUT_DIR),
        "luma2alpha"
    )
    
    if elapsed is None:
        print("\nBenchmark failed!")
        sys.exit(1)
    
    # Calculate statistics
    throughput = image_count / elapsed
    time_per_image = elapsed / image_count * 1000  # milliseconds
    
    print("\n" + "=" * 60)
    print("Benchmark Results")
    print("=" * 60)
    print(f"Total images processed: {image_count}")
    print(f"Total time: {elapsed:.2f} seconds")
    print(f"Throughput: {throughput:.2f} images/second")
    print(f"Time per image: {time_per_image:.2f} ms")
    print("=" * 60)
    
    # Verify output
    output_files = list(TEST_OUTPUT_DIR.glob("*.png"))
    print(f"\nOutput verification: {len(output_files)}/{image_count} files generated")
    
    if len(output_files) == image_count:
        print("✓ All images processed successfully")
    else:
        print("✗ Some images failed to process")


if __name__ == "__main__":
    main()
