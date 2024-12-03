#define NOMINMAX
#include <iostream>
#include <string>
#include <algorithm>
#include "AudioFileReader.h"
#include "PluginHost.h"
#include <windows.h>

// Main function
int main()
{
    PluginHost host("C:/Program Files/VSTPlugIns/AQ1 Stereo.dll"); // Replace with the path to your VST2 plugin
    
    const int bufferSize = 512;
    float audioBuffer[bufferSize];

    // Load the audio file and fill the audio buffer
    std::string audioFilePath = "C:/Users/filip/Desktop/test.wav"; // Replace this with the actual audio file path
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
    const std::string outputFilePath = "C:/Users/filip/Desktop/file.wav"; // Replace this with the desired output path
    audioReader.saveAudioToSNDFile(outputFilePath, audio, totalSamples);

    delete[] audio;

    return 0;
}