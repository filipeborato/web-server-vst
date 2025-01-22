/*
  ==============================================================================

    PluginHost.h
    Created: 8 Sep 2023 9:33:20am
    Author:  Filipe Borato

  ==============================================================================
*/

#pragma once
#include <string>
#include <iostream>
#include <cstring>

// Inclua somente o que for necessário do VST2
#include "pluginterfaces/vst2.x/aeffectx.h"

// Definições auxiliares
typedef AEffect* (*pluginFuncPtr)(audioMasterCallback host);

constexpr int kVstParameterIsAutomatable = 1 << 2; // Automatable parameter flag
constexpr int kVstParameterIsDiscrete = 0;    // Discrete parameter flag

class PluginHost
{
public:
    PluginHost(const char* pluginPath);
    ~PluginHost();

    void initialize();
    void suspend();
    void processAudio(float** inBuffer, float** outBuffer, int numSamples);
    void setParameter(int index, float value);
    std::string getEffectName();
    void printParameterProperties();

    static void pluginCategory(AEffect* plugin);

private:
    AEffect* effect;
    void* pluginHandle; // Para armazenar o handle retornado por dlopen
};

extern "C" {
    VstIntPtr VSTCALLBACK hostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
}
