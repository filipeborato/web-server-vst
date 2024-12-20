#include "Host.h"
#include "PluginHost.h"
#include <algorithm>
#include <iostream>
#include <crow.h>
#include <fstream>
#include <string>
#include <vector>
#include <uuid/uuid.h>

// Diretório do projeto ajustado conforme seu ambiente
static const char* PROJECT_DIR = nullptr;

void initializeProjectDir(const char* dir) {
    static char buffer[1024];
    std::strncpy(buffer, dir, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Garante o null-terminator
    PROJECT_DIR = buffer;
}

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

std::string generateUUID() {
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[37]; // 36 caracteres + null terminator
    uuid_unparse(uuid, uuid_str);
    return std::string(uuid_str);
}

int main(int argc, char* argv[]) {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/process")
        .methods("POST"_method,"OPTIONS"_method)
    ([&](const crow::request& req) {        
        crow::multipart::message msg(req);
        crow::response r;
       
        r.add_header("Access-Control-Allow-Origin", "*");
        r.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        r.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        r.add_header("Access-Control-Allow-Credentials", "true");

        if (req.method == "OPTIONS"_method) {
            r.code = 204; // Pré-flight OPTIONS
            r.end();
            return crow::response(204, "options");
        }

        // Verificar se há partes no multipart
        if (msg.parts.empty()) {
            std::cerr << "No parts in the request " << std::endl;
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

        if (argc < 2) {
            std::cerr << "Uso: " << argv[0] << " <project_dir>" << std::endl;
            return crow::response(400, "Invalid project dir" );
        }

        initializeProjectDir(argv[1]);

        std::cout << "Projeto inicializado em: " << PROJECT_DIR << std::endl;

        std::string job_id = generateUUID();

        // Construir o caminho do arquivo de entrada com a extensão correta
        std::string inputFile = std::string(PROJECT_DIR) + "/tmp/input_audio_" + job_id + "." + extension;

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

       std::vector<float> params;
        int i = 0;

        while (true) {
            std::string paramKey = "p" + std::to_string(i);
            const char* val = req.url_params.get(paramKey);

            if (!val) {
                break; // Sai do loop quando não há mais parâmetros
            }

            try {
                params.push_back(std::stof(val));
            } catch (const std::invalid_argument&) {
                return crow::response(400, "Invalid parameter value for " + paramKey);
            }

            ++i;
        }

        // Verificar se pelo menos um parâmetro foi encontrado
        if (params.empty()) {
            return crow::response(400, "No parameters provided");
        }

        // Obter o parâmetro preview
        bool isPreview = false;
        if (req.url_params.get("preview")) {
            std::string previewParam = req.url_params.get("preview");
            isPreview = (previewParam == "true");
        }        

        // Construir o caminho do plugin
        std::string pluginPath = std::string(PROJECT_DIR) + "/vst/" + pluginName + ".so";

        // Definir o caminho do arquivo de saída sem extensão
        std::string outputFile = std::string(PROJECT_DIR) + "/tmp/output_audio_" + job_id + "." + extension;

        // Processar o arquivo de áudio
        Host host;
        bool success = host.processAudioFile(pluginPath, params, inputFile, outputFile, isPreview);
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
    bool success = host.processAudioFile(std::string(PROJECT_DIR) + "/vst/" + pluginName, params, inputFile, outputFile, false);
    return;
}
