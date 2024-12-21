#ifndef HOST_H
#define HOST_H

#include <string>
#include <vector>

class Host {
public:
    bool processAudioFile(const std::string& pluginPath,
                          const std::vector<float>& params,
                          const std::string& inputFilePath,
                          const std::string& outputFilePath,
                          bool isPreview,
                          bool fadeOut); // Adicionada a flag fadeOut
};

#endif // HOST_H
