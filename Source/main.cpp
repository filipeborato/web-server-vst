#include "Host.h"
#include <crow.h>
#include <fstream>
#include <iostream>
#include <string>

// Este diretório seria ajustado conforme seu ambiente
static const char* PROJECT_DIR="/workspaces/web-server-vst";

// Exemplo: rota: 
// POST /process?plugin=TheFunction.so&p0=1.0&p1=1.0&p2=1.0&p3=0.5&p4=0.5&p5=0.0&p6=1.0
// Body: multipart/form-data com um campo "audio_file" contendo o arquivo de áudio
int main(int argc, char* argv[]) {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/process")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        crow::multipart::message msg(req);

        // Obter parâmetros do plugin
        std::string pluginName = req.url_params.get("plugin") ? req.url_params.get("plugin") : "";
        if (pluginName.empty()) {
            return crow::response(400, "Missing plugin parameter");
        }

        std::vector<float> params(7, 0.0f);
        for (int i = 0; i < 7; ++i) {
            std::string paramKey = "p" + std::to_string(i);
            const char* val = req.url_params.get(paramKey);
            if (val) {
                params[i] = std::stof(val);
            } else {
                return crow::response(400, "Missing parameter " + paramKey);
            }
        }

        // Arquivo de áudio (campo "audio_file")
        // Supondo que o upload foi feito como multipart/form-data
        // e que o arquivo está no primeiro part
        if (msg.parts.size() == 0) {
            return crow::response(400, "No file uploaded");
        }

        auto& part = msg.parts[0];
        auto& hdr = part.get_header_object("Content-Disposition");
        auto it = hdr.params.find("name");
        if (it != hdr.params.end())
        {
            std::string fieldName = it->second;
            if (fieldName != "audio_file" || part.body.size() == 0) {
                return crow::response(400, "Invalid audio file field");
            }
        }    

        // Salvar arquivo temporariamente
        // Aqui seria bom usar um diretório temporário do seu sistema
        std::string inputFile = std::string(PROJECT_DIR) + "/tmp/input_audio.wav";
        std::ofstream ofs(inputFile, std::ios::binary);
        ofs.write(part.body.data(), (std::streamsize)part.body.size());
        ofs.close();

        std::string outputFile = std::string(PROJECT_DIR) + "/tmp/output_audio";
        std::string pluginPath = std::string(PROJECT_DIR) + "/vst/" + pluginName;

        Host host;
        bool success = host.processAudioFile(pluginPath, params, inputFile, outputFile);
        if (!success) {
            return crow::response(500, "Failed to process audio");
        }

        // Ler o arquivo de saída e devolver como resposta binária
        std::ifstream ifs(outputFile, std::ios::binary);
        if (!ifs.is_open()) {
            return crow::response(500, "Failed to read output file");
        }
        std::string fileContent((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

        crow::response r;
        r.code = 200;
        r.set_header("Content-Type", "audio/wav");
        r.write(fileContent);
        return r;
    });

    app.port(18080).multithreaded().run();
}
void test(){
    std::string inputFile = std::string(PROJECT_DIR) + "/Alesis-Sanctuary-QCard-Tines-Aahs-C4.wav";   
    std::string outputFile =  std::string(PROJECT_DIR) + "/tmp/output_audio";
    std::string pluginName = "TheFunction.so";
    std::vector<float> params(7, 0.0f);
    params[0] = 1.0;
    params[1] = 1.0;
    params[2] = 1.0;
    params[3] = 0.5;
    params[4] = 0.5;
    params[5] = 0.0;
    params[6] = 1.0;

    Host host;
    bool success = host.processAudioFile(std::string(PROJECT_DIR) + "/vst/" + pluginName, params, inputFile, outputFile);
    return;
}
