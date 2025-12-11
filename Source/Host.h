#ifndef HOST_H
#define HOST_H

#include <string>
#include <vector>
#include <memory>

// RAII wrapper for audio buffer management
class AudioBuffer {
public:
    explicit AudioBuffer(int size) : data(new float[size]), size(size) {}
    ~AudioBuffer() { delete[] data; }
    
    // Disable copy, allow move
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer& operator=(const AudioBuffer&) = delete;
    AudioBuffer(AudioBuffer&&) = default;
    AudioBuffer& operator=(AudioBuffer&&) = default;
    
    float* get() { return data; }
    const float* get() const { return data; }
    int getSize() const { return size; }
    
private:
    float* data;
    int size;
};

class Host {
public:
    bool processAudioFile(const std::string& pluginPath,
                            const std::vector<float>& params,
                            const std::string& inputFilePath,
                            const std::string& outputFilePath,
                            bool isPreview,
                            bool fadeOut,
                            int previewStartTime /* em segundos, default = 0 */); 
};

#endif // HOST_H
