cmake_minimum_required(VERSION 3.20)
project(FastBatchImageUtility VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_GUI "Build GUI application" ON)
option(BUILD_CLI "Build CLI application" ON)
option(ENABLE_SIMD "Enable SIMD optimizations (SSE2/AVX2)" OFF)

# Find Qt6 for GUI
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# Third-party: stb_image (header-only)
include_directories(${CMAKE_SOURCE_DIR}/third_party/stb)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /O2)
    # Force C++20 standard for MSVC to ensure char8_t is available
    add_compile_options(/W4 /O2 /std:c++20)
    if(ENABLE_SIMD)
        add_compile_options(/arch:AVX2)
    endif()
else()
    add_compile_options(-Wall -Wextra -O3)
    if(ENABLE_SIMD)
        add_compile_options(-mavx2 -mfma)
    endif()
endif()

# Common library: image processing core
add_library(image_core STATIC
    src/image_processor.cpp
    src/thread_pool.cpp
)
target_include_directories(image_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

if(ENABLE_SIMD)
    target_compile_definitions(image_core PRIVATE ENABLE_SIMD)
endif()

# GUI Application
if(BUILD_GUI)
    set(GUI_SOURCES
        src/gui_main.cpp
        src/main_window.cpp
        include/main_window.h
        resources/app.qrc
    )
    
    # Translation files configuration
    set(TRANS_DIR "${CMAKE_SOURCE_DIR}/resources/translations")
    set(TS_FILES
        "${TRANS_DIR}/app_ja.ts"
        "${TRANS_DIR}/app_en.ts"
        "${TRANS_DIR}/app_fr.ts"
    )
    
    # Automatically generate .qm files from .ts files
    set(QM_FILES)
    foreach(TS_FILE IN LISTS TS_FILES)
        get_filename_component(FILENAME ${TS_FILE} NAME_WE)
        set(QM_FILE "${TRANS_DIR}/${FILENAME}.qm")
        list(APPEND QM_FILES ${QM_FILE})
        
        add_custom_command(
            OUTPUT ${QM_FILE}
            COMMAND Qt6::lrelease ${TS_FILE} -qm ${QM_FILE}
            DEPENDS ${TS_FILE}
            COMMENT "Generating ${QM_FILE}"
        )
    endforeach()
    
    add_executable(fbiu_gui ${GUI_SOURCES} ${QM_FILES})
    target_link_libraries(fbiu_gui PRIVATE 
        image_core
        Qt6::Core
        Qt6::Widgets
    )
    target_include_directories(fbiu_gui PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
    )
    
    # Copy translation files to output directory
    add_custom_command(TARGET fbiu_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory 
            $<TARGET_FILE_DIR:fbiu_gui>/translations
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${QM_FILES}
            $<TARGET_FILE_DIR:fbiu_gui>/translations/
    )
endif()

# CLI Application
if(BUILD_CLI)
    add_executable(fbiu_cli
        src/cli_main.cpp
    )
    target_link_libraries(fbiu_cli PRIVATE image_core)
    target_include_directories(fbiu_cli PRIVATE ${CMAKE_SOURCE_DIR}/include)
endif()

# Install rules
install(TARGETS image_core ARCHIVE DESTINATION lib)
if(BUILD_GUI)
    install(TARGETS fbiu_gui RUNTIME DESTINATION bin)
    install(FILES ${QM_FILES} DESTINATION bin/translations)
endif()
if(BUILD_CLI)
    install(TARGETS fbiu_cli RUNTIME DESTINATION bin)
endif()
