#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <crow.h>

extern std::string PROJECT_DIR;

// Inicializa o diretório do projeto
void initializeProjectDir(const char* dir);

// Retorna a extensão de um arquivo
std::string getFileExtension(const std::string& filename);

// Valida se uma extensão é suportada
bool isValidAudioExtension(const std::string& extension);

// Gera um UUID único
std::string generateUUID();

// Processa os parâmetros do plugin enviados via URL
#include <utility>

std::vector<std::pair<int, float>> extractPluginParams(const crow::request& req);

// Valida um caminho fornecido no argumento
bool validateProjectDir(const char* dir);

// Valida o nome do plugin vindo da query string (bloqueia path traversal)
bool isValidPluginName(const std::string& name);

// Converte um arquivo de áudio (MP3/AAC) para WAV via ffmpeg
std::string convertToWav(const std::string& inputFile);

#endif // UTILS_H
