#include "Host.h"
#include "PluginHost.h"
#include <algorithm>
#include <iostream>
#include <crow.h>
#include <fstream>
#include <string>
#include <vector>

// Diretório do projeto ajustado conforme seu ambiente
static const char* PROJECT_DIR = "/workspaces/web-server-vst";

// Função auxiliar para extrair a extensão do nome do arquivo
std::string getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return ""; // Nenhuma extensão encontrada
    }
    return filename.substr(dotPos + 1); // Retorna sem o ponto
}

// Função auxiliar para validar a extensão do arquivo
bool isValidAudioExtension(const std::string& extension) {
    // Lista de extensões de áudio suportadas
    const std::vector<std::string> supportedExtensions = {"wav", "aiff", "flac", "ogg", "mp3", "aac"};
    // Converte a extensão para minúsculas para comparação
    std::string extLower = extension;
    std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);
    return std::find(supportedExtensions.begin(), supportedExtensions.end(), extLower) != supportedExtensions.end();
}

void add_cors_headers(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin", "*"); // Permite qualquer origem
    res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, Cookie");
    res.add_header("Access-Control-Allow-Credentials", "true");
}


int main(int argc, char* argv[]) {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/process")
        .methods("POST"_method,"OPTIONS"_method)
    ([&](const crow::request& req) {
        
        crow::multipart::message msg(req);

        // Verificar se há partes no multipart
        if (msg.parts.empty()) {
            return crow::response(400, "No parts in the request");
        }

        // Acessar a primeira parte
        auto& part = msg.parts[0];

        // Obter o objeto de cabeçalho "Content-Disposition"
        auto& hdr = part.get_header_object("Content-Disposition");
        auto it = hdr.params.find("name");
        if (it != hdr.params.end())
        {
            std::string fieldName = it->second;
            if (fieldName != "audio_file" || part.body.size() == 0) {
                return crow::response(400, "Invalid audio file field");
            }
        }
        else {
            return crow::response(400, "Missing 'name' in Content-Disposition");
        }

        // Obter o nome original do arquivo
        auto filenameIt = hdr.params.find("filename");
        if (filenameIt == hdr.params.end() || filenameIt->second.empty()) {
            return crow::response(400, "Uploaded file must have a filename");
        }

        std::string originalFilename = filenameIt->second;

        // Extrair a extensão do arquivo
        std::string extension = getFileExtension(originalFilename);
        if (extension.empty()) {
            return crow::response(400, "Uploaded file does not have an extension");
        }

        // Validar a extensão do arquivo
        if (!isValidAudioExtension(extension)) {
            return crow::response(400, "Unsupported audio file extension");
        }

        // Construir o caminho do arquivo de entrada com a extensão correta
        std::string inputFile = std::string(PROJECT_DIR) + "/tmp/input_audio." + extension;

        // Salvar o arquivo de áudio enviado no caminho definido
        std::ofstream ofs(inputFile, std::ios::binary);
        if (!ofs.is_open()) {
            return crow::response(500, "Failed to create temporary input file");
        }
        ofs.write(part.body.data(), static_cast<std::streamsize>(part.body.size()));
        ofs.close();

        // Obter os parâmetros do plugin a partir dos parâmetros de URL
        std::string pluginName = req.url_params.get("plugin") ? req.url_params.get("plugin") : "";
        if (pluginName.empty()) {
            return crow::response(400, "Missing plugin parameter");
        }

        // Obter os parâmetros p0 a p5
        std::vector<float> params(7, 0.0f);
        for (int i = 0; i < 7; ++i) {
            std::string paramKey = "p" + std::to_string(i);
            const char* val = req.url_params.get(paramKey);
            if (val) {
                try {
                    params[i] = std::stof(val);
                } catch (const std::invalid_argument&) {
                    return crow::response(400, "Invalid parameter value for " + paramKey);
                }
            } else {
                return crow::response(400, "Missing parameter " + paramKey);
            }
        }

        // Construir o caminho do plugin
        std::string pluginPath = std::string(PROJECT_DIR) + "/vst/" + pluginName;

        // Definir o caminho do arquivo de saída sem extensão
        std::string outputFile = std::string(PROJECT_DIR) + "/tmp/output_audio";

        // Processar o arquivo de áudio
        Host host;
        bool success = host.processAudioFile(pluginPath, params, inputFile, outputFile);
        if (!success) {
            return crow::response(500, "Failed to process audio");
        }

        // Construir o caminho completo do arquivo de saída com a extensão correta
        std::string outputFileWithExt = outputFile + "." + extension;

        // Verificar se o arquivo de saída foi salvo corretamente
        std::ifstream checkFile(outputFileWithExt, std::ios::binary);
        if (!checkFile.good()) {
            std::cerr << "Error: Output file was not saved correctly: " << outputFileWithExt << std::endl;
            return crow::response(500, "Processed file not found");
        }

        // Ler o arquivo de saída e devolver como resposta binária
        std::ifstream ifs(outputFileWithExt, std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "Error: Failed to open output file for reading: " << outputFileWithExt << std::endl;
            return crow::response(500, "Failed to read output file");
        }

        // Ler o conteúdo do arquivo
        std::string fileContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        // Definir o Content-Type dinamicamente com base na extensão
        std::string contentType = "audio/" + extension;
        // Alguns navegadores podem não reconhecer certos tipos, então você pode mapear manualmente se necessário
        if (extension == "mp3") {
            contentType = "audio/mpeg";
        } else if (extension == "aac") {
            contentType = "audio/aac";
        }

        // Devolver o arquivo como resposta
        crow::response r;
        if (req.method == "OPTIONS"_method) {                
            add_cors_headers(r);               
        }
        r.code = 200;
        r.set_header("Content-Type", contentType);
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
