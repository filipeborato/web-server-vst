#include <iostream>
#include <fstream>
#include <vector>
#include <sndfile.h>
#include <cstring>

class AudioFileReader {
public:
    AudioFileReader(const std::string& filePath) : filePath(filePath), sampleRate(44100), bitDepth(16), numChannels(1) {
        // Read the audio file metadata and initialize class members
        readAudioMetadata();
    }

    void readSamples(float* buffer, int numSamples, int offset);
    int getTotalSamples() const;
    void saveAudioToFile(const std::string& filePath, const float* audioBuffer, int bufferSize);
    void saveAudioToSNDFile(const std::string& filePath, const float* audioBuffer, int bufferSize);
    float* makeAudio(float* audio, const float* audioBuffer, int samples, int offset);
    float* cpyTotalAudio(float* audio, float* buffer, int samples, int offset);

private:
    void readAudioMetadata();

    std::string filePath;
    int sampleRate;
    int bitDepth;
    int numChannels;
    int totalSamples;
    int dataChunkOffset;
};