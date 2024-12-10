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
        delete plugin;
    }
}

void PluginHost::initialize()
{
    if (effect != nullptr)
    {
        effect->dispatcher(effect, effOpen, 0, 0, NULL, 0.0f);
        effect->dispatcher(effect, effSetSampleRate, 0, 0, NULL, 44100.0);   // hard-coded for now
        effect->dispatcher(effect, effSetBlockSize, 0, 512, NULL, 0.0f);   // hard-coded for now
        effect->setParameter(effect, 0, 0.75f); // Example: Set parameter 0 to 75%
        effect->setParameter(effect, 1, 0.5f);  // Example: Set parameter 1 to 50%        
    }
}

void PluginHost::processAudio(float** inBuffer, float** outBuffer, int numSamples)
{
    if (effect != nullptr && inBuffer != nullptr)    
    {   
        // Log buffer information       
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
        for (int paramIndex = 0; paramIndex < effect->numParams; ++paramIndex)
        {
            char paramDisplay[32];
            char paramLabel[32];

            // Retrieve the display value of the parameter
            effect->dispatcher(effect, effGetParamDisplay, paramIndex, 0, paramDisplay, 0.0f);

            // Retrieve the unit/label of the parameter (e.g., "Hz", "dB")
            effect->dispatcher(effect, effGetParamLabel, paramIndex, 0, paramLabel, 0.0f);

            std::cout << "Parameter " << paramIndex << ": " << paramDisplay << " " << paramLabel << std::endl;
        }
		effect->dispatcher(effect, effMainsChanged, 0, 1, NULL, 0.0f); // Set a plugin program
        effect->dispatcher(effect, effStopProcess, 0, 0, NULL, 0.0f);
	}
}

std::string PluginHost::getEffectName() {
    if (effect == nullptr) {
        return "No Plugin Loaded";
    }
    char effectName[256]; // Buffer para o nome do efeito
    std::memset(effectName, 0, sizeof(effectName)); // Garante que o buffer seja inicializado com 0

    // Chama o dispatcher do plugin para obter o nome do efeito
    if (effect->dispatcher(effect, effGetEffectName, 0, 0, static_cast<void*>(effectName), 0.0f) != 0){
        return std::string(effectName); // Retorna o nome do efeito
    } else {
        return "Unknown Effect"; // Caso a chamada falhe
    }
}


void PluginHost::printParameterProperties() {
    if (!effect) {
        std::cerr << "Plugin not loaded!" << std::endl;
        return;
    }

    for (int i = 0; i < effect->numParams; ++i) {
        VstParameterProperties properties;
        std::memset(&properties, 0, sizeof(VstParameterProperties)); // Certifique-se de inicializar

        // Chamada ao dispatcher para obter as propriedades
        int result = effect->dispatcher(effect, effGetParameterProperties, i, 0, &properties, 0.0f);

        if (result == 1) { // Verifica se a função retornou sucesso
            std::cout << "Parameter " << i << ":" << std::endl;
            std::cout << "  Label: " << properties.label << std::endl;
            std::cout << "  Automatable: " << ((properties.flags & kVstParameterIsAutomatable) ? "Yes" : "No") << std::endl;
            std::cout << "  Discrete: " << ((properties.flags & kVstParameterIsDiscrete) ? "Yes" : "No") << std::endl;
            std::cout << "  Category: " << static_cast<int>(properties.category) << std::endl;
            std::cout << "  Short Label: " << properties.shortLabel << std::endl;
        } else {
            std::cerr << "Failed to retrieve properties for parameter " << i << "." << std::endl;
        }
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