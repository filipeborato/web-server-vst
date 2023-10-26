/*
  ==============================================================================

    PluginHost.h
    Created: 8 Sep 2023 9:33:20am
    Author:  Filipe Borato

  ==============================================================================
*/

#pragma once
#include "vstsdk2.4/public.sdk/source/vst2.x/audioeffect.h" 
#include "vstsdk2.4/public.sdk/source/vst2.x/audioeffectx.h"
#include <JuceHeader.h>

typedef AEffect* (*pluginFuncPtr)(audioMasterCallback host);
//==============================================================================
/*
*/
class PluginHost
{
public:
    PluginHost(const char* pluginPath);
    ~PluginHost();
    void initialize();
    void pluginCategory(AEffect* plugin);
    void processAudio(float* buffer, int numSamples);
    void setParameter(int index, float value);
    void loadAudioFile(const std::string& filePath, float* audioBuffer, int bufferSize);
    void saveAudioToFile(const std::string& filePath, const float* audioBuffer, int bufferSize);


private:
    AudioEffect* plugin; // Pointer to the loaded VST2 plugin
    AEffect* host;
};

extern "C" {
    VstIntPtr VSTCALLBACK hostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
}