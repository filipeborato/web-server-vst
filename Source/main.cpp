#include <iostream>
#include <string>
#include <algorithm>
#include "AudioFileReader.h"
#include "PluginHost.h"

const char* PROJECT_DIR="/workspaces/web-server-vst";

int main() {    
    std::string pluginPath = std::string(PROJECT_DIR) + "/vst/TheFunction.so";
    PluginHost host(pluginPath.c_str());

    // Obter e imprimir o nome do efeito
    std::string effectName = host.getEffectName();
    std::cout << "Loaded Effect: " << effectName << std::endl;
    host.printParameterProperties();
    
    const int bufferSize = 512;  // Tamanho do bloco
    AudioFileReader audioReader( std::string(PROJECT_DIR)+"/Alesis-Sanctuary-QCard-Tines-Aahs-C4.wav");

    const int totalSamples = audioReader.getTotalSamples();
    const int numChannels = audioReader.getNumChannels();

    if (totalSamples <= 0 || numChannels <= 0) {
        std::cerr << "Error: Invalid audio file or metadata." << std::endl;
        return 1;
    }

    // Alocar buffer principal para todos os canais
    float* audio = new float[totalSamples * numChannels];  // Buffer principal
    float* audioForProcess[2] = {new float[bufferSize], new float[bufferSize]};
    float* processedAudio[2] = {new float[bufferSize], new float[bufferSize]};

    host.initialize();

    int processedSamples = 0;
    while (processedSamples < totalSamples) {
        int samplesToRead = std::min(bufferSize, totalSamples - processedSamples);

        // Ler os dados do primeiro canal
        audioReader.readSamples(audioForProcess[0], samplesToRead, processedSamples);

        if (numChannels == 1) {
            // Mono: duplicar para o segundo canal
            std::copy(audioForProcess[0], audioForProcess[0] + samplesToRead, audioForProcess[1]);
        } else {
            // Estéreo: Ler o segundo canal
            audioReader.readSamples(audioForProcess[1], samplesToRead, processedSamples);
        }

        // Processar o áudio
        host.processAudio(audioForProcess, processedAudio, samplesToRead);

        // Copiar o áudio processado de volta para o buffer principal
        for (int i = 0; i < samplesToRead; ++i) {
            audio[(processedSamples + i) * numChannels] = processedAudio[0][i]; // Canal esquerdo
            if (numChannels > 1) {
                audio[(processedSamples + i) * numChannels + 1] = processedAudio[1][i]; // Canal direito
            }
        }

        processedSamples += samplesToRead;
    }

    // Salvar o áudio processado    
    audioReader.saveAudioToSNDFile(std::string(PROJECT_DIR)+"/test/processed_audio", audio, totalSamples * numChannels);

    // Liberar memória
    delete[] audio;
    delete[] audioForProcess[0];
    delete[] audioForProcess[1];
    delete[] processedAudio[0];
    delete[] processedAudio[1];

    return 0;
}
