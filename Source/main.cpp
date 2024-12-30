#include "Host.h"
#include "utils.h"
#include <crow.h>
#include <fstream>
#include <vector>

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

        if (!isValidAudioExtension(extension)) {
            return crow::response(400, "Unsupported audio file extension");
        }
        
        std::cout << "Project initialized at: " << PROJECT_DIR << std::endl;           
        
        std::string job_id = generateUUID();        
        std::string inputFile = std::string(PROJECT_DIR) + "/tmp/input_audio_" + job_id + "." + extension;

        std::ofstream ofs(inputFile, std::ios::binary);
        if (!ofs.is_open()) {
            return crow::response(500, "Failed to create temporary input file");
        }
        ofs.write(part.body.data(), static_cast<std::streamsize>(part.body.size()));
        ofs.close();
       
        std::string pluginName = req.url_params.get("plugin") ? req.url_params.get("plugin") : "";
        if (pluginName.empty()) {
            return crow::response(400, "Missing plugin parameter");
        }

        std::vector<float> params = extractPluginParams(req);
        if (params.empty()) {
            return crow::response(400, "No parameters provided");
        }

        bool isPreview = req.url_params.get("preview") ? (std::string(req.url_params.get("preview")) == "true") : false;
        bool fadeOut = req.url_params.get("fadeout") ? (std::string(req.url_params.get("fadeout")) == "true") : false;

        std::string pluginPath = std::string(PROJECT_DIR) + "/vst/" + pluginName + ".so";
        std::string outputFile = std::string(PROJECT_DIR) + "/tmp/output_audio_" + job_id;

        std::cout << "Vst path: " << pluginPath << std::endl;   
        Host host;
        bool success = host.processAudioFile(pluginPath, params, inputFile, outputFile, isPreview, fadeOut);

        if (!success) {
            return crow::response(500, "Failed to process audio");
        }

        std::string outputFileWithExt = outputFile + "." + extension;
        std::ifstream ifs(outputFileWithExt, std::ios::binary);

        if (!ifs.is_open()) {
            return crow::response(500, "Processed file not found");
        }

        std::string fileContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        std::string contentType = "audio/" + extension;
        if (extension == "mp3") contentType = "audio/mpeg";
        else if (extension == "aac") contentType = "audio/aac";
        else if (extension == "wav") contentType = "audio/wav";
        else if (extension == "ogg") contentType = "audio/ogg";
        else if (extension == "flac") contentType = "audio/flac";
        else if (extension == "aiff") contentType = "audio/aiff";

        r.code = 200;
        r.set_header("Content-Type", contentType);
        r.write(fileContent);

        return r;
    });

    app.port(18080).multithreaded().run();
}
