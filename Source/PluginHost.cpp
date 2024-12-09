/*
  ==============================================================================

    PluginHost.cpp
    Created: 8 Sep 2023 9:33:20am
    Author:  Filipe Borato

  ==============================================================================
*/
#include "PluginHost.h"
//#include <windows.h>
#include <dlfcn.h>  // Header for dynamic loading in Linux

//==============================================================================
PluginHost::PluginHost(const char* pluginPath)
{
    effect = nullptr;

    // Load the VST2 plugin
    void* pluginHandle = dlopen(pluginPath, RTLD_LAZY);
    if (!pluginHandle) {
        std::cerr << "Error loading plugin: " << dlerror() << std::endl;
        return ;
    }
    
    // Procura a função VSTPluginMain
    auto vstPluginMain = (AEffect* (*)(audioMasterCallback))dlsym(pluginHandle, "VSTPluginMain");
    if (!vstPluginMain) {
        std::cerr << "Error: VSTPluginMain not found!" << std::endl;
        dlclose(pluginHandle);
        return;
    }
    
      // Load the shared library
    if (pluginHandle != nullptr)
    {
        // Get the address of the VST plugin main entry point
        pluginFuncPtr create = (pluginFuncPtr)dlsym(pluginHandle, "VSTPluginMain");

        if (create != nullptr)
        {            
            effect = create(hostCallback); // Instantiate the plugin                  
            effect->dispatcher(effect, effOpen, 0, 0, nullptr, 0.0f); // Open the host            
            PluginHost::pluginCategory(effect);
        }
        else
        {
            std::cerr << "Error: Failed to find VSTPluginMain function in plugin." << std::endl;
            dlclose(pluginHandle);  // Close the plugin handle if function not found
            return;
        }
    }
    else
    {
        std::cerr << "Error: Unable to load plugin: " << dlerror() << std::endl;
        return;
    }
}

PluginHost::~PluginHost()
{
    if (effect != nullptr)
    {
        suspend();
        effect->dispatcher(effect, effClose, 0, 0, NULL, 0.0f); // Close the plugin
        //delete plugin;
    }
}

void PluginHost::initialize()
{
    if (effect != nullptr)
    {
        effect->dispatcher(effect, effOpen, 0, 0, NULL, 0.0f);
        effect->dispatcher(effect, effSetSampleRate, 0, 0, NULL, 44100.0);   // hard-coded for now
        effect->dispatcher(effect, effSetBlockSize, 0, 512, NULL, 0.0f);   // hard-coded for now
        
    }
}

void PluginHost::processAudio(float** inBuffer, float** outBuffer, int numSamples)
{
    if (effect != nullptr && inBuffer != nullptr)    
    {   
        // Log buffer information
        std::cout << "Processing " << numSamples << " samples." << std::endl;
        std::cout << "Input Buffer[0]: " << inBuffer[0] << ", Output Buffer[0]: " << outBuffer[0] << std::endl;    
        effect->processReplacing(effect, inBuffer, outBuffer, numSamples); // Process audio
    }
}

void PluginHost::setParameter(int index, float value)
{
    if (effect != nullptr)
    {
        effect->setParameter(effect, index, value); // Set a plugin parameter
    }
}

void PluginHost::suspend()
{
	if (effect != nullptr)
	{
		effect->dispatcher(effect, effMainsChanged, 0, 1, NULL, 0.0f); // Set a plugin program
        effect->dispatcher(effect, effStopProcess, 0, 0, NULL, 0.0f);
	}
}

void PluginHost::pluginCategory(AEffect* plugin) {
    printf("Category: ");
    VstInt32 pluginCategory = plugin->dispatcher(plugin, effGetPlugCategory, 0, 0, NULL, 0.0f); // strangely enough this query returns either kPlugCategSynth or kPlugCategUnknown only so the rest of the switch statement is ineffective
    switch (pluginCategory) {
    case kPlugCategUnknown:
        printf("Unknown, category not implemented.\n");
        break;
    case kPlugCategEffect:
        printf("Simple Effect.\n");
        break;
    case kPlugCategSynth:
        printf("VST Instrument (Synths, samplers, etc.)\n");
        break;
    case kPlugCategAnalysis:
        printf("Scope, Tuner, etc.\n");
        break;
    case kPlugCategMastering:
        printf("Dynamics, etc.\n");
        break;
    case kPlugCategSpacializer:
        printf("Panners, etc.\n");
        break;
    case kPlugCategRoomFx:
        printf("Delays and Reverbs.\n");
        break;
    case kPlugSurroundFx:
        printf("Dedicated surround processor.\n");
        break;
    case kPlugCategRestoration:
        printf("Denoiser, etc.\n");
        break;
    case kPlugCategOfflineProcess:
        printf("Offline Process.\n");
        break;
    case kPlugCategShell:
        printf("Plug-in is container of other plug-ins (shell plug-in).\n");
        break;
    case kPlugCategGenerator:
        printf("Tone Generator.\n");
        break;
    default:
        printf("Plugin type: other, category %d.\n", pluginCategory);
        break;
    }
}

extern "C" {
    VstIntPtr VSTCALLBACK hostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
    {
        switch (opcode) {
        case audioMasterVersion:
            return kVstVersion;

        case audioMasterCurrentId:
            return effect->uniqueID;    // use the current plugin ID; needed by VST shell plugins to determine which sub-plugin to load

        case audioMasterIdle:
            return 1;   // ignore this call (as per Steinberg: "Give idle time to Host application, e.g. if plug-in editor is doing mouse tracking in a modal loop.") 

        case __audioMasterWantMidiDeprecated:
            return 1;

        case audioMasterGetCurrentProcessLevel:
            return kVstProcessLevelRealtime;

        default:
            printf("\nPlugin requested value of opcode %d.\n", opcode);
        }
        #if TRACE
                printf("\nOpcode %d was requested by the plugin.\n", opcode);
        #endif  // TRACE
    }
}