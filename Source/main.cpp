#include <iostream>
#include <string>
#include <algorithm>
#include "AudioFileReader.h"
#include "PluginHost.h"

int main()
{
    std::string pluginPath = "/workspaces/web-server-vst/TheFunction.so";  // Path to VST2 plugin
    PluginHost host(pluginPath.c_str());

    const int bufferSize = 512;  // Block size
    //const int totalSamples = 44100 * 10;  // Example: 10 seconds of audio at 44.1 kHz
    
    AudioFileReader audioReader("/workspaces/web-server-vst/Alesis-Sanctuary-QCard-Tines-Aahs-C4.wav");
    const int totalSamples = audioReader.getTotalSamples();
    
    float* audio[2] = {new float[totalSamples], new float[totalSamples]};
    float* audioForProcess[2] = {new float[bufferSize], new float[bufferSize]};
    float* processedAudio[2] = {new float[bufferSize], new float[bufferSize]};

    host.initialize();

    int processedSamples = 0;
    while (processedSamples < totalSamples)
    {
        int samplesToRead = std::min(bufferSize, totalSamples - processedSamples);
        audioReader.readSamples(audioForProcess[0], samplesToRead, processedSamples);

        // Duplicate mono input for stereo processing
        std::copy(audioForProcess[0], audioForProcess[0] + samplesToRead, audioForProcess[1]);

        host.processAudio(audioForProcess, processedAudio, samplesToRead);

        // Copy processed audio back to the main audio buffer
        std::copy(processedAudio[0], processedAudio[0] + samplesToRead, audio[0] + processedSamples);
        std::copy(processedAudio[1], processedAudio[1] + samplesToRead, audio[1] + processedSamples);

        processedSamples += samplesToRead;
    }

    // Save processed audio
    audioReader.saveAudioToSNDFile("/workspaces/web-server-vst/test/output.wav", *audio, totalSamples);

    // Cleanup
    delete[] audio[0];
    delete[] audio[1];
    delete[] audioForProcess[0];
    delete[] audioForProcess[1];
    delete[] processedAudio[0];
    delete[] processedAudio[1];

    return 0;
}
