#include "utils.h"
#include <uuid/uuid.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <cstdlib>  // Para system()
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

// Função para converter MP3 para WAV usando FFmpeg
std::string convertMp3ToWav(const std::string& mp3File) {
    std::string wavFile = mp3File.substr(0, mp3File.find_last_of('.')) + ".wav";
    std::string command = "ffmpeg -i " + mp3File + " -ar 44100 -ac 2 -f wav " + wavFile + " -loglevel error";

    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("Error converting MP3 to WAV. FFmpeg failed.");
    }

    return wavFile;
}
