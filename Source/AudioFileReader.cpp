#include "AudioFileReader.h"

void AudioFileReader::readAudioMetadata() {
    std::ifstream inputFile(filePath, std::ios::binary);

    if (inputFile.is_open()) {
        // Assuming WAV format for simplicity
        inputFile.seekg(40); // Skip header to reach Subchunk2Size field
        int subchunk2Size;
        inputFile.read(reinterpret_cast<char*>(&subchunk2Size), sizeof(subchunk2Size));

        totalSamples = subchunk2Size / (bitDepth / 8) / numChannels;
        inputFile.close();
    }
    else {
        std::cerr << "Failed to open audio file for reading metadata." << std::endl;
    }
}


void AudioFileReader::readSamples(float* buffer, int numSamples) {
    // Here you would implement the code to read audio samples from the file
    // and convert them to float format
    // This is just a placeholder implementation
    // You might need to handle endianness, different bit depths, etc.
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfInfo);

    if (!file) {
        std::cerr << "Error opening the input file: " << filePath << std::endl;
        return;
    }

    sf_readf_float(file, buffer, numSamples);

    sf_close(file);
}

int AudioFileReader::getTotalSamples() const {
    return totalSamples;
}
