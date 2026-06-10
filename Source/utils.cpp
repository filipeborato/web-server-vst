#include "utils.h"
#include <uuid/uuid.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <cstdlib>  // Para system()
#include <cctype>
#include <stdexcept>

std::string PROJECT_DIR;

void initializeProjectDir(const char* dir) {
    if (dir == nullptr || std::strlen(dir) == 0) {
        throw std::invalid_argument("Invalid project directory provided");
    }
    PROJECT_DIR = dir;
}

std::string getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return ""; // Nenhuma extensão encontrada
    }
    return filename.substr(dotPos + 1);
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

std::vector<std::pair<int, float>> extractPluginParams(const crow::request& req) {
    std::vector<std::pair<int, float>> params;

    for (int i = 0; i < 256; ++i) {
        std::string paramKey = "p" + std::to_string(i);
        const char* val = req.url_params.get(paramKey);

        if (val) {
            try {
                params.push_back({i, std::stof(val)});
            } catch (const std::invalid_argument&) {
                std::cerr << "Invalid parameter value for " << paramKey << std::endl;
            }
        }
    }

    return params;
}

bool validateProjectDir(const char* dir) {
    return dir != nullptr && std::strlen(dir) > 0;
}

// Valida o nome do plugin vindo da query string: apenas [A-Za-z0-9._-],
// sem ".." — impede path traversal no dlopen (ex.: plugin=../../lib/x).
bool isValidPluginName(const std::string& name) {
    if (name.empty() || name.front() == '.' || name.find("..") != std::string::npos) {
        return false;
    }
    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_' && c != '.') {
            return false;
        }
    }
    return true;
}

// Converte um arquivo de áudio (MP3/AAC) para WAV usando FFmpeg.
// -y: sobrescreve sem perguntar (sem -y, o ffmpeg trava esperando stdin se o .wav já existir);
// </dev/null: garante que nunca há prompt interativo; caminhos entre aspas.
std::string convertToWav(const std::string& inputFile) {
    std::string wavFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".wav";
    std::string command = "ffmpeg -y -i \"" + inputFile + "\" -ar 44100 -ac 2 -f wav \"" +
                          wavFile + "\" -loglevel error </dev/null";

    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("Error converting audio to WAV. FFmpeg failed.");
    }

    return wavFile;
}
