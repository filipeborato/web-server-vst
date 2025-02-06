#include "Host.h"
#include "PluginHost.h"
#include "AudioFileReader.h" // Classe para leitura de arquivos de áudio
#include <algorithm>
#include <iostream>

// Implementação da função processAudioFile
// Nota: Adicionamos o parâmetro previewStartTime (em segundos) com valor default 0
bool Host::processAudioFile(const std::string& pluginPath,
                            const std::vector<float>& params,
                            const std::string& inputFilePath,
                            const std::string& outputFilePath,
                            bool isPreview,
                            bool fadeOut,
                            int previewStartTime /* em segundos, default = 0 */) 
{    
    // Se previewStartTime for negativo (ou "null" na lógica da aplicação), forçamos a zero
    if (previewStartTime < 0) {
        previewStartTime = 0;
    }

    // Carrega o plugin    
    PluginHost host(pluginPath.c_str());
    // Obter e imprimir o nome do efeito
    std::string effectName = host.getEffectName();
    std::cout << "Loaded Effect: " << effectName << std::endl;
    host.printParameterProperties();
    
    // Cria o leitor de áudio e obtém a taxa de amostragem
    AudioFileReader audioReader(inputFilePath);
    const int sampleRate = audioReader.getSampleRate();

    // Inicializa o host e seta os parâmetros
    host.initialize(static_cast<float>(sampleRate));
    for (size_t i = 0; i < params.size(); ++i) {
        host.setParameter(static_cast<int>(i), params[i]);
    }

    std::cout << "\nAfter Setting:" << std::endl;
    host.printParameterProperties();  

    // Obtém o total de frames e o número de canais
    int totalSamples = audioReader.getTotalSamples(); // totalSamples representa frames
    const int numChannels = audioReader.getNumChannels();

    if (totalSamples <= 0 || numChannels <= 0) {
        return false;
    }

    // Se for preview, vamos definir um trecho fixo de 10 segundos
    int previewStartFrame = 0;
    int previewDurationFrames = totalSamples; // valor padrão caso não seja preview

    if (isPreview) {
        // Converte o instante de início (em segundos) para frames
        previewStartFrame = previewStartTime * sampleRate;
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

    // Aloca os buffers:
    const int bufferSize = 512; 
    float* audio = new float[totalSamples * numChannels]; 
    // Para processamento, cada canal terá um buffer com "bufferSize" frames
    float* audioForProcess[2] = {
        new float[bufferSize], // Canal 0
        new float[bufferSize]  // Canal 1
    };
    float* processedAudio[2] = {
        new float[bufferSize],
        new float[bufferSize]
    };

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

        // Copia os dados processados para o buffer principal com o fade-out se necessário
        for (int i = 0; i < samplesToRead; ++i) {
            int currentSample = processedSamples + i;
            float multiplier = 1.0f; // Valor padrão

            if (fadeOut && currentSample >= (totalSamples - fadeOutSamples)) {
                int fadeSample = currentSample - (totalSamples - fadeOutSamples);
                multiplier = 1.0f - static_cast<float>(fadeSample) / fadeOutSamples;
            }

            audio[currentSample * numChannels] = processedAudio[0][i] * multiplier;
            if (numChannels > 1) {
                audio[currentSample * numChannels + 1] = processedAudio[1][i] * multiplier;
            }
        }

        processedSamples += samplesToRead;
    }

    // Salva o áudio processado
    bool saved = audioReader.saveAudioToSNDFile(outputFilePath, audio, totalSamples * numChannels);
   
    delete[] audio;
    delete[] audioForProcess[0];
    delete[] audioForProcess[1];
    delete[] processedAudio[0];
    delete[] processedAudio[1];

    return saved;
}
