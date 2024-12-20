#pragma once
#include <string>
#include <vector>
#include "AudioFileReader.h"
#include "PluginHost.h"

class Host {
public:
    Host() = default;
    ~Host() = default;
    
    // Processa o arquivo de áudio de entrada com o plugin especificado
    // pluginName: caminho ou nome do plugin VST2
    // params: vetor com 6 parâmetros em float
    // inputFilePath: arquivo de áudio de entrada
    // outputFilePath: arquivo de áudio de saída (onde o resultado será salvo)
    // Retorna true se bem sucedido, false caso contrário.
    bool processAudioFile(const std::string& pluginPath,
                            const std::vector<float>& params,
                            const std::string& inputFilePath,
                            const std::string& outputFilePath,
                            bool isPreview);
};
