#include "AudioFileReader.h"

void AudioFileReader::readAudioMetadata() {
    std::ifstream inputFile(filePath, std::ios::binary);

    if (inputFile.is_open()) {
        // Assuming WAV format for simplicity
        inputFile.seekg(40); // Skip header to reach Subchunk2Size field
        int subchunk2Size;
        inputFile.read(reinterpret_cast<char*>(&subchunk2Size), sizeof(subchunk2Size));

        totalSamples = subchunk2Size / (bitDepth / 8) / numChannels;
        // Read and analyze WAV header to find data chunk offset
        inputFile.seekg(0, std::ios::end);
        int fileSize = static_cast<int>(inputFile.tellg());

        inputFile.seekg(0); // Move back to the beginning
        char header[44];
        inputFile.read(header, sizeof(header));

        if (fileSize >= 44 && strncmp(header, "RIFF", 4) == 0 && strncmp(header + 8, "WAVE", 4) == 0) {
            // Find "data" subchunk
            for (int offset = 12; offset < 44; offset += 4) {
                if (strncmp(header + offset, "data", 4) == 0) {
                    dataChunkOffset = offset + 8; // Offset of data chunk
                    break;
                }
            }

            inputFile.close();
        }
        else {
            std::cerr << "Failed to open audio file for reading metadata." << std::endl;
        }
    }
}


    void AudioFileReader::readSamples(float* buffer, int numSamples){
    std::ifstream inputFile(filePath, std::ios::binary);

    if (inputFile.is_open()) {
        // Assuming 16-bit PCM audio for simplicity
        inputFile.seekg(dataChunkOffset); // Seek to the beginning of audio data

        // Read audio samples and convert to float (-1.0 to 1.0 range)
        std::vector<short> sampleBuffer(numSamples * numChannels);
        inputFile.read(reinterpret_cast<char*>(sampleBuffer.data()), sizeof(short) * numSamples * numChannels);

        for (int i = 0; i < numSamples * numChannels; ++i) {
            buffer[i] = static_cast<float>(sampleBuffer[i]) / 32768.0f; // Convert to float range
        }

        inputFile.close();
    }
    else {
        std::cerr << "Failed to open audio file for reading samples." << std::endl;
    }
}

int AudioFileReader::getTotalSamples() const {
    return totalSamples;
}

void AudioFileReader::saveAudioToFile(const std::string& filePath, const float* buffer, int numSamples) {
    std::ofstream outputFile(filePath, std::ios::binary);

    if (outputFile.is_open()) {
        // Convert float audio samples to 16-bit PCM format
        std::vector<short> pcmSamples(numSamples * numChannels);
        for (int i = 0; i < numSamples * numChannels; ++i) {
            pcmSamples[i] = static_cast<short>(buffer[i] * 32767.0f); // Convert to short range
        }

        // Write the PCM samples to the output file
        outputFile.write(reinterpret_cast<const char*>(pcmSamples.data()), sizeof(short) * numSamples * numChannels);

        outputFile.close();
        std::cout << "Saved audio to " << filePath << std::endl;
    }
    else {
        std::cerr << "Failed to open output file." << std::endl;
    }
}

float* AudioFileReader::createAudio(float* audio, const float* buffer, int samples, int offset){
    int j = 0;
    for (int i = offset; i < (offset + samples); i++) {
        audio[i] = buffer[j];
        j++;
    }
    return audio;
}