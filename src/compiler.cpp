// A big part of this compiler was taken from my other project Seworks.
// Also partially taken from other projects (not mine).

#include "compiler.hpp"
#include "vrtx_encoder.hpp"

#include <json-glib/json-glib.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using namespace seworks::vrtx;

const std::string DefaultExtension = ".vrtx";

std::string CompileInput = "";
std::string CompileOutput = "";

std::string ReadString(JsonNode* node, const std::string& context) {
    if (node == nullptr || !JSON_NODE_HOLDS_VALUE(node) || json_node_get_value_type(node) != G_TYPE_STRING) {
        throw std::runtime_error(context + " must be a string.");
    }
    const gchar* value = json_node_get_string(node);
    return value == nullptr ? std::string{} : std::string(value);
}

std::string RequiredString(JsonObject* object, const char* key, const std::string& context) {
    if (!json_object_has_member(object, key)) {
        throw std::runtime_error(context + " is missing '" + key + "'.");
    }
    return ReadString(json_object_get_member(object, key), context + "." + key);
}

double ReadNumber(JsonNode* node, const std::string& context) {
    if (node == nullptr || !JSON_NODE_HOLDS_VALUE(node)) {
        throw std::runtime_error(context + " must be a number.");
    }
    const GType type = json_node_get_value_type(node);
    if (type != G_TYPE_DOUBLE && type != G_TYPE_INT64 && type != G_TYPE_INT &&
        type != G_TYPE_UINT64 && type != G_TYPE_UINT) {
        throw std::runtime_error(context + " must be a number.");
    }
    return json_node_get_double(node);
}

std::vector<double> ReadNumberArray(JsonNode* node, std::size_t expected_size, const std::string& context) {
    if (node == nullptr || !JSON_NODE_HOLDS_ARRAY(node)) {
        throw std::runtime_error(context + " must be an array.");
    }
    JsonArray* array = json_node_get_array(node);
    if (json_array_get_length(array) != expected_size) {
        throw std::runtime_error(context + " must contain " + std::to_string(expected_size) + " numbers.");
    }

    std::vector<double> values;
    values.reserve(expected_size);
    for (std::size_t index = 0; index < expected_size; ++index) {
        values.push_back(ReadNumber(json_array_get_element(array, index), context));
    }
    return values;
}

Vector3 ReadVector3(JsonNode* node, const std::string& context) {
    const auto values = ReadNumberArray(node, 3, context);
    return Vector3{values[0], values[1], values[2]};
}

PropertyValue ReadProperty(const std::string& name, JsonNode* node, const std::string& context) {
    if (node == nullptr) {
        throw std::runtime_error(context + " has a null value.");
    }

    if (JSON_NODE_HOLDS_VALUE(node)) {
        const GType type = json_node_get_value_type(node);
        if (type == G_TYPE_BOOLEAN) {
            return json_node_get_boolean(node) != FALSE;
        }
        if (type == G_TYPE_STRING) {
            return ReadString(node, context);
        }
        if (type == G_TYPE_INT64 || type == G_TYPE_INT) {
            return static_cast<std::int64_t>(json_node_get_int(node));
        }
        if (type == G_TYPE_DOUBLE || type == G_TYPE_UINT64 || type == G_TYPE_UINT) {
            return json_node_get_double(node);
        }
        throw std::runtime_error(context + " has an unsupported JSON scalar type.");
    }

    if (JSON_NODE_HOLDS_ARRAY(node)) {
        JsonArray* array = json_node_get_array(node);
        const std::size_t count = json_array_get_length(array);
        if (name == "Color" && count == 3) {
            const auto values = ReadNumberArray(node, 3, context);
            return Color3{values[0], values[1], values[2]};
        }
        if (count == 2) {
            const auto values = ReadNumberArray(node, 2, context);
            return Vector2{values[0], values[1]};
        }
        if (count == 3) {
            return ReadVector3(node, context);
        }
        throw std::runtime_error(context + " arrays must contain 2 or 3 numeric values.");
    }

    if (JSON_NODE_HOLDS_OBJECT(node)) {
        JsonObject* object = json_node_get_object(node);
        const std::string type = RequiredString(object, "type", context);
        if (type == "EnumValue") {
            return EnumValue{RequiredString(object, "enumType", context), RequiredString(object, "value", context)};
        }
        if (type == "CFrame") {
            const Vector3 position = ReadVector3(json_object_get_member(object, "position"), context + ".position");
            const auto rotation = ReadNumberArray(json_object_get_member(object, "rotation"), 9, context + ".rotation");
            CFrame frame{position, {}};
            std::copy(rotation.begin(), rotation.end(), frame.rotation.begin());
            return frame;
        }
        throw std::runtime_error(context + " has unsupported typed value '" + type + "'.");
    }

    throw std::runtime_error(context + " has an unsupported JSON value.");
}

Scene ReadSceneData(const fs::path& path) {
    using ParserPtr = std::unique_ptr<JsonParser, void (*)(JsonParser*)>;
    ParserPtr parser(json_parser_new(), [](JsonParser* value) {
        if (value != nullptr) {
            g_object_unref(value);
        }
    });
    if (!parser) {
        throw std::runtime_error("Cannot create JSON parser.");
    }

    GError* error = nullptr;
    if (!json_parser_load_from_file(parser.get(), path.string().c_str(), &error)) {
        const std::string message = error == nullptr ? "invalid JSON" : error->message;
        if (error != nullptr) {
            g_error_free(error);
        }
        throw std::runtime_error("Cannot read scene input '" + path.string() + "': " + message);
    }

    JsonNode* root_node = json_parser_get_root(parser.get());
    if (root_node == nullptr || !JSON_NODE_HOLDS_OBJECT(root_node)) {
        throw std::runtime_error("Scene JSON root must be an object.");
    }
    JsonObject* root = json_node_get_object(root_node);
    if (!json_object_has_member(root, "instances") ||
        !JSON_NODE_HOLDS_ARRAY(json_object_get_member(root, "instances"))) {
        throw std::runtime_error("Scene JSON requires an 'instances' array.");
    }

    Scene scene;
    JsonArray* instances = json_object_get_array_member(root, "instances");
    for (guint index = 0; index < json_array_get_length(instances); ++index) {
        JsonNode* instance_node = json_array_get_element(instances, index);
        if (instance_node == nullptr || !JSON_NODE_HOLDS_OBJECT(instance_node)) {
            throw std::runtime_error("Each instances entry must be an object.");
        }
        JsonObject* instance_data = json_node_get_object(instance_node);
        const std::string context = "instances[" + std::to_string(index) + "]";
        const std::string class_name = RequiredString(instance_data, "class", context);
        const std::string name = RequiredString(instance_data, "name", context);

        std::optional<std::string> parent;
        if (json_object_has_member(instance_data, "parent")) {
            JsonNode* parent_node = json_object_get_member(instance_data, "parent");
            if (JSON_NODE_HOLDS_VALUE(parent_node) && json_node_get_value_type(parent_node) == G_TYPE_STRING) {
                parent = ReadString(parent_node, context + ".parent");
            } else if (!JSON_NODE_HOLDS_NULL(parent_node)) {
                throw std::runtime_error(context + ".parent must be a service/instance name or null.");
            }
        }

        const auto id = scene.AddInstance(class_name, name, std::move(parent));
        if (!json_object_has_member(instance_data, "properties")) {
            continue;
        }
        JsonNode* properties_node = json_object_get_member(instance_data, "properties");
        if (!JSON_NODE_HOLDS_OBJECT(properties_node)) {
            throw std::runtime_error(context + ".properties must be an object.");
        }
        JsonObject* properties = json_node_get_object(properties_node);
        GList* names = json_object_get_members(properties);
        for (GList* member = names; member != nullptr; member = member->next) {
            const char* property_name = static_cast<const char*>(member->data);
            scene.SetProperty(id, property_name,
                              ReadProperty(property_name, json_object_get_member(properties, property_name),
                                           context + ".properties." + property_name));
        }
        g_list_free(names);
    }
    return scene;
}

void WriteFile(const fs::path& path, const Record& bytes) {
    if (!path.parent_path().empty()) {
        fs::create_directories(path.parent_path());
    }
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Cannot open output file: " + path.string());
    }
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!output) {
        throw std::runtime_error("Cannot write output file: " + path.string());
    }
}

} 

void SetCompileInput(std::string Path) {
    CompileInput = Path;
}

void SetCompileOutput(std::string Path) {
    CompileOutput = Path;
}

bool RunCompile() {
    if (CompileInput == "") {
        if (CompileOutput != "") {
            std::cout 
            << "\033[1;31m" 
            << "ERROR: "
            << "\033[0m"
            << "-output needs a scene to compile (-compile)." << "\n";

            return false;
        }

        return true;
    }

    fs::path outputPath = CompileOutput;
    if (CompileOutput == "") {
        outputPath = fs::path(CompileInput);
        outputPath.replace_extension(DefaultExtension);
    }

    try {
        const Scene scene = ReadSceneData(CompileInput);
        const VrtxEncoder encoder;
        const Record output = encoder.EncodeScene(scene);
        WriteFile(outputPath, output);

        std::cout << "Wrote " << output.size() << " bytes to " << outputPath.string() << "\n";
    } catch (const std::exception& error) {
        std::cout 
        << "\033[1;31m" 
        << "ERROR: "
        << "\033[0m"
        << error.what() << "\n";

        return false;
    }

    return true;
}