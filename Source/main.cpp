#include <iostream>
#include <string>
#include <algorithm>
#include "AudioFileReader.h"
#include "PluginHost.h"
#include <dlfcn.h>  // For dynamic library loading on Linux

// Main function
int main()
{
    // Replace with the correct path to your VST2 plugin in Linux (use a .so file)
    std::string pluginPath = "/path/to/your/plugin/AQ1_Stereo.so";  // Linux version of the plugin path
    
    PluginHost host(pluginPath.c_str()); // PluginHost constructor takes path to shared library

    const int bufferSize = 512;
    float audioBuffer[bufferSize];

    // Update path for Linux
    std::string audioFilePath = "/home/user/Desktop/test.wav"; // Replace with the actual audio file path in Linux
    AudioFileReader audioReader(audioFilePath);

    const int totalSamples = audioReader.getTotalSamples();

    int processedSamples = 0;
    float *audio = new float[totalSamples];
    float **audioForProcess = new float*[bufferSize];
    float **audioProcessed = new float*[totalSamples];

    while (processedSamples < totalSamples)
    {
        int samplesToRead = std::min(bufferSize, totalSamples - processedSamples);

        // Read audio chunk from the file
        audioReader.readSamples(audioBuffer, samplesToRead, processedSamples);

        host.initialize();

        memcpy(audioForProcess, audioBuffer, samplesToRead);

        // Process audio samples
        host.processAudio(audioForProcess, samplesToRead);

        // Set a plugin parameter
        host.setParameter(0, 0.75f);

        host.suspend();

        audioReader.cpyTotalAudio(audio, audioBuffer, samplesToRead, processedSamples);
        
        processedSamples += samplesToRead;
    }

    // Save the processed audio to a new .wav file
    const std::string outputFilePath = "/home/user/Desktop/file.wav"; // Replace with the desired output path for Linux
    audioReader.saveAudioToSNDFile(outputFilePath, audio, totalSamples);

    delete[] audio;

    return 0;
}
