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
        # Check for Release folder (MSVC default)
        if (BUILD_DIR / "Release" / "fbiu_cli.exe").exists():
            CLI_EXECUTABLE = BUILD_DIR / "Release" / "fbiu_cli.exe"
        else:
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
    
    # Scenarios matching README
    scenarios = [
        {"w": 1000, "h": 1000, "count": 1000},
        {"w": 2000, "h": 2000, "count": 500},
    ]

    for s in scenarios:
        w, h, count = s["w"], s["h"], s["count"]
        input_dir = SCRIPT_DIR / f"test_input_{w}x{h}"
        output_dir = SCRIPT_DIR / f"test_output_{w}x{h}"
        
        print(f"\nTest configuration: {w}x{h}, {count} images")
        
        if not input_dir.exists() or len(list(input_dir.glob("*.png"))) < count:
            generate_test_images(str(input_dir), count, w, h)
        else:
            print(f"Using existing test images in {input_dir}")
        
        # Run benchmark
        elapsed = run_benchmark(
            str(CLI_EXECUTABLE),
            str(input_dir),
            str(output_dir),
            "luma2alpha"
        )
        
        if elapsed is None:
            print("\nBenchmark failed!")
            continue
        
        # Calculate statistics
        throughput = count / elapsed
        time_per_image = elapsed / count * 1000
        
        print("\n" + "-" * 60)
        print(f"Results: {w}x{h}")
        print("-" * 60)
        print(f"Time:       {elapsed:.2f} s")
        print(f"Throughput: {throughput:.2f} imgs/sec")
        print(f"Per image:  {time_per_image:.2f} ms")
        print("-" * 60)


if __name__ == "__main__":
    main()
