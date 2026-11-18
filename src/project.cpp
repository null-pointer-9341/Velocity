#include "framework.hpp"
#include "project.hpp"
#include "vrtx_encoder.hpp"

#include <json-glib/json-glib.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using namespace seworks::vrtx;

// constants
const std::string SourceRoot = "src";
const std::string ConfigFolder = "config";
const std::string ConfigFile = "project.json";
const std::string OutputFolder = "build";
const std::string OutputExtension = ".vrtx";
const std::string ConfigNameKey = "name";
const std::string ConfigVersionKey = "version";
const std::string DefaultProjectName = "Place";
const std::string DefaultProjectVersion = "0.1.0";

struct SourceFolder {
    std::string Folder;
    std::string Service;
};

SourceFolder SourceFolders[] = {
    {
        "server", 
        "ServerScriptService"
    },
    {
        "client", 
        "StarterPlayerScripts"
    },
    {
        "shared", 
        "ReplicatedStorage"
    },
};

const int SourceFolderCount = sizeof(SourceFolders) / sizeof(SourceFolders[0]);

struct ScriptKind {
    std::string Extension;
    std::string ClassName;
};

ScriptKind ScriptKinds[] = {
    {
        ".luau.server", 
        "Script"
    },
    {
        ".luau.client", 
        "LocalScript"
    },
    {
        ".luau", 
        "ModuleScript"
    },
};

const int ScriptKindCount = sizeof(ScriptKinds) / sizeof(ScriptKinds[0]);

// Cyan and bold "INFO: " print
void PrintInfo(std::string Message) {
    std::cout
    << "\033[1;36m"
    << "INFO: "
    << "\033[0m"
    << Message << "\n";
}

// Red and bold and cool "ERROR: " print
void PrintError(std::string Message) {
    std::cout 
    << "\033[1;31m" 
    << "ERROR: "
    << "\033[0m"
    << Message << "\n";
}

ScriptKind* FindScriptKind(std::string FileName) {
    for (int index = 0; index < ScriptKindCount; index++) {
        const std::string extension = ScriptKinds[index].Extension;

        if (FileName.size() <= extension.size()) {
            continue;
        }

        if (FileName.compare(FileName.size() - extension.size(), extension.size(), extension) == 0) {
            return &ScriptKinds[index];
        }
    }
    return nullptr;
}

std::string ReadFileText(fs::path Path) {
    std::ifstream file(Path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open " + Path.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool IsIdentifierStart(char character) {
    return (character >= 'a' && character <= 'z') ||
           (character >= 'A' && character <= 'Z') || character == '_';
}

bool IsIdentifierPart(char character) {
    return IsIdentifierStart(character) || (character >= '0' && character <= '9');
}

bool ReadLongBracket(const std::string& source, std::size_t offset,
                     std::size_t& openingLength, std::string& closing) {
    if (offset >= source.size() || source[offset] != '[') {
        return false;
    }

    std::size_t cursor = offset + 1;
    while (cursor < source.size() && source[cursor] == '=') {
        cursor++;
    }
    if (cursor >= source.size() || source[cursor] != '[') {
        return false;
    }

    openingLength = cursor - offset + 1;
    closing = "]" + source.substr(offset + 1, cursor - offset - 1) + "]";
    return true;
}

std::string ExpandFrameworkAliases(const std::string& source, bool includeFramework) {
    const std::string frameworkExpression =
        "require(game:GetService(\"ReplicatedStorage\").Velocity.Framework)";
    const std::string signalExpression =
        "require(game:GetService(\"ReplicatedStorage\").Velocity.Signal)";
    std::string result;
    result.reserve(source.size());

    for (std::size_t index = 0; index < source.size();) {
        const std::size_t tokenStart = index;
        const char character = source[index];

        if (character == '-' && index + 1 < source.size() && source[index + 1] == '-') {
            index += 2;
            std::size_t openingLength = 0;
            std::string closing;
            if (ReadLongBracket(source, index, openingLength, closing)) {
                index += openingLength;
                const std::size_t end = source.find(closing, index);
                index = end == std::string::npos ? source.size() : end + closing.size();
            } else {
                const std::size_t end = source.find('\n', index);
                index = end == std::string::npos ? source.size() : end;
            }
            result.append(source, tokenStart, index - tokenStart);
            continue;
        }

        if (character == '\'' || character == '"') {
            const char quote = character;
            index++;
            while (index < source.size()) {
                if (source[index] == '\\') {
                    index = std::min(index + 2, source.size());
                } else if (source[index++] == quote) {
                    break;
                }
            }
            result.append(source, tokenStart, index - tokenStart);
            continue;
        }

        std::size_t openingLength = 0;
        std::string closing;
        if (ReadLongBracket(source, index, openingLength, closing)) {
            index += openingLength;
            const std::size_t end = source.find(closing, index);
            index = end == std::string::npos ? source.size() : end + closing.size();
            result.append(source, tokenStart, index - tokenStart);
            continue;
        }

        if (IsIdentifierStart(character)) {
            index++;
            while (index < source.size() && IsIdentifierPart(source[index])) {
                index++;
            }
            const std::string identifier = source.substr(tokenStart, index - tokenStart);
            if (identifier == "_framework" || identifier == "_signal") {
                if (!includeFramework) {
                    throw std::runtime_error("'" + identifier + "' requires the framework; build with -f.");
                }
                result += identifier == "_framework" ? frameworkExpression : signalExpression;
            } else {
                result += identifier;
            }
            continue;
        }

        result += character;
        index++;
    }

    return result;
}

void WriteBytes(fs::path Path, const Record& Bytes) {
    if (!Path.parent_path().empty()) {
        fs::create_directories(Path.parent_path());
    }

    std::ofstream output(Path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Cannot open output file: " + Path.string());
    }

    output.write(reinterpret_cast<const char*>(Bytes.data()), static_cast<std::streamsize>(Bytes.size()));
    if (!output) {
        throw std::runtime_error("Cannot write output file: " + Path.string());
    }
}

std::string EscapeJson(std::string Text) {
    std::string result = "";
    for (std::size_t index = 0; index < Text.size(); index++) {
        char letter = Text[index];
        if (letter == '\\' || letter == '"') {
            result += '\\';
        }
        result += letter;
    }
    return result;
}

std::string ReadConfigString(std::string Key, std::string Fallback) {
    const fs::path path = fs::path(ConfigFolder) / ConfigFile;
    if (!fs::exists(path)) {
        return Fallback;
    }

    std::unique_ptr<JsonParser, void (*)(JsonParser*)> parser(json_parser_new(), [](JsonParser* value) {
        g_object_unref(value);
    });

    GError* error = nullptr;
    if (!json_parser_load_from_file(parser.get(), path.string().c_str(), &error)) {
        const std::string message = error == nullptr ? "invalid JSON" : error->message;
        if (error != nullptr) {
            g_error_free(error);
        }
        throw std::runtime_error("Cannot read '" + path.string() + "': " + message);
    }

    JsonNode* rootNode = json_parser_get_root(parser.get());
    if (rootNode == nullptr || !JSON_NODE_HOLDS_OBJECT(rootNode)) {
        throw std::runtime_error("'" + path.string() + "' must contain a json object.");
    }

    JsonObject* root = json_node_get_object(rootNode);
    if (!json_object_has_member(root, Key.c_str())) {
        return Fallback;
    }

    JsonNode* valueNode = json_object_get_member(root, Key.c_str());
    if (!JSON_NODE_HOLDS_VALUE(valueNode) || json_node_get_value_type(valueNode) != G_TYPE_STRING) {
        throw std::runtime_error("'" + Key + "' in '" + path.string() + "' must be a string.");
    }

    const gchar* value = json_node_get_string(valueNode);
    if (value == nullptr || std::string(value) == "") {
        return Fallback;
    }

    return std::string(value);
}

}

bool RunInit() {
    try {
        for (int index = 0; index < SourceFolderCount; index++) {
            const fs::path path = fs::path(SourceRoot) / SourceFolders[index].Folder;

            if (fs::create_directories(path)) {
                PrintInfo("Created " + path.string());
            } else {
                PrintInfo("Already exists: " + path.string());
            }
        }
    } catch (const std::exception& error) {
        PrintError(error.what());

        return false;
    }

    return true;
}

bool RunConfig() {
    const fs::path path = fs::path(ConfigFolder) / ConfigFile;

    if (fs::exists(path)) {
        PrintInfo("Already exists: " + path.string());

        return true;
    }

    try {
        fs::create_directories(path.parent_path());

        std::string projectName = fs::current_path().filename().string();
        if (projectName == "") {
            projectName = DefaultProjectName;
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot write " + path.string());
        }

        file 
            << "{" << "\n"
            << "    \"" << ConfigNameKey << "\": \"" << EscapeJson(projectName) << "\"," << "\n"
            << "    \"" << ConfigVersionKey << "\": \"" << DefaultProjectVersion << "\"" << "\n"
            << "}" << "\n";

        PrintInfo("Created " + path.string());
    } catch (const std::exception& error) {
        PrintError(error.what());

        return false;
    }

    return true;
}

bool RunBuild(bool includeFramework) {
    try {
        if (!fs::exists(SourceRoot)) {
            throw std::runtime_error("There is no '" + SourceRoot + "' folder, run `velocity init` first.");
        }

        const std::string projectName = ReadConfigString(ConfigNameKey, DefaultProjectName);

        Scene scene;
        int scriptCount = 0;

        for (int folderIndex = 0; folderIndex < SourceFolderCount; folderIndex++) {
            const SourceFolder& sourceFolder = SourceFolders[folderIndex];
            const fs::path folderPath = fs::path(SourceRoot) / sourceFolder.Folder;

            if (!fs::exists(folderPath)) {
                continue;
            }

            std::vector<fs::path> files;
            std::vector<fs::path> directories;
            for (const fs::directory_entry& entry : fs::recursive_directory_iterator(folderPath)) {
                if (entry.is_directory()) {
                    directories.push_back(entry.path());
                    continue;
                }

                files.push_back(entry.path());
            }
            std::sort(directories.begin(), directories.end(), [](const fs::path& left, const fs::path& right) {
                const auto leftDepth = std::distance(left.begin(), left.end());
                const auto rightDepth = std::distance(right.begin(), right.end());
                if (leftDepth != rightDepth) {
                    return leftDepth < rightDepth;
                }
                return left < right;
            });
            std::sort(files.begin(), files.end());

            for (const fs::path& directory : directories) {
                const fs::path relativePath = directory.lexically_relative(folderPath);
                const fs::path parentPath = relativePath.parent_path();
                const std::string parent = parentPath.empty()
                    ? sourceFolder.Service
                    : parentPath.filename().string();
                scene.AddInstance("Folder", relativePath.filename().string(), parent);
            }

            for (const fs::path& file : files) {
                const std::string fileName = file.filename().string();

                ScriptKind* kind = FindScriptKind(fileName);
                if (kind == nullptr) {
                    continue;
                }

                const std::string scriptName = fileName.substr(0, fileName.size() - kind->Extension.size());
                const fs::path relativeParent = file.parent_path().lexically_relative(folderPath);
                const std::string parent = relativeParent.empty() || relativeParent == "."
                    ? sourceFolder.Service
                    : relativeParent.filename().string();
                Scene::InstanceId id = scene.AddInstance(kind->ClassName, scriptName, parent);
                scene.SetProperty(id, "Source", ExpandFrameworkAliases(ReadFileText(file), includeFramework));

                scriptCount++;
            }
        }

        if (includeFramework) {
            AddFrameworkModules(scene);
        }

        VrtxEncoder encoder;

        const Record output = encoder.EncodeScene(scene);

        const fs::path outputPath = fs::path(OutputFolder) / (projectName + OutputExtension);
        WriteBytes(outputPath, output);

        PrintInfo("Compiled " + std::to_string(scriptCount) + " scripts into " + outputPath.string() + " (" + std::to_string(output.size()) + " bytes)");
    } catch (const std::exception& error) {
        PrintError(error.what());

        return false;
    }

    return true;
}