/*
  ==============================================================================

    PluginHost.h
    Created: 8 Sep 2023 9:33:20am
    Author:  Filipe Borato

  ==============================================================================
*/

#pragma once
#include "public.sdk/source/vst2.x/audioeffect.h" 
#include "public.sdk/source/vst2.x/audioeffectx.h"
#include "pluginterfaces/vst2.x/aeffectx.h"
//#include <string>  // Add this line
#include <iostream>
#include <cstring>

typedef AEffect* (*pluginFuncPtr)(audioMasterCallback host);
constexpr int kVstParameterIsAutomatable = 1 << 2; // Automatable parameter flag
constexpr int kVstParameterIsDiscrete = 1 << 6;    // Discrete parameter flag
//==============================================================================
/*
*/
class PluginHost
{
public:
    PluginHost(const char* pluginPath);
    ~PluginHost();
    void initialize();
    void suspend();
    void pluginCategory(AEffect* plugin);
    void processAudio(float** inBuffer, float** outBuffer, int numSamples);
    void setParameter(int index, float value);
    std::string getEffectName();
    void printParameterProperties();
    void loadAudioFile(const std::string& filePath, float* audioBuffer, int bufferSize);
    void saveAudioToFile(const std::string& filePath, const float* audioBuffer, int bufferSize);


private:
    AudioEffect* plugin; // Pointer to the loaded VST2 plugin
    AEffect* effect;
};

extern "C" {
    VstIntPtr VSTCALLBACK hostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
}