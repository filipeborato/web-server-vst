// TestProcessAudio.cpp
#include "PluginHost.h"
#include <iostream>
#include <cstring>
#include <algorithm>

// --- Definições Dummy para simular um plugin VST2 ---

// Dispatcher dummy: simplesmente retorna 0 para todos os opcodes.
VstIntPtr dummyDispatcher(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
    return 0;
}

// processReplacing dummy: soma 1.0 a cada sample (assumindo 2 canais)
void dummyProcessReplacing(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames) {
    for (int i = 0; i < sampleFrames; ++i) {
        outputs[0][i] = inputs[0][i] + 1.0f;
        outputs[1][i] = inputs[1][i] + 1.0f;
    }
}

// Cria um efeito dummy para testes.
AEffect* createDummyEffect(audioMasterCallback hostCallback) {
    AEffect* dummy = new AEffect;
    dummy->dispatcher = dummyDispatcher;
    dummy->processReplacing = dummyProcessReplacing;
    dummy->numParams = 0; // Nenhum parâmetro para teste.
    return dummy;
}

// --- "Accessor" para acessar os membros privados de PluginHost ---
// Supondo que PluginHost seja um layout padrão, a estrutura a seguir
// tem a mesma disposição dos membros. Essa técnica é um hack para testes.
struct PluginHostAccessor {
    AEffect* effect;
    void* pluginHandle;
};

int main() {
    // Cria uma instância do PluginHost (o caminho não será usado para carregar o plugin real)
    PluginHost host("/home/filipec/projetos/web-server-vst/vst/libcompass_gravitator.so");

    // "Hack": reinterpret_cast para acessar os membros privados de PluginHost.
    PluginHostAccessor* accessor = reinterpret_cast<PluginHostAccessor*>(&host);
    accessor->effect = createDummyEffect(hostCallback);

    // Inicializa o plugin com uma taxa de amostragem de 44100 Hz.
    host.initialize(44100.0f);

    // Define o tamanho do bloco e o número de canais.
    const int blockSize = 512;
    const int numChannels = 2;

    // Aloca buffers de entrada para cada canal.
    float* inBuffers[2];
    inBuffers[0] = new float[blockSize];
    inBuffers[1] = new float[blockSize];

    // Preenche os buffers de entrada com dados de teste:
    // Canal 0: rampa (0, 1, 2, ...).
    // Canal 1: constante (0.5f).
    for (int i = 0; i < blockSize; ++i) {
        inBuffers[0][i] = static_cast<float>(i);
        inBuffers[1][i] = 0.5f;
    }

    // Aloca buffers de saída para cada canal e inicializa com zero.
    float* outBuffers[2];
    outBuffers[0] = new float[blockSize];
    outBuffers[1] = new float[blockSize];
    std::memset(outBuffers[0], 0, blockSize * sizeof(float));
    std::memset(outBuffers[1], 0, blockSize * sizeof(float));

    // Cria arrays de ponteiros conforme esperado pela função processReplacing.
    float* inBufferPtrs[2] = { inBuffers[0], inBuffers[1] };
    float* outBufferPtrs[2] = { outBuffers[0], outBuffers[1] };

    // Chama o processamento usando o método processAudio do host.
    host.processAudio(inBufferPtrs, outBufferPtrs, blockSize);

    // Exibe os primeiros 10 samples de cada canal da saída para ver o resultado.
    std::cout << "Resultados do processamento (primeiros 10 samples):\n";
    std::cout << "Canal 0: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << outBuffers[0][i] << " ";
    }
    std::cout << "\nCanal 1: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << outBuffers[1][i] << " ";
    }
    std::cout << std::endl;

    // Libera a memória alocada.
    delete[] inBuffers[0];
    delete[] inBuffers[1];
    delete[] outBuffers[0];
    delete[] outBuffers[1];

    // Limpa o efeito dummy.
    accessor->effect = nullptr;

    return 0;
}
