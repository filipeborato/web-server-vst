#include <windows.h>
#include <iostream>
#include "vstsdk2.4/public.sdk/source/vst2.x/audioeffect.h" 
#include "vstsdk2.4/public.sdk/source/vst2.x/audioeffectx.h" // Include the VST2 SDK header

// PluginHost class

typedef AEffect* (*pluginFuncPtr)(audioMasterCallback host);    // plugin's entry point

extern "C" {
    VstIntPtr VSTCALLBACK hostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
}

class PluginHost
{
public:
    PluginHost(const char* pluginPath);
    ~PluginHost();
    void initialize();
    void processAudio(float* buffer, int numSamples);
    void setParameter(int index, float value);

private:
    AudioEffect* plugin; // Pointer to the loaded VST2 plugin
};

PluginHost::PluginHost(const char* pluginPath)
{
    plugin = nullptr;

    // Load the VST2 plugin
    HINSTANCE hInstance = LoadLibrary(pluginPath);
    if (hInstance != nullptr)
    {        
        pluginFuncPtr create = (pluginFuncPtr)GetProcAddress(hInstance, "VSTPluginMain");
        if (create != nullptr)
        {
            AEffect* plugin;
            plugin = create(hostCallback); // Instantiate the plugin
            plugin->dispatcher(plugin, effOpen, 0, 0, NULL, 0.0f);  // Open the plugin
        }
    }
}

PluginHost::~PluginHost()
{
    if (plugin != nullptr)
    {
        plugin->dispatcher(effClose, 0, 0, nullptr, 0.0f); // Close the plugin
        delete plugin;
    }
}

void PluginHost::initialize()
{
    if (plugin != nullptr)
    {
        plugin->setParameterAutomated(0, 0.5f); // Set a parameter (index 0) to a value (0.5f)
        plugin->setSampleRate(44100.0f); // Set the sample rate
        plugin->setBlockSize(512); // Set the block size
    }
}

void PluginHost::processAudio(float* buffer, int numSamples)
{
    if (plugin != nullptr)
    {
        plugin->processReplacing(&buffer, nullptr, numSamples); // Process audio
    }
}

void PluginHost::setParameter(int index, float value)
{
    if (plugin != nullptr)
    {
        plugin->setParameter(index, value); // Set a plugin parameter
    }
}

// Main function
int main()
{
    PluginHost host("C:/Users/filip/Desktop/projetos/again/again.dll"); // Replace with the path to your VST2 plugin
    
    host.initialize();

    // Process audio samples
    float audioBuffer[512];
    host.processAudio(audioBuffer, 512);

    // Set a plugin parameter
    host.setParameter(0, 0.75f);

    return 0;
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
