#ifndef HOST_H
#define HOST_H

#include <string>
#include <vector>

#include <utility>

class Host {
public:
    bool processAudioFile(const std::string& pluginPath,
                            const std::vector<std::pair<int, float>>& params,
                            const std::string& inputFilePath,
                            const std::string& outputFilePath,
                            bool isPreview,
                            bool fadeOut,
                            int previewStartTime /* em segundos, default = 0 */); 
};

#endif // HOST_H
