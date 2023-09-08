#include <iostream>
#include <fstream>
#include <vector>
#include <sndfile.h>

class AudioFileReader {
public:
    AudioFileReader(const std::string& filePath) : filePath(filePath), sampleRate(22050), bitDepth(16), numChannels(1) {
        // Read the audio file metadata and initialize class members
        readAudioMetadata();
    }

    void readSamples(float* buffer, int numSamples);
    int getTotalSamples() const;
    void saveAudioToFile(const std::string& filePath, const float* audioBuffer, int bufferSize);
    float* createAudio(const float* buffer, const int samples);

private:
    void readAudioMetadata();

    std::string filePath;
    int sampleRate;
    int bitDepth;
    int numChannels;
    int totalSamples;
    int dataChunkOffset;
};