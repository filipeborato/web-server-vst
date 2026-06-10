#include <iostream>
#include <fstream>
#include <vector>
#include <sndfile.h>
#include <cstring>

class AudioFileReader {
public:
    AudioFileReader(const std::string& filePath) : filePath(filePath), sampleRate(44100), bitDepth(16), numChannels(1) {
        // Read the audio file metadata and initialize class members.
        // Mantém o arquivo aberto (sndFile) para leitura sequencial.
        readAudioMetadata();
    }

    ~AudioFileReader();

    void readSamples(float* buffer, int numFrames, int frameOffset, int channel);
    int getTotalSamples() const;
    int getNumChannels() const;
    int getSampleRate() const;
    int getFormat() const;
    bool verifyAudioType(const std::string& filePath );
    void saveAudioToFile(const std::string& filePath, const float* audioBuffer, int bufferSize);
    bool saveAudioToSNDFile(const std::string& filePath, const float* audioBuffer, int bufferSize);
    float* makeAudio(float* audio, const float* audioBuffer, int samples, int offset);
    float* cpyTotalAudio(float* audio, float* buffer, int samples, int offset);

private:
    void readAudioMetadata();

    std::string filePath;
    int format;
    int sampleRate;
    int bitDepth;
    int numChannels;
    int totalSamples;
    int dataChunkOffset;

    // Arquivo mantido aberto durante todo o ciclo de vida do reader.
    SNDFILE* sndFile = nullptr;

    // Estado de leitura sequencial + cache do último bloco intercalado lido.
    // Evita reabrir/seek o arquivo a cada bloco (causa do pico de CPU em arquivos longos, ex.: MP3).
    sf_count_t currentFrame = 0;        // posição atual do ponteiro do arquivo (em frames)
    std::vector<float> blockBuffer;     // último bloco lido, intercalado
    sf_count_t blockStartFrame = -1;    // frame inicial do bloco em cache
    int blockFrames = 0;                // nº de frames válidos no bloco em cache
};