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
std::vector<float> extractPluginParams(const crow::request& req);

// Valida um caminho fornecido no argumento
bool validateProjectDir(const char* dir);

#endif // UTILS_H
