#include "AudioFileReader.h"

void AudioFileReader::readAudioMetadata() {
   SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(SF_INFO)); // Certifique-se de inicializar a estrutura

    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open audio file: " << sf_strerror(file) << std::endl;
        std::cerr << "File path: " << filePath << std::endl;
        exit(1);
    }

    // Exibir informações do arquivo para debug
    std::cout << "Audio file opened successfully!" << std::endl;
    std::cout << "Sample Rate: " << sfinfo.samplerate << std::endl;
    std::cout << "Channels: " << sfinfo.channels << std::endl;
    std::cout << "Frames: " << sfinfo.frames << std::endl;
    std::cout << "Format: " << sfinfo.format << std::endl;

    // Atribuir os valores às variáveis da classe
    sampleRate = sfinfo.samplerate;
    numChannels = sfinfo.channels;
    totalSamples = sfinfo.frames * sfinfo.channels;
    format = sfinfo.format;

    sf_close(file);
}

void AudioFileReader::readSamples(float* buffer, int numSamples, int offset) {
    if (buffer == nullptr) {
        std::cerr << "Buffer is null. Unable to store audio samples." << std::endl;
        return;
    }

    // Abrir o arquivo de áudio
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open audio file: " << sf_strerror(file) << std::endl;
        return;
    }

    // Validar offset e numSamples
    if (offset < 0 || offset >= sfinfo.frames * sfinfo.channels) {
        std::cerr << "Invalid offset. It must be within the file's total samples." << std::endl;
        sf_close(file);
        return;
    }

    if (numSamples <= 0) {
        std::cerr << "Invalid number of samples to read. Must be greater than zero." << std::endl;
        sf_close(file);
        return;
    }

    // Ajustar o número de amostras se exceder o tamanho disponível
    if ((offset / sfinfo.channels) + numSamples > sfinfo.frames) {
        numSamples = sfinfo.frames - (offset / sfinfo.channels);
        if (numSamples <= 0) {
            std::cerr << "Offset is beyond the end of the audio file." << std::endl;
            sf_close(file);
            return;
        }
    }

    // Posicionar o ponteiro no trecho desejado
    if (sf_seek(file, offset / sfinfo.channels, SEEK_SET) < 0) {
        std::cerr << "Failed to seek in audio file: " << sf_strerror(file) << std::endl;
        sf_close(file);
        return;
    }

    // Ler os dados do arquivo
    sf_count_t readFrames = sf_readf_float(file, buffer, numSamples);
    if (readFrames != numSamples) {
        std::cerr << "Warning: Number of frames read is less than expected." << std::endl;
    }

    // Fechar o arquivo
    sf_close(file);
}

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

float* AudioFileReader::makeAudio(float *audio, const float* audioBuffer, int samples, int offset) {
    int j = 0;
    for (int i = offset; i < (offset + samples); i++) {
        audio[i] = audioBuffer[j];
        j++;
    }
    return audio;
}

float* AudioFileReader::cpyTotalAudio(float* audio, float* buffer, int samples, int offset) {
    // Verifique os limites dos arrays para evitar acessos inv�lidos
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

    // Determinar a extensão com base no formato de áudio
    // Determine the extension based on the main format
    std::string extension;
    int mainFormat = format & SF_FORMAT_TYPEMASK; // Extract main format

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


    // Adicionar a extensão ao caminho do arquivo, se necessário
    std::string fullFilePath = filePath + extension;

    SF_INFO sfinfo = {0}; // Inicializar com zeros para evitar valores aleatórios
    sfinfo.samplerate = sampleRate;         // Taxa de amostragem do áudio
    sfinfo.channels = numChannels;         // Número de canais (mono ou estéreo)
    sfinfo.format = format;                // Formato de saída

    SNDFILE* file = sf_open(fullFilePath.c_str(), SFM_WRITE, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open output file: " << sf_strerror(file) << std::endl;
        return false;
    }

    // Validar que o tamanho do buffer é consistente com o número de canais
    if (bufferSize % numChannels != 0) {
        std::cerr << "Buffer size is not aligned with the number of channels." << std::endl;
        sf_close(file);
        return false;
    }

    sf_count_t framesToWrite = bufferSize / numChannels; // Calcular número de frames
    sf_count_t framesWritten = sf_writef_float(file, audioBuffer, framesToWrite);

    if (framesWritten != framesToWrite) {
        std::cerr << "Warning: Only " << framesWritten << " frames were written out of " << framesToWrite << "." << std::endl;
    }

    sf_close(file);
    std::cout << "Audio file saved successfully: " << fullFilePath << std::endl;
    return true;
}
