#include "Host.h"
#include "utils.h"
#include <crow.h>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <vector>
#include <iostream>

// Remove os arquivos temporários do request ao sair do handler (qualquer caminho)
struct TempFileGuard {
    std::vector<std::string> paths;
    void add(const std::string& p) { paths.push_back(p); }
    ~TempFileGuard() {
        for (const auto& p : paths) {
            std::remove(p.c_str());
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2 || !validateProjectDir(argv[1])) {
        std::cerr << "Usage: " << argv[0] << " <project_dir>" << std::endl;
        return 1;
    }

    initializeProjectDir(argv[1]);

    crow::SimpleApp app;

    CROW_ROUTE(app, "/process")
        .methods("POST"_method, "OPTIONS"_method)
    ([&](const crow::request& req) {
        crow::response r;

        r.add_header("Access-Control-Allow-Origin", "*");
        r.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        r.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        r.add_header("Access-Control-Allow-Credentials", "true");

        if (req.method == "OPTIONS"_method) {
            return crow::response(204, "options");
        }

        crow::multipart::message msg(req);
        if (msg.parts.empty()) {
            return crow::response(400, "No parts in the request");
        }

        auto& part = msg.parts[0];
        auto& hdr = part.get_header_object("Content-Disposition");

        auto it = hdr.params.find("name");
        if (it == hdr.params.end() || it->second != "audio_file" || part.body.empty()) {
            return crow::response(400, "Invalid audio file field");
        }

        auto filenameIt = hdr.params.find("filename");
        if (filenameIt == hdr.params.end() || filenameIt->second.empty()) {
            return crow::response(400, "Uploaded file must have a filename");
        }

        std::string originalFilename = filenameIt->second;
        std::string extension = getFileExtension(originalFilename);
        // Normaliza para minúsculo: "musica.MP3" precisa entrar no fluxo de conversão
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (!isValidAudioExtension(extension)) {
            return crow::response(400, "Unsupported audio file extension");
        }

        std::cout << "Project initialized at: " << PROJECT_DIR << std::endl;

        std::string job_id = generateUUID();
        std::string inputFile = std::string(PROJECT_DIR) + "/tmp/input_audio_" + job_id + "." + extension;

        TempFileGuard tempFiles;
        tempFiles.add(inputFile);

        std::ofstream ofs(inputFile, std::ios::binary);
        if (!ofs.is_open()) {
            return crow::response(500, "Failed to create temporary input file");
        }
        ofs.write(part.body.data(), static_cast<std::streamsize>(part.body.size()));
        ofs.close();

        // Formatos que o libsndfile não lê são convertidos para WAV via ffmpeg.
        // (MP3/AAC sempre; OGG porque o libsndfile vendorizado foi compilado sem Vorbis.)
        if (extension == "mp3" || extension == "aac" || extension == "ogg") {
            try {
                inputFile = convertToWav(inputFile);
                extension = "wav"; // Atualiza a extensão
                tempFiles.add(inputFile);
            } catch (const std::exception& e) {
                return crow::response(500, "Failed to convert audio to WAV");
            }
        }

        std::string pluginName = req.url_params.get("plugin") ? req.url_params.get("plugin") : "";
        if (pluginName.empty()) {
            return crow::response(400, "Missing plugin parameter");
        }
        if (!isValidPluginName(pluginName)) {
            return crow::response(400, "Invalid plugin name");
        }

        std::string pluginPath = std::string(PROJECT_DIR) + "/vst/" + pluginName + ".so";
        bool pluginExists = std::ifstream(pluginPath).good();

        bool isPreview = req.url_params.get("preview") ? (std::string(req.url_params.get("preview")) == "true") : false;
        bool fadeOut = req.url_params.get("fadeout") ? (std::string(req.url_params.get("fadeout")) == "true") : false;

        // Aceita fração de segundo (o front manda ex.: 0.08); valor inválido vira 400
        float previewStartTime = 0.0f;
        if (const char* pst = req.url_params.get("previewStartTime")) {
            try {
                previewStartTime = std::stof(std::string(pst));
            } catch (const std::exception&) {
                return crow::response(400, "Invalid previewStartTime");
            }
        }

        std::string outputFileWithExt = std::string(PROJECT_DIR) + "/tmp/output_audio_" + job_id + "." + extension;
        tempFiles.add(outputFileWithExt);

        bool success = false;
        if (!pluginExists) {
            std::cout << "Plugin " << pluginName << " not found. Falling back to pass-through." << std::endl;
            std::ifstream src(inputFile, std::ios::binary);
            std::ofstream dst(outputFileWithExt, std::ios::binary);
            if (src.is_open() && dst.is_open()) {
                dst << src.rdbuf();
                success = true;
            } else {
                success = false;
            }
        } else {
            std::vector<std::pair<int, float>> params = extractPluginParams(req);
            if (params.empty()) {
                return crow::response(400, "No parameters provided");
            }
            Host host;
            success = host.processAudioFile(pluginPath, params, inputFile, outputFileWithExt, isPreview, fadeOut, previewStartTime);
        }

        if (!success) {
            return crow::response(500, "Failed to process audio");
        }

        std::ifstream ifs(outputFileWithExt, std::ios::binary);

        if (!ifs.is_open()) {
            return crow::response(500, "Processed file not found");
        }

        std::string fileContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        std::string contentType = "audio/" + extension;
        r.code = 200;
        r.set_header("Content-Type", contentType);
        r.write(fileContent);

        return r;
    });

    CROW_ROUTE(app, "/plugins")
        .methods("GET"_method, "OPTIONS"_method)
    ([&](const crow::request& req) {
        crow::response r;
        r.add_header("Access-Control-Allow-Origin", "*");
        r.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        r.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        r.add_header("Access-Control-Allow-Credentials", "true");

        if (req.method == "OPTIONS"_method) {
            return crow::response(204, "options");
        }

        std::string registryPath = std::string(PROJECT_DIR) + "/vst_registry.json";
        std::ifstream ifs(registryPath, std::ios::binary);
        if (!ifs.is_open()) {
            r.code = 404;
            r.write("vst_registry.json not found");
            return r;
        }

        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        r.code = 200;
        r.set_header("Content-Type", "application/json");
        r.write(content);
        return r;
    });

    CROW_ROUTE(app, "/")
        .methods("GET"_method)
    ([&](const crow::request& req) {        
        return crow::response(200, "alive");
    });

    app.port(18080).multithreaded().run();
}
