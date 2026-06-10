#include "Host.h"
#include "PluginHost.h"
#include "AudioFileReader.h" // Classe para leitura de arquivos de áudio
#include <sndfile.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

// Implementação da função processAudioFile
// O áudio é processado em blocos e escrito em streaming no arquivo de saída:
// o uso de memória é constante (~alguns KB), independente da duração do arquivo.
bool Host::processAudioFile(const std::string& pluginPath,
                            const std::vector<std::pair<int, float>>& params,
                            const std::string& inputFilePath,
                            const std::string& outputFilePath,
                            bool isPreview,
                            bool fadeOut,
                            float previewStartTime /* em segundos */)
{
    // Se previewStartTime for negativo (ou "null" na lógica da aplicação), forçamos a zero
    if (previewStartTime < 0.0f) {
        previewStartTime = 0.0f;
    }

    // Carrega o plugin
    PluginHost host(pluginPath.c_str());
    if (!host.isLoaded()) {
        // Sem esta guarda, o processamento seguiria como no-op e devolveria
        // buffers não inicializados (lixo de memória) como áudio "processado".
        std::cerr << "Failed to load plugin: " << pluginPath << std::endl;
        return false;
    }

    // Obter e imprimir o nome do efeito
    std::string effectName = host.getEffectName();
    std::cout << "Loaded Effect: " << effectName << std::endl;
    host.printParameterProperties();

    // Cria o leitor de áudio (unique_ptr: liberado em qualquer caminho de saída)
    std::unique_ptr<AudioFileReader> audioReaderPtr;
    try {
        audioReaderPtr = std::make_unique<AudioFileReader>(inputFilePath);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    AudioFileReader& audioReader = *audioReaderPtr;
    const int sampleRate = audioReader.getSampleRate();

    // Inicializa o host e seta os parâmetros
    host.initialize(static_cast<float>(sampleRate));
    for (const auto& param : params) {
        host.setParameter(param.first, param.second);
    }

    std::cout << "\nAfter Setting:" << std::endl;
    host.printParameterProperties();

    // Obtém o total de frames e o número de canais
    int totalSamples = audioReader.getTotalSamples(); // totalSamples representa frames
    const int numChannels = audioReader.getNumChannels();

    if (totalSamples <= 0 || numChannels <= 0) {
        return false;
    }
    if (numChannels > 2) {
        // O pipeline de processamento só suporta mono/estéreo; com mais canais
        // os canais extras sairiam com lixo de memória.
        std::cerr << "Unsupported channel count: " << numChannels << std::endl;
        return false;
    }

    // Se for preview, vamos definir um trecho fixo de 10 segundos
    int previewStartFrame = 0;
    int previewDurationFrames = totalSamples; // valor padrão caso não seja preview

    if (isPreview) {
        // Converte o instante de início (em segundos, com fração) para frames
        previewStartFrame = static_cast<int>(previewStartTime * sampleRate);
        if (previewStartFrame >= totalSamples) {
            std::cerr << "Preview start time is outside the audio duration." << std::endl;
            return false;
        }
        // Define 10 segundos de preview
        previewDurationFrames = 10 * sampleRate;
        // Se o trecho ultrapassar o final do áudio, ajusta para o restante disponível
        if (previewStartFrame + previewDurationFrames > totalSamples) {
            previewDurationFrames = totalSamples - previewStartFrame;
        }
        // Agora, totalSamples será o número de frames a serem processados no preview
        totalSamples = previewDurationFrames;
    }

    // Calcula o número de frames para o fade-out
    int fadeOutSamples = 0;
    if (fadeOut) {
        fadeOutSamples = static_cast<int>(0.3f * sampleRate); // 300 ms
        fadeOutSamples = std::min(fadeOutSamples, totalSamples);
    }

    // Buffers por bloco (vector zera a memória — nunca devolvemos lixo)
    const int bufferSize = 512;
    std::vector<float> in0(bufferSize), in1(bufferSize);
    std::vector<float> out0(bufferSize), out1(bufferSize);
    float* audioForProcess[2] = { in0.data(), in1.data() };
    float* processedAudio[2] = { out0.data(), out1.data() };
    std::vector<float> interleaved(static_cast<size_t>(bufferSize) * numChannels);

    // Abre o arquivo de saída antes do loop — escrita em streaming, bloco a bloco.
    SF_INFO outInfo;
    std::memset(&outInfo, 0, sizeof(outInfo));
    outInfo.samplerate = sampleRate;
    outInfo.channels = numChannels;
    outInfo.format = audioReader.getFormat();

    SNDFILE* outFile = sf_open(outputFilePath.c_str(), SFM_WRITE, &outInfo);
    if (!outFile) {
        std::cerr << "Failed to open output file: " << sf_strerror(nullptr) << std::endl;
        return false;
    }

    int processedSamples = 0;
    while (processedSamples < totalSamples) {
        int samplesToRead = std::min(bufferSize, totalSamples - processedSamples);
        // Se for preview, o offset para leitura é o previewStartFrame + processedSamples
        int readOffset = isPreview ? (previewStartFrame + processedSamples) : processedSamples;

        // Ler os dados do canal 0 (deintercalados)
        audioReader.readSamples(audioForProcess[0], samplesToRead, readOffset, 0);

        // Se for mono, copia os dados para o segundo buffer; se stereo, lê o canal 1
        if (numChannels == 1) {
            std::copy(audioForProcess[0], audioForProcess[0] + samplesToRead, audioForProcess[1]);
        } else {
            audioReader.readSamples(audioForProcess[1], samplesToRead, readOffset, 1);
        }

        // Processa os dados (a função processAudio espera dois buffers: um por canal)
        host.processAudio(audioForProcess, processedAudio, samplesToRead);

        // Intercala o bloco processado aplicando o fade-out se necessário
        for (int i = 0; i < samplesToRead; ++i) {
            int currentSample = processedSamples + i;
            float multiplier = 1.0f; // Valor padrão

            if (fadeOut && currentSample >= (totalSamples - fadeOutSamples)) {
                int fadeSample = currentSample - (totalSamples - fadeOutSamples);
                multiplier = 1.0f - static_cast<float>(fadeSample) / fadeOutSamples;
            }

            interleaved[i * numChannels] = processedAudio[0][i] * multiplier;
            if (numChannels > 1) {
                interleaved[i * numChannels + 1] = processedAudio[1][i] * multiplier;
            }
        }

        // Escreve o bloco direto no arquivo de saída
        sf_count_t written = sf_writef_float(outFile, interleaved.data(), samplesToRead);
        if (written != samplesToRead) {
            std::cerr << "Failed to write audio block: " << sf_strerror(outFile) << std::endl;
            sf_close(outFile);
            return false;
        }

        processedSamples += samplesToRead;
    }

    sf_close(outFile);
    std::cout << "Audio file saved successfully: " << outputFilePath << std::endl;
    return true;
}
