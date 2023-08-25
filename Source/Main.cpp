#include <iostream>
#include <string>
#include <algorithm>
#include "AudioFileReader.h"

#include <windows.h>
#include <sndfile.h>
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
    void pluginCategory(AEffect* plugin);
    void processAudio(float* buffer, int numSamples);
    void setParameter(int index, float value);
    void loadAudioFile(const std::string& filePath, float* audioBuffer, int bufferSize);
    void saveAudioToFile(const std::string& filePath, const float* audioBuffer, int bufferSize);


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
            PluginHost::pluginCategory(plugin);
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
// Function to load audio file and fill the audio buffer
void PluginHost::loadAudioFile(const std::string& filePath, float* audioBuffer, int bufferSize) {
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfInfo);

    if (!file) {
        std::cerr << "Error opening the input file: " << filePath << std::endl;
        return;
    }

    sf_readf_float(file, audioBuffer, bufferSize);

    sf_close(file);
}


void PluginHost::saveAudioToFile(const std::string& filePath, const float* audioBuffer, int bufferSize) {
    SF_INFO sfInfo;
    sfInfo.channels = 1; // Mono audio
    sfInfo.samplerate = 44100; // Sample rate (adjust as per your requirements)
    sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; // 16-bit PCM WAV file format

    SNDFILE* file = sf_open(filePath.c_str(), SFM_WRITE, &sfInfo);
    if (!file) {
        std::cerr << "Error opening the output file: " << filePath << std::endl;
        return;
    }

    // Write the audio data to the file
    sf_count_t framesWritten = sf_writef_float(file, audioBuffer, bufferSize);

    if (framesWritten != bufferSize) {
        std::cerr << "Error writing audio data to the file: " << filePath << std::endl;
    }

    // Close the file
    sf_close(file);
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

// Main function
int main()
{
    PluginHost host("C:/Program Files/VSTPlugIns/AQ1 Stereo.dll"); // Replace with the path to your VST2 plugin
    
    host.initialize();

    const int bufferSize = 44100;
    float audioBuffer[bufferSize];

    // Load the audio file and fill the audio buffer
    std::string audioFilePath = "C:/Users/filip/Desktop/test.wav"; // Replace this with the actual audio file path
    AudioFileReader audioReader(audioFilePath);

    const int totalSamples = audioReader.getTotalSamples();
    std::cout << "Total samples in the audio file: " << totalSamples << std::endl;
    
    //host.loadAudioFile(audioFilePath, audioBuffer, bufferSize);
    audioReader.readSamples(audioBuffer, bufferSize);

    // Process audio samples
    host.processAudio(audioBuffer, bufferSize);

    // Set a plugin parameter
    host.setParameter(0, 0.75f);

    // Save the processed audio to a new .wav file
    std::string outputFilePath = "C:/Users/filip/Desktop/file.wav"; // Replace this with the desired output path
    host.saveAudioToFile(outputFilePath, audioBuffer, bufferSize);

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
