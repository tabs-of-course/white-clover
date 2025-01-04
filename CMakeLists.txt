# Minimum CMake version and project definition
cmake_minimum_required(VERSION 3.15)
project(white-clover VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Collect source files
set(SOURCES
    src/main.cpp
    src/thread_manager.cpp
    src/sender.cpp
    src/receiver.cpp
    src/thread_context.cpp
    src/key_monitor_context.cpp
)

# Collect header files
set(HEADERS
    include/message_types.h
    include/message_channel.h
    include/i_sender.h
    include/i_receiver.h
    include/i_thread_manager.h
    include/i_thread_context.h
    include/thread_manager.h
    include/sender.h
    include/receiver.h
    include/thread_context.h
    include/key_monitor_context.h
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Windows-specific settings
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE 
        NOMINMAX  # Prevents Windows.h from defining min/max macros
        WIN32_LEAN_AND_MEAN  # Reduces Windows.h overhead
    )
    
    # Link Windows libraries
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Threads::Threads
            user32  # For keyboard input functions
            gdi32   # Required by some Windows API functions
    )
else()
    # Non-Windows systems just need pthread
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Threads::Threads
    )
endif()

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Enable testing
enable_testing()

# Output directories
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Print configuration summary
message(STATUS "")
message(STATUS "Project configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
if(WIN32)
    message(STATUS "  Windows libraries: user32, gdi32")
endif()
message(STATUS "")