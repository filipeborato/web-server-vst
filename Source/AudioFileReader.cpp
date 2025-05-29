#include "AudioFileReader.h"
#include <sndfile.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>

// Lê os metadados do arquivo e inicializa as variáveis da classe
void AudioFileReader::readAudioMetadata() {
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo)); // Inicializa a estrutura

    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open audio file: " << sf_strerror(file) << std::endl;
        std::cerr << "File path: " << filePath << std::endl;
        exit(1);
    }

    // Exibir informações para debug
    std::cout << "Audio file opened successfully!" << std::endl;
    std::cout << "Sample Rate: " << sfinfo.samplerate << std::endl;
    std::cout << "Channels: " << sfinfo.channels << std::endl;
    std::cout << "Frames: " << sfinfo.frames << std::endl;
    std::cout << "Format: " << sfinfo.format << std::endl;

    // Atribuir os valores às variáveis da classe
    sampleRate = sfinfo.samplerate;
    numChannels = sfinfo.channels;
    totalSamples = sfinfo.frames;  // totalSamples representa o número de frames
    format = sfinfo.format;

    sf_close(file);
}

// Nova função readSamples que recebe também o número do canal desejado.
// Ela lê os frames intercalados, deintercala e extrai somente os dados do canal indicado.
void AudioFileReader::readSamples(float* buffer, int numFrames, int frameOffset, int channel) {
    SF_INFO sfinfo;
    std::memset(&sfinfo, 0, sizeof(sfinfo));

    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open audio file: " << sf_strerror(file) << std::endl;
        return;
    }

    // Verifica se o canal solicitado é válido
    if (channel < 0 || channel >= sfinfo.channels) {
        std::cerr << "Invalid channel requested." << std::endl;
        sf_close(file);
        return;
    }

    // Verifica se o frameOffset está dentro do intervalo válido
    if (frameOffset < 0 || frameOffset >= sfinfo.frames) {
        std::cerr << "Invalid frame offset." << std::endl;
        sf_close(file);
        return;
    }

    // Ajusta numFrames se a leitura extrapolar o final do arquivo
    if (frameOffset + numFrames > sfinfo.frames) {
        numFrames = sfinfo.frames - frameOffset;
        if (numFrames <= 0) {
            std::cerr << "Frame offset is beyond the end of the audio file." << std::endl;
            sf_close(file);
            return;
        }
    }

    // Posiciona o ponteiro no frame desejado
    if (sf_seek(file, frameOffset, SEEK_SET) < 0) {
        std::cerr << "Failed to seek in audio file: " << sf_strerror(file) << std::endl;
        sf_close(file);
        return;
    }

    // Aloca um buffer temporário para armazenar os frames intercalados
    float* tempBuffer = new float[numFrames * sfinfo.channels];
    sf_count_t readFrames = sf_readf_float(file, tempBuffer, numFrames);
    if (readFrames != numFrames) {
        std::cerr << "Warning: Number of frames read (" << readFrames 
                  << ") is less than expected (" << numFrames << ")." << std::endl;
    }
    
    // Deintercala: para cada frame, extrai o sample do canal desejado
    for (int i = 0; i < numFrames; i++) {
        buffer[i] = tempBuffer[i * sfinfo.channels + channel];
    }

    delete[] tempBuffer;
    sf_close(file);
}

// As demais funções permanecem inalteradas

bool AudioFileReader::verifyAudioType(const std::string& filePath) {
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        return false; // Arquivo inválido
    }
    sf_close(file);
    return true;
}

int AudioFileReader::getTotalSamples() const {
    return totalSamples;
}

int AudioFileReader::getNumChannels() const {
    return numChannels;
}

int AudioFileReader::getSampleRate() const {
    return sampleRate;
}

void AudioFileReader::saveAudioToFile(const std::string& filePath, const float* buffer, int numSamples) {
    std::ofstream outputFile(filePath, std::ios::binary);

    if (outputFile.is_open()) {
        // Converte os samples float para 16-bit PCM
        std::vector<short> pcmSamples(numSamples * numChannels);
        for (int i = 0; i < numSamples * numChannels; ++i) {
            pcmSamples[i] = static_cast<short>(buffer[i] * 32767.0f);
        }

        // Escreve os samples no arquivo
        outputFile.write(reinterpret_cast<const char*>(pcmSamples.data()), sizeof(short) * numSamples * numChannels);
        outputFile.close();
        std::cout << "Saved audio to " << filePath << std::endl;
    }
    else {
        std::cerr << "Failed to open output file." << std::endl;
    }
}

float* AudioFileReader::makeAudio(float* audio, const float* audioBuffer, int samples, int offset) {
    int j = 0;
    for (int i = offset; i < (offset + samples); i++) {
        audio[i] = audioBuffer[j];
        j++;
    }
    return audio;
}

float* AudioFileReader::cpyTotalAudio(float* audio, float* buffer, int samples, int offset) {
    if (audio && buffer && offset >= 0 && offset + samples <= totalSamples) {
        memcpy(audio + offset, buffer, samples * sizeof(float));
    }
    return audio;
}

bool AudioFileReader::saveAudioToSNDFile(const std::string& filePath, const float* audioBuffer, int bufferSize) {
    if (audioBuffer == nullptr) {
        std::cerr << "Audio buffer is null. Cannot write audio to file." << std::endl;
        return false;
    }

    if (bufferSize <= 0) {
        std::cerr << "Buffer size is invalid: " << bufferSize << std::endl;
        return false;
    }

    // Determina a extensão com base no formato de áudio
    std::string extension;
    int mainFormat = format & SF_FORMAT_TYPEMASK;

    switch (mainFormat) {
        case SF_FORMAT_WAV: 
            extension = ".wav"; 
            break;
        case SF_FORMAT_AIFF: 
            extension = ".aiff"; 
            break;
        case SF_FORMAT_FLAC: 
            extension = ".flac"; 
            break;
        case SF_FORMAT_OGG: 
            extension = ".OGG"; 
            break;
        default:
            std::cerr << "Unsupported format: " << mainFormat << ". Cannot determine file extension." << std::endl;
            return false;
    }

    std::string fullFilePath = filePath + extension;

    SF_INFO sfinfo = {0};
    sfinfo.samplerate = sampleRate;
    sfinfo.channels = numChannels;
    sfinfo.format = format;

    SNDFILE* file = sf_open(fullFilePath.c_str(), SFM_WRITE, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open output file: " << sf_strerror(file) << std::endl;
        return false;
    }

    if (bufferSize % numChannels != 0) {
        std::cerr << "Buffer size is not aligned with the number of channels." << std::endl;
        sf_close(file);
        return false;
    }

    sf_count_t framesToWrite = bufferSize / numChannels;
    sf_count_t framesWritten = sf_writef_float(file, audioBuffer, framesToWrite);

    if (framesWritten != framesToWrite) {
        std::cerr << "Warning: Only " << framesWritten << " frames were written out of " << framesToWrite << "." << std::endl;
    }

    sf_close(file);
    std::cout << "Audio file saved successfully: " << fullFilePath << std::endl;
    return true;
}
