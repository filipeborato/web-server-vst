#include "utils.h"
#include <uuid/uuid.h>
#include <algorithm>
#include <iostream>
#include <cstring>

std::string PROJECT_DIR;

void initializeProjectDir(const char* dir) {
    if (dir == nullptr || std::strlen(dir) == 0) {
        throw std::invalid_argument("Invalid project directory provided");
    }
    PROJECT_DIR = dir; // Atualiza a variável global
}

std::string getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return ""; // Nenhuma extensão encontrada
    }
    return filename.substr(dotPos + 1); // Retorna sem o ponto
}

bool isValidAudioExtension(const std::string& extension) {
    const std::vector<std::string> supportedExtensions = {"wav", "aiff", "flac", "ogg", "mp3", "aac"};
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

std::vector<float> extractPluginParams(const crow::request& req) {
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
            std::cerr << "Invalid parameter value for " << paramKey << std::endl;
        }

        ++i;
    }

    return params;
}

bool validateProjectDir(const char* dir) {
    return dir != nullptr && std::strlen(dir) > 0;
}
