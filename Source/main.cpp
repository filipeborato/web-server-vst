#include <iostream>
#include <string>
#include <algorithm>
#include "AudioFileReader.h"
#include "PluginHost.h"
int main() {
    std::string pluginPath = "/workspaces/web-server-vst/TheFunction.so";  // Path to VST2 plugin
    PluginHost host(pluginPath.c_str());

    const int bufferSize = 512;  // Block size
    AudioFileReader audioReader("/workspaces/web-server-vst/Alesis-Sanctuary-QCard-Tines-Aahs-C4.wav");

    const int totalSamples = audioReader.getTotalSamples();
    const int numChannels = audioReader.getNumChannels();

    // Allocate buffers for processing
    float* audio = new float[totalSamples];
    float* audioForProcess[2] = {new float[bufferSize], new float[bufferSize]};
    float* processedAudio[2] = {new float[bufferSize], new float[bufferSize]};

    host.initialize();

    int processedSamples = 0;
    while (processedSamples < totalSamples) {
        int samplesToRead = std::min(bufferSize, totalSamples - processedSamples);
        audioReader.readSamples(audioForProcess[0], samplesToRead, processedSamples);

        if (numChannels == 1) {
            // Mono: Duplicate for stereo processing
            std::copy(audioForProcess[0], audioForProcess[0] + samplesToRead, audioForProcess[1]);
        } else {
            // Stereo: Read both channels
            audioReader.readSamples(audioForProcess[1], samplesToRead, processedSamples + bufferSize);
        }

        // Process audio
        host.processAudio(audioForProcess, processedAudio, samplesToRead);

        // Copy processed audio back to the main buffer
        for (int i = 0; i < samplesToRead; ++i) {
            audio[(processedSamples + i) * numChannels] = processedAudio[0][i];
            if (numChannels > 1) {
                audio[(processedSamples + i) * numChannels + 1] = processedAudio[1][i];
            }
        }

        processedSamples += samplesToRead;
    }

    // Save processed audio
    audioReader.saveAudioToSNDFile("/workspaces/web-server-vst/test/output.wav", audio, totalSamples);

    // Cleanup
    delete[] audio;
    delete[] audioForProcess[0];
    delete[] audioForProcess[1];
    delete[] processedAudio[0];
    delete[] processedAudio[1];

    return 0;
}
