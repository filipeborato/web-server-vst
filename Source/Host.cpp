#include "Host.h"
#include "PluginHost.h"
#include "AudioFileReader.h" // Supondo que você tenha essa classe para leitura de arquivos de áudio
#include <algorithm>
#include <iostream>

// Implementação da função processAudioFile
bool Host::processAudioFile(const std::string& pluginPath,
                            const std::vector<float>& params,
                            const std::string& inputFilePath,
                            const std::string& outputFilePath,
                            bool isPreview,
                            bool fadeOut) 
{    
    // Carrega o plugin    
    PluginHost host(pluginPath.c_str());
    // Obter e imprimir o nome do efeito
    std::string effectName = host.getEffectName();
    std::cout << "Loaded Effect: " << effectName << std::endl;
    host.printParameterProperties();

    // Inicializa o host e seta parâmetros
    host.initialize();
    for (size_t i = 0; i < params.size(); ++i) {
        host.setParameter((int)i, params[i]);
    }

    std::cout << "\nAfter Setting:" << std::endl;
    host.printParameterProperties();  

    AudioFileReader audioReader(inputFilePath);
    int totalSamples = audioReader.getTotalSamples();
    const int sampleRate = audioReader.getSampleRate();
    const int numChannels = audioReader.getNumChannels();

    if (totalSamples <= 0 || numChannels <= 0) {
        return false;
    }

    if (isPreview) {
        const int previewSamples = sampleRate * 5; // 5 segundos
        totalSamples = std::min(totalSamples, previewSamples);
    }

    // Calcular o número de amostras para o fade-out
    int fadeOutSamples = 0;
    if (fadeOut) {
        fadeOutSamples = static_cast<int>(0.3f * sampleRate); // 300 ms
        // Assegure-se de que fadeOutSamples não exceda totalSamples
        fadeOutSamples = std::min(fadeOutSamples, totalSamples);
    }

    // Aloca buffers
    const int bufferSize = 512; 
    float* audio = new float[totalSamples * numChannels]; 
    float* audioForProcess[2] = {new float[bufferSize * numChannels], new float[bufferSize * numChannels]};
    float* processedAudio[2] = {new float[bufferSize * numChannels], new float[bufferSize * numChannels]};

    int processedSamples = 0;
    while (processedSamples < totalSamples) {
        int samplesToRead = std::min(bufferSize, totalSamples - processedSamples);

        // Ler dados do primeiro canal
        audioReader.readSamples(audioForProcess[0], samplesToRead, processedSamples);

        if (numChannels == 1) {
            std::copy(audioForProcess[0], audioForProcess[0] + samplesToRead, audioForProcess[1]);
        } else {
            audioReader.readSamples(audioForProcess[1], samplesToRead, processedSamples);
        }

        // Processar
        host.processAudio(audioForProcess, processedAudio, samplesToRead);

        // Copiar de volta para buffer principal com FadeOut
        for (int i = 0; i < samplesToRead; ++i) {
            int currentSample = processedSamples + i;
            float multiplier = 1.0f; // Valor padrão para amostras fora do fade-out

            if (fadeOut && currentSample >= (totalSamples - fadeOutSamples)) {
                int fadeSample = currentSample - (totalSamples - fadeOutSamples);
                if (fadeSample < fadeOutSamples) {
                    multiplier = 1.0f - static_cast<float>(fadeSample) / fadeOutSamples;
                } else {
                    multiplier = 0.0f;
                }
            }

            audio[currentSample * numChannels] = processedAudio[0][i] * multiplier;
            if (numChannels > 1) {
                audio[currentSample * numChannels + 1] = processedAudio[1][i] * multiplier;
            }
        }

        processedSamples += samplesToRead;
    }

    // Salvar áudio processado
    bool saved = audioReader.saveAudioToSNDFile(outputFilePath, audio, totalSamples * numChannels);
   
    delete[] audio;
    delete[] audioForProcess[0];
    delete[] audioForProcess[1];
    delete[] processedAudio[0];
    delete[] processedAudio[1];

    return saved;
}
