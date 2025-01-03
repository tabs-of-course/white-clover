# Default build directory
BUILD_DIR := build

# Default build type (Debug/Release)
BUILD_TYPE ?= Debug

# Number of parallel jobs for building
JOBS ?= 8

# Ensure using bash shell
SHELL := /bin/bash

.PHONY: all build clean configure

# Default target
all: build

# Configure the project using CMake
configure:
	@echo "Configuring CMake..."
	@mkdir -p $(BUILD_DIR)
	@cmake -B $(BUILD_DIR) -S . -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

# Build the project
build: configure
	@echo "Building..."
	@cmake --build $(BUILD_DIR) --config $(BUILD_TYPE) -j $(JOBS)
	@echo "Build complete!"

# Clean build directory
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

# Print help information
help:
	@echo "Available commands:"
	@echo "  make          : Configure and build the project (default)"
	@echo "  make build    : Configure and build the project"
	@echo "  make clean    : Remove all build artifacts"
	@echo "  make configure: Only run CMake configuration"
	@echo ""
	@echo "Options:"
	@echo "  BUILD_TYPE=Debug|Release (default: Debug)"
	@echo "  JOBS=N                   (default: 8)"