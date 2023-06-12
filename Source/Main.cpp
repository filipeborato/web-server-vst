/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

//#include <JuceHeader.h>
//#include "vstsdk2.4/pluginterfaces/vst2.x/aeffect.h"
//#include "vstsdk2.4/public.sdk/source/vst2.x/audioeffect.h"
#include <windows.h>

#include "vstsdk2.4/public.sdk/source/vst2.x/audioeffectx.h" // Include the VST2 SDK header

// PluginHost class

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
        typedef AudioEffect* (*createFunc)();
        createFunc create = (createFunc)GetProcAddress(hInstance, "VSTPluginMain");
        if (create != nullptr)
        {
            plugin = create(); // Instantiate the plugin
            plugin->dispatcher(effOpen, 0, 0, nullptr, 0.0f); // Open the plugin
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
    PluginHost host("path/to/your/plugin.dll"); // Replace with the path to your VST2 plugin

    host.initialize();

    // Process audio samples
    float audioBuffer[512];
    host.processAudio(audioBuffer, 512);

    // Set a plugin parameter
    host.setParameter(0, 0.75f);

    return 0;
}
