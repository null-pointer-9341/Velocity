// Taken from my Seworks project.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace seworks::vrtx {

struct Vector2 {
    double x;
    double y;
};

struct Vector3 {
    double x;
    double y;
    double z;
};

struct Color3 {
    double r;
    double g;
    double b;
};

struct CFrame {
    Vector3 position;
    std::array<double, 9> rotation;
};

struct EnumValue {
    std::string type;
    std::string value;
};

using PropertyValue = std::variant<bool, std::int64_t, double, std::string, Vector2,
                                   Vector3, Color3, CFrame, EnumValue>;

struct Instance {
    std::string class_name;
    std::string name;
    std::optional<std::string> parent;
    std::map<std::string, PropertyValue> properties;
};

class Scene {
public:
    using InstanceId = std::size_t;

    InstanceId AddInstance(std::string class_name, std::string name,
                           std::optional<std::string> parent = std::nullopt);
    void SetProperty(InstanceId instance, std::string property, PropertyValue value);

    const std::vector<Instance>& instances() const noexcept;

private:
    std::vector<Instance> instances_;
};

using Record = std::vector<std::uint8_t>;
using RecordCodec = std::function<Record(const Instance&)>;

class VrtxEncoder {
public:
    VrtxEncoder();

    void RegisterCodec(std::string class_name, RecordCodec codec);
    std::vector<Record> EncodeRecords(const Scene& scene) const;
    Record EncodeScene(const Scene& scene) const;

private:
    std::map<std::string, RecordCodec> codecs_;
};

}  // namespace seworks::vrtx