#ifndef HOST_H
#define HOST_H

#include <string>
#include <vector>

#include <utility>

class Host {
public:
    // outputFilePath: caminho completo do arquivo de saída (com extensão).
    // previewStartTime: em segundos (aceita fração, ex.: 0.08).
    bool processAudioFile(const std::string& pluginPath,
                            const std::vector<std::pair<int, float>>& params,
                            const std::string& inputFilePath,
                            const std::string& outputFilePath,
                            bool isPreview,
                            bool fadeOut,
                            float previewStartTime);
};

#endif // HOST_H
