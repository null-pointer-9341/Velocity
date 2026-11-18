//
//  When I created this encoder for Seworks I forgot that I will later on need to read it.
//  Due to that fact, I, the creator of this file, declare it impossible to debug unless you are some sort of mastermind.
//
//  Might as well leave it as is :(
//  Update this value so the next collaborator can know how many people spent their time on this hell.
int timeSpent = 2; // Hours
//  Thanks for trying to fix my code :D
//

#include "vrtx_encoder.hpp"

#include <zstd.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <limits>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace seworks::vrtx {
namespace {

constexpr std::uint8_t kSceneMagic[] = {'V', 'R', 'T', 'X'};
constexpr std::uint8_t kSceneVersion = 5;

constexpr unsigned char kServicesBytes[] = {
    0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x6f, 0x72, 0x6b, 0x73, 0x70, 0x61, 0x63, 0x65, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x69, 0x67, 0x68, 0x74, 0x69, 0x6e, 0x67,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
    0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x65, 0x70, 0x6c, 0x69, 0x63,
    0x61, 0x74, 0x65, 0x64, 0x53, 0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x53, 0x63, 0x72, 0x69, 0x70,
    0x74, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x53, 0x74, 0x61, 0x72, 0x74, 0x65, 0x72, 0x50, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x53,
    0x63, 0x72, 0x69, 0x70, 0x74, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x53, 0x74, 0x61, 0x72, 0x74, 0x65, 0x72, 0x47, 0x75, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x53, 0x74, 0x61, 0x72, 0x74, 0x65, 0x72, 0x50, 0x61, 0x63, 0x6b, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

constexpr unsigned char kPartRecordBytes[] = {
    0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x61, 0x72, 0x74,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x61, 0x72, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x3f, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x40, 0xff, 0xff,
    0x7f, 0x3f, 0xff, 0xff, 0x7f, 0x3f, 0xff, 0xff, 0x7f, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

constexpr unsigned char kFooterBytes[] = {
    0xff, 0xff, 0x7f, 0x3f, 0xff, 0xff, 0x7f, 0x3f, 0xff, 0xff, 0x7f, 0x3f, 0x00, 0x00, 0x80, 0x3f,
    0x00, 0x00, 0xfa, 0x44, 0x00, 0x00, 0x80, 0x3f, 0x8f, 0xc2, 0x75, 0x3f, 0x66, 0x66, 0x66, 0x3f,
    0x00, 0x00, 0x80, 0x3f, 0x00, 0xa0, 0x0c, 0x46, 0x01, 0x3d, 0xdc, 0xbb, 0xbe, 0x22, 0xed, 0xb9,
    0xbe, 0x8d, 0x19, 0x22, 0xbe, 0x26, 0x79, 0x57, 0x3f, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x60, 0x41, 0x00, 0x00, 0x00, 0x00, 0x07, 0x98,
    0xee, 0x26, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};

std::uint64_t ReadU64(const Record& record, std::size_t offset) {
    if (offset > record.size() || record.size() - offset < 8) {
        throw std::runtime_error("Record ended while reading a string length.");
    }
    std::uint64_t value = 0;
    for (std::size_t index = 0; index < 8; ++index) {
        value |= static_cast<std::uint64_t>(record[offset + index]) << (index * 8);
    }
    return value;
}

std::uint32_t ClassIdFor(const std::string& class_name) {
    static const std::map<std::string, std::uint32_t> class_ids{
        {"Workspace", 0},
        {"Lighting", 1},
        {"Part", 2},
        {"Folder", 4},
        {"LocalScript", 7},
        {"Script", 8},
        {"ModuleScript", 9},
        {"ReplicatedStorage", 10},
        {"StarterPlayerScripts", 11},
        {"ServerScriptService", 12},
        {"RemoteEvent", 13},
        {"RemoteFunction", 15},
        {"StarterGui", 19},
        {"StarterPack", 31},
    };
    const auto found = class_ids.find(class_name);
    if (found == class_ids.end()) {
        throw std::runtime_error("No class ID is registered for '" + class_name + "'.");
    }
    return found->second;
}

void AppendU32(Record& record, std::uint32_t value) {
    for (std::size_t index = 0; index < 4; ++index) {
        record.push_back(static_cast<std::uint8_t>((value >> (index * 8)) & 0xff));
    }
}

void AppendU64(Record& record, std::uint64_t value) {
    for (std::size_t index = 0; index < 8; ++index) {
        record.push_back(static_cast<std::uint8_t>((value >> (index * 8)) & 0xff));
    }
}

Record CreateInstanceRecord(const Instance& instance) {
    Record record;
    AppendU32(record, ClassIdFor(instance.class_name));
    AppendU64(record, instance.name.size());
    record.insert(record.end(), instance.name.begin(), instance.name.end());
    record.push_back(0x01);
    AppendU64(record, 0);
    return record;
}

void WriteU32(Record& record, std::size_t offset, std::uint32_t value) {
    if (offset > record.size() || record.size() - offset < 4) {
        throw std::runtime_error("Record ended while writing a property.");
    }
    for (std::size_t index = 0; index < 4; ++index) {
        record[offset + index] = static_cast<std::uint8_t>((value >> (index * 8)) & 0xff);
    }
}

void WriteU64(Record& record, std::size_t offset, std::uint64_t value) {
    if (offset > record.size() || record.size() - offset < 8) {
        throw std::runtime_error("Record ended while writing a size field.");
    }
    for (std::size_t index = 0; index < 8; ++index) {
        record[offset + index] = static_cast<std::uint8_t>((value >> (index * 8)) & 0xff);
    }
}

void WriteFloat(Record& record, std::size_t offset, float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    WriteU32(record, offset, bits);
}

std::size_t ShiftedOffset(std::size_t offset, std::ptrdiff_t delta) {
    if (delta < 0) {
        const auto magnitude = static_cast<std::size_t>(-delta);
        if (magnitude > offset) {
            throw std::runtime_error("Property offset underflow.");
        }
        return offset - magnitude;
    }
    return offset + static_cast<std::size_t>(delta);
}

Record ReplaceName(const Record& record, std::string_view name) {
    constexpr std::size_t name_offset = 12;
    constexpr std::size_t length_offset = name_offset - 8;
    const std::uint64_t old_length = ReadU64(record, length_offset);
    if (old_length > record.size() - name_offset) {
        throw std::runtime_error("Default record has an invalid instance name length.");
    }

    Record result;
    result.reserve(record.size() - static_cast<std::size_t>(old_length) + name.size());
    result.insert(result.end(), record.begin(), record.begin() + static_cast<std::ptrdiff_t>(length_offset));
    const std::uint64_t new_length = name.size();
    for (std::size_t index = 0; index < 8; ++index) {
        result.push_back(static_cast<std::uint8_t>((new_length >> (index * 8)) & 0xff));
    }
    result.insert(result.end(), name.begin(), name.end());
    result.insert(result.end(), record.begin() + static_cast<std::ptrdiff_t>(name_offset + old_length), record.end());
    return result;
}

Record EncodeScriptLikeInstance(const Instance& instance) {
    std::string source;
    bool disabled = false;
    for (const auto& [name, value] : instance.properties) {
        if (name == "Source") {
            const auto* text = std::get_if<std::string>(&value);
            if (text == nullptr) {
                throw std::runtime_error("Source must be a string.");
            }
            source = *text;
        } else if (name == "Disabled") {
            const auto* enabled = std::get_if<bool>(&value);
            if (enabled != nullptr) {
                disabled = *enabled;
            }
        }
    }

    if (!source.empty() && source.back() != '\n') {
        source.push_back('\n');
    }
    const std::uint64_t source_length = source.size();

    Record record = CreateInstanceRecord(instance);
    record.insert(record.end(), {0x00, 0x00, 0x00, 0x01});
    AppendU64(record, source_length);
    record.insert(record.end(), source.begin(), source.end());
    record.insert(record.end(), {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    if (disabled) {
        record[record.size() - 10] = 0x00;
    }

    return record;
}

Record EncodeGenericInstance(const Instance& instance) {
    if (instance.class_name != "Part") {
        Record record = CreateInstanceRecord(instance);
        record.resize(record.size() + 13, 0);
        return record;
    }

    Record source(std::begin(kPartRecordBytes), std::end(kPartRecordBytes));

    const std::uint64_t old_name_length = ReadU64(source, 4);
    const std::ptrdiff_t length_delta = static_cast<std::ptrdiff_t>(instance.name.size()) -
                                        static_cast<std::ptrdiff_t>(old_name_length);
    Record record = ReplaceName(source, instance.name);
    WriteU32(record, 0, ClassIdFor(instance.class_name));

    for (const auto& [name, value] : instance.properties) {
        if (name == "Position" || name == "Size") {
            const auto* vector = std::get_if<Vector3>(&value);
            if (vector == nullptr) {
                throw std::runtime_error(name + " must be a Vector3.");
            }
            const std::size_t offset = ShiftedOffset(name == "Position" ? 38 : 66, length_delta);
            WriteFloat(record, offset, static_cast<float>(vector->x));
            WriteFloat(record, offset + 4, static_cast<float>(vector->y));
            WriteFloat(record, offset + 8, static_cast<float>(vector->z));
        } else if (name == "Rotation") {
            const auto* rotation = std::get_if<Vector3>(&value);
            if (rotation == nullptr) {
                throw std::runtime_error("Rotation must be a Vector3 of degrees.");
            }
            constexpr double kDegreesToHalfRadians = 3.14159265358979323846 / 360.0;
            const double half_x = rotation->x * kDegreesToHalfRadians;
            const double half_y = rotation->y * kDegreesToHalfRadians;
            const double half_z = rotation->z * kDegreesToHalfRadians;
            const double cx = std::cos(half_x);
            const double sx = std::sin(half_x);
            const double cy = std::cos(half_y);
            const double sy = std::sin(half_y);
            const double cz = std::cos(half_z);
            const double sz = std::sin(half_z);
            const std::array<float, 4> quaternion{
                static_cast<float>(sx * cy * cz - cx * sy * sz),
                static_cast<float>(cx * sy * cz + sx * cy * sz),
                static_cast<float>(cx * cy * sz - sx * sy * cz),
                static_cast<float>(cx * cy * cz + sx * sy * sz),
            };
            const std::size_t offset = ShiftedOffset(50, length_delta);
            for (std::size_t index = 0; index < quaternion.size(); ++index) {
                WriteFloat(record, offset + index * sizeof(float), quaternion[index]);
            }
        } else if (name == "Color") {
            const auto* color = std::get_if<Color3>(&value);
            if (color == nullptr) {
                throw std::runtime_error("Color must be a Color3.");
            }
            const std::size_t offset = ShiftedOffset(78, length_delta);
            WriteFloat(record, offset, static_cast<float>(color->r));
            WriteFloat(record, offset + 4, static_cast<float>(color->g));
            WriteFloat(record, offset + 8, static_cast<float>(color->b));
        } else if (name == "Source" || name == "Disabled") {
            continue;
        } else {
            throw std::runtime_error("The built-in generic codec does not encode property '" + name + "'. "
                                     "Register a class codec for this property.");
        }
    }

    return record;
}

}  // namespace

Scene::InstanceId Scene::AddInstance(std::string class_name, std::string name,
                                     std::optional<std::string> parent) {
    if (class_name.empty() || name.empty()) {
        throw std::invalid_argument("Instance class name and name must not be empty.");
    }
    instances_.push_back(Instance{std::move(class_name), std::move(name), parent, {}});
    return instances_.size() - 1;
}

void Scene::SetProperty(InstanceId instance, std::string property, PropertyValue value) {
    if (instance >= instances_.size()) {
        throw std::out_of_range("Instance id is out of range.");
    }
    if (property.empty()) {
        throw std::invalid_argument("Property name must not be empty.");
    }
    instances_[instance].properties.insert_or_assign(std::move(property), std::move(value));
}

const std::vector<Instance>& Scene::instances() const noexcept {
    return instances_;
}

VrtxEncoder::VrtxEncoder() {
    RegisterCodec("Part", EncodeGenericInstance);
    RegisterCodec("Folder", EncodeGenericInstance);
    RegisterCodec("Script", EncodeScriptLikeInstance);
    RegisterCodec("LocalScript", EncodeScriptLikeInstance);
    RegisterCodec("ModuleScript", EncodeScriptLikeInstance);
    RegisterCodec("RemoteEvent", EncodeScriptLikeInstance);
    RegisterCodec("RemoteFunction", EncodeScriptLikeInstance);
    RegisterCodec("Workspace", EncodeGenericInstance);
    RegisterCodec("Lighting", EncodeGenericInstance);
    RegisterCodec("ReplicatedStorage", EncodeGenericInstance);
    RegisterCodec("ServerScriptService", EncodeGenericInstance);
    RegisterCodec("StarterPlayerScripts", EncodeGenericInstance);
    RegisterCodec("StarterGui", EncodeGenericInstance);
    RegisterCodec("StarterPack", EncodeGenericInstance);
}

void VrtxEncoder::RegisterCodec(std::string class_name, RecordCodec codec) {
    if (class_name.empty() || !codec) {
        throw std::invalid_argument("A codec requires a class name and callback.");
    }
    codecs_.insert_or_assign(std::move(class_name), std::move(codec));
}

std::vector<Record> VrtxEncoder::EncodeRecords(const Scene& scene) const {
    const std::array<std::string, 7> services{
        "Workspace", "Lighting", "ReplicatedStorage", "ServerScriptService",
        "StarterPlayerScripts", "StarterGui", "StarterPack",
    };
    std::map<std::string, std::uint64_t> referents;
    for (std::size_t index = 0; index < services.size(); ++index) {
        referents.emplace(services[index], index);
    }

    std::vector<Record> records;
    records.reserve(scene.instances().size());

    for (std::size_t index = 0; index < scene.instances().size(); ++index) {
        const Instance& instance = scene.instances()[index];
        const auto codec = codecs_.find(instance.class_name);
        if (codec == codecs_.end()) {
            throw std::runtime_error("No record codec registered for class '" + instance.class_name + "'.");
        }
        Record record = codec->second(instance);
        std::uint64_t parent_referent = 0;
        if (instance.parent.has_value()) {
            const auto parent = referents.find(*instance.parent);
            if (parent == referents.end()) {
                throw std::runtime_error("Parent '" + *instance.parent + "' does not exist.");
            }
            parent_referent = parent->second;
        }
        WriteU64(record, 13 + instance.name.size(), parent_referent);
        records.push_back(std::move(record));
        referents.insert_or_assign(instance.name, 7 + index);
    }

    return records;
}

Record VrtxEncoder::EncodeScene(const Scene& scene) const {
    if (scene.instances().size() > std::numeric_limits<std::uint64_t>::max() - 7) {
        throw std::length_error("Scene has too many instances.");
    }

    std::vector<Record> records = EncodeRecords(scene);
    Record payload(std::begin(kServicesBytes), std::end(kServicesBytes));
    const std::uint64_t instance_count = 7 + records.size();
    for (std::size_t index = 0; index < 8; ++index) {
        payload[1 + index] = static_cast<std::uint8_t>((instance_count >> (index * 8)) & 0xff);
    }
    for (const Record& record : records) {
        payload.insert(payload.end(), record.begin(), record.end());
    }
    payload.insert(payload.end(), std::begin(kFooterBytes), std::end(kFooterBytes));

    ZSTD_CCtx* raw_context = ZSTD_createCCtx();
    if (raw_context == nullptr) {
        throw std::runtime_error("Cannot create a Zstandard compressor.");
    }
    const auto context_deleter = [](ZSTD_CCtx* context) { ZSTD_freeCCtx(context); };
    std::unique_ptr<ZSTD_CCtx, decltype(context_deleter)> context(raw_context, context_deleter);
    auto check_zstd = [](std::size_t result, const char* operation) {
        if (ZSTD_isError(result)) {
            throw std::runtime_error(std::string(operation) + ": " + ZSTD_getErrorName(result));
        }
    };
    check_zstd(ZSTD_CCtx_setParameter(context.get(), ZSTD_c_compressionLevel, 3), "Set compression level");
    check_zstd(ZSTD_CCtx_setParameter(context.get(), ZSTD_c_checksumFlag, 0), "Disable Zstandard checksum");
    check_zstd(ZSTD_CCtx_setParameter(context.get(), ZSTD_c_contentSizeFlag, 0), "Disable frame content size");

    Record output(5 + ZSTD_compressBound(payload.size()));
    std::copy(std::begin(kSceneMagic), std::end(kSceneMagic), output.begin());
    output[4] = kSceneVersion;
    const std::size_t compressed_size = ZSTD_compress2(context.get(), output.data() + 5,
                                                       output.size() - 5, payload.data(), payload.size());
    check_zstd(compressed_size, "Compress VRTX payload");
    output.resize(5 + compressed_size);
    return output;
}

}  // namespace seworks::vrtx