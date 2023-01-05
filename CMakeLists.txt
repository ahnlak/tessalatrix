# CMake file for Tesselatrix

# Declare a required version; 3.16 is what's default on Ubuntu 20.04
cmake_minimum_required(VERSION 3.16)

# Enable CMP0048, to allow for version to be set
cmake_policy(SET CMP0048 NEW)

# Declare the project details
project(tesselatrix 
        VERSION 0.1.1
        DESCRIPTION "A cross platform implementation of Tetris"
        HOMEPAGE_URL "https://github.com/ahnlak/tessalatrix"
        LANGUAGES C)

# Build the config header
configure_file(config.h.in config.h)

# Add in the source code
add_subdirectory(src)
