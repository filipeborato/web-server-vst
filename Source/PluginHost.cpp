#include "PluginHost.h"
#include <dlfcn.h>  // Header para loading dinâmico no Linux

PluginHost::PluginHost(const char* pluginPath)
    : effect(nullptr), pluginHandle(nullptr)
{
    std::cout << "PPath: " << pluginPath << std::endl;
    pluginHandle = dlopen(pluginPath, RTLD_LAZY);
    if (!pluginHandle) {
        std::cerr << "Error loading plugin: " << dlerror() << std::endl;
        return;
    }
    
    // Procura a função VSTPluginMain
    auto vstPluginMain = (AEffect* (*)(audioMasterCallback))dlsym(pluginHandle, "VSTPluginMain");
    if (!vstPluginMain) {
        std::cerr << "Error: VSTPluginMain not found!" << std::endl;
        dlclose(pluginHandle);
        pluginHandle = nullptr;
        return;
    }

    // Cria o plugin
    pluginFuncPtr create = (pluginFuncPtr)vstPluginMain;
    effect = create(hostCallback); 
    if (!effect) {
        std::cerr << "Error: failed to create plugin instance." << std::endl;
        dlclose(pluginHandle);
        pluginHandle = nullptr;
        return;
    }

    // Abre o plugin (chamada única)
    effect->dispatcher(effect, effOpen, 0, 0, nullptr, 0.0f);
    PluginHost::pluginCategory(effect);
}

PluginHost::~PluginHost()
{
    if (effect != nullptr) {
        // Antes de fechar, desligue o áudio se estiver ligado
        suspend();

        // Agora feche o plugin
        effect->dispatcher(effect, effClose, 0, 0, NULL, 0.0f);
        effect = nullptr;
    }

    if (pluginHandle) {
        dlclose(pluginHandle);
        pluginHandle = nullptr;
    }
}

void PluginHost::initialize()
{
    if (effect != nullptr) {
        // Ajuste sample rate e block size
        effect->dispatcher(effect, effSetSampleRate, 0, 0, NULL, 44100.0f);
        effect->dispatcher(effect, effSetBlockSize, 0, 512, NULL, 0.0f);     

        // Se for necessário iniciar processamento (ligar áudio):
        effect->dispatcher(effect, effMainsChanged, 0, 1, NULL, 0.0f); 
    }
}

void PluginHost::processAudio(float** inBuffer, float** outBuffer, int numSamples)
{
    if (effect != nullptr && inBuffer != nullptr) {
        effect->processReplacing(effect, inBuffer, outBuffer, numSamples);
    }
}

void PluginHost::setParameter(int index, float value)
{
    if (effect != nullptr) {
        effect->setParameter(effect, index, value);
    }
}

void PluginHost::suspend()
{
    if (effect != nullptr) {
        // Desligar áudio (mains)
        effect->dispatcher(effect, effMainsChanged, 0, 0, NULL, 0.0f);
    }
}

std::string PluginHost::getEffectName()
{
    if (effect == nullptr) {
        return "No Plugin Loaded";
    }
    char effectName[256];
    std::memset(effectName, 0, sizeof(effectName));

    if (effect->dispatcher(effect, effGetEffectName, 0, 0, static_cast<void*>(effectName), 0.0f) != 0) {
        return std::string(effectName);
    } else {
        return "Unknown Effect";
    }
}

void PluginHost::printParameterProperties()
{
    if (!effect) {
        std::cerr << "Plugin not loaded!" << std::endl;
        return;
    }

    for (int paramIndex = 0; paramIndex < effect->numParams; ++paramIndex) {
        char paramName[64];
        char paramLabel[64];
        char paramDisplay[64];
        memset(paramLabel, 0, sizeof(paramLabel));
        memset(paramDisplay, 0, sizeof(paramDisplay));
        memset(paramName, 0, sizeof(paramName));

        effect->dispatcher(effect, effGetParamName, paramIndex, 0, paramName, 0.0f);
        effect->dispatcher(effect, effGetParamLabel, paramIndex, 0, paramLabel, 0.0f);
        effect->dispatcher(effect, effGetParamDisplay, paramIndex, 0, paramDisplay, 0.0f);

        std::cout << "Parameter " << paramIndex << " name: " << paramName 
                  << ", Label: " << paramLabel 
                  << ", Display: " << paramDisplay << std::endl;
    }
}

void PluginHost::pluginCategory(AEffect* plugin)
{
    printf("Category: ");
    VstInt32 pluginCategory = plugin->dispatcher(plugin, effGetPlugCategory, 0, 0, NULL, 0.0f);
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
            return effect->uniqueID;
        case audioMasterIdle:
            return 1;
        case __audioMasterWantMidiDeprecated:
            return 1;
        case audioMasterGetCurrentProcessLevel:
            return kVstProcessLevelRealtime;
        //default:
            //printf("\nPlugin requested value of opcode %d.\n", opcode);
        }
        return 0;
    }
}
