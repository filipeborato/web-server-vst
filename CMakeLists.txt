# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(AudioProcessingProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the output binary directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the executable target (main.cpp is in the Source directory)
#add_executable(${PROJECT_NAME} Source/main.cpp)

add_executable(${PROJECT_NAME} Source/main.cpp Source/Host.cpp Source/PluginHost.cpp Source/AudioFileReader.cpp Source/utils.cpp)

# Include directories for external libraries and VST SDK
include_directories(${CMAKE_SOURCE_DIR}/external/vstsdk2.4)

add_subdirectory(external/Crow)

# Link libsndfile from the external directory
find_library(SNDFILE_LIBRARY sndfile PATHS ${LIBSNDFILE_DIR}/lib REQUIRED)

# Vincular as bibliotecas necessárias
target_link_libraries(AudioProcessingProject PRIVATE 
    ${SNDFILE_LIBRARY} 
    dl 
    pthread 
    uuid 
    Crow::Crow
)

