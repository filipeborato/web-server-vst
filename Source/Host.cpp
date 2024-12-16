#include "Host.h"
#include <algorithm>
#include <iostream>

bool Host::processAudioFile(const std::string& pluginPath,
                            const std::vector<float>& params,
                            const std::string& inputFilePath,
                            const std::string& outputFilePath) 
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

    // Leitura do arquivo de entrada
    AudioFileReader audioReader(inputFilePath);
    const int totalSamples = audioReader.getTotalSamples();
    const int numChannels = audioReader.getNumChannels();

    if (totalSamples <= 0 || numChannels <= 0) {
        std::cerr << "Invalid audio file or metadata." << std::endl;
        return false;
    }

    // Aloca buffers
    const int bufferSize = 512; 
    float* audio = new float[totalSamples * numChannels]; 
    float* audioForProcess[2] = {new float[bufferSize], new float[bufferSize]};
    float* processedAudio[2] = {new float[bufferSize], new float[bufferSize]};

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

        // Copiar de volta para buffer principal
        for (int i = 0; i < samplesToRead; ++i) {
            audio[(processedSamples + i) * numChannels] = processedAudio[0][i];
            if (numChannels > 1) {
                audio[(processedSamples + i) * numChannels + 1] = processedAudio[1][i];
            }
        }

        processedSamples += samplesToRead;
    }

    // Salvar áudio processado
    bool saved = audioReader.saveAudioToSNDFile(outputFilePath, audio, totalSamples * numChannels);

    // Libera memória
    delete[] audio;
    delete[] audioForProcess[0];
    delete[] audioForProcess[1];
    delete[] processedAudio[0];
    delete[] processedAudio[1];

    return saved;
}
