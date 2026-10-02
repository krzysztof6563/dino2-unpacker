#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "help.h"
#include "unpacker/DC2ModelExtractor.h"

int main(int argc, char** argv) {
    dc2::AnimationOptions animationOptions;
    std::vector<std::filesystem::path> inputs, files;
    std::filesystem::path output;
    bool raw = false;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help") {
                std::cout << "Dino Crisis 2 collection unpacker with animations\n"
                          << "Usage: dino2-unpacker [options] files-or-folders...\n"
                          << "  --output-dir=PATH     Write extracted assets into PATH\n"
                          << "  --save-chunks         Retain all archive entries and decompressed blocks\n"
                          << "  --animation-tps=60    Assumed playback tick rate\n"
                          << "  --in-place            Omit animated root translation\n"
                          << "  --no-animations       Export static rigs\n"
                          << "Folders are scanned recursively for .DAT files.\n";
                return 0;
            } else if (arg == "--save-chunks") raw = true;
            else if (arg == "--no-animations") animationOptions.enabled = false;
            else if (arg == "--in-place") animationOptions.inPlace = true;
            else if (arg.rfind("--output-dir=", 0) == 0) output = arg.substr(13);
            else if (arg.rfind("--animation-tps=", 0) == 0) {
                std::size_t used = 0;
                const auto value = arg.substr(16);
                animationOptions.ticksPerSecond = std::stod(value, &used);
                if (used != value.size() || !std::isfinite(animationOptions.ticksPerSecond) ||
                    animationOptions.ticksPerSecond <= 0 || animationOptions.ticksPerSecond > 1000000)
                    throw std::runtime_error("Invalid animation tick rate");
            } else if (arg.rfind("--", 0) == 0) throw std::runtime_error("Unknown option: " + arg);
            else inputs.emplace_back(arg);
        }
        if (inputs.empty()) { std::cout << "Drag DAT files or a Data folder onto Extract.bat, or use --help.\n"; return 1; }
        std::set<std::string> seen;
        for (const auto& path : inputs) {
            if (std::filesystem::is_directory(path)) {
                for (const auto& item : std::filesystem::recursive_directory_iterator(path)) {
                    std::string ext = item.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::toupper(c); });
                    if (item.is_regular_file() && ext == ".DAT") files.push_back(item.path());
                }
            } else files.push_back(path);
        }
        std::sort(files.begin(), files.end());
        DC2ModelExtractor::setAnimationOptions(animationOptions);
        DC2ModelExtractor::setSaveArchiveEntries(raw);
        if (!output.empty()) std::filesystem::create_directories(output);
        QJsonArray results;
        int failed = 0, archived = 0, special = 0, fallback = 0;
        for (const auto& path : files) {
            const auto absolute = std::filesystem::absolute(path).lexically_normal();
            if (!seen.insert(absolute.string()).second) continue;
            QJsonObject item;
            item["source"] = QString::fromStdString(absolute.string());
            // Preserve distinct same-named files from separate directories.
            auto destination = output;
            if (!output.empty()) {
                auto name = path.filename().string();
                auto candidate = destination / name;
                if (std::any_of(results.begin(), results.end(), [&](const QJsonValue& r) {
                    return r.toObject()["output_prefix"].toString() == QString::fromStdString(candidate.string());
                })) {
                    destination /= std::to_string(results.size());
                    std::filesystem::create_directories(destination);
                }
            }
            DC2ModelExtractor::setOutputDirectory(destination.string());
            const auto prefix = output.empty() ? absolute : destination / path.filename();
            item["output_prefix"] = QString::fromStdString(prefix.string());
            int status = 1;
            bool specialContainer = false;
            try {
                status = DC2ModelExtractor::extract(absolute.string());
                if (status == 2) {
                    status = DC2ModelExtractor::extractOtherContainer(absolute.string());
                    specialContainer = status == 0;
                }
                if (status == 2) {
                    // FILE/ITEM/MAP use other containers. Preserve them explicitly;
                    // do not describe a raw copy as a successful animation decode.
                    std::ifstream src(absolute, std::ios::binary);
                    std::ofstream dest(prefix.string() + ".raw.bin", std::ios::binary);
                    dest << src.rdbuf();
                    status = src && dest ? 2 : 1;
                }
            } catch (const std::exception& ex) {
                item["error"] = ex.what(); status = 1;
            }
            item["status"] = status == 0 ? (specialContainer ? "special_container_extracted" : "archive_extracted") : status == 2 ? "other_container_preserved_raw" : "failed";
            if (status == 1) ++failed;
            else if (status == 2) ++fallback;
            else if (specialContainer) ++special;
            else ++archived;
            results.append(item);
            std::cout << "[INFO] " << path.filename().string() << ": " << item["status"].toString().toStdString() << '\n';
        }
        QJsonObject report;
        report["files"] = results;
        report["archives_extracted"] = archived;
        report["special_containers_extracted"] = special;
        report["other_containers_preserved_raw"] = fallback;
        report["failed"] = failed;
        const auto reportPath = output.empty() ? files.front().parent_path() / "Batch_Report.json" : output / "Batch_Report.json";
        std::ofstream summary(reportPath, std::ios::binary);
        const auto bytes = QJsonDocument(report).toJson();
        summary.write(bytes.constData(), bytes.size());
        if (!summary) ++failed;
        std::cout << "[INFO] Batch complete: " << archived << " archives, " << special << " specialized containers, " << fallback
                  << " other containers preserved raw, " << failed << " failures.\n";
        return failed ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "[ERROR] " << error.what() << '\n'; return 1;
    }
}
