#include "DC2Animations.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace dc2 {
namespace {
std::uint16_t word(const std::vector<unsigned char>& b, std::size_t o) {
    return b[o] | (std::uint16_t(b[o + 1]) << 8);
}
std::uint32_t dword(const std::vector<unsigned char>& b, std::size_t o) {
    return word(b, o) | (std::uint32_t(word(b, o + 2)) << 16);
}
std::int16_t signedWord(const std::vector<unsigned char>& b, std::size_t o) {
    const auto v = word(b, o);
    return static_cast<std::int16_t>(v >= 32768 ? int(v) - 65536 : int(v));
}
std::array<float, 4> quaternion(const std::vector<unsigned char>& b, std::size_t o) {
    constexpr double halfAngle = 3.14159265358979323846 / 4096.0;
    const double x = signedWord(b, o) * halfAngle;
    const double y = -signedWord(b, o + 2) * halfAngle;
    const double z = -signedWord(b, o + 4) * halfAngle;
    const double sx = std::sin(x), cx = std::cos(x), sy = std::sin(y), cy = std::cos(y);
    const double sz = std::sin(z), cz = std::cos(z);
    return {float(sx*cy*cz - cx*sy*sz), float(cx*sy*cz + sx*cy*sz),
            float(cx*cy*sz - sx*sy*cz), float(cx*cy*cz + sx*sy*sz)};
}


std::vector<std::size_t> recordStrides(std::size_t joints) {
    const auto raw = 16 + joints * 6;
    std::vector<std::size_t> values{(raw + 3) & ~std::size_t(3)};
    if (raw != values.front()) values.push_back(raw);
    // Other actors' records can share an archive. Recognize their size generically
    // to keep them from preventing export of motions belonging to this model.
    for (std::size_t n = 1; n <= 256; ++n) {
        for (auto v : {16 + n * 6, (16 + n * 6 + 3) & ~std::size_t(3)})
            if (std::find(values.begin(), values.end(), v) == values.end()) values.push_back(v);
    }
    return values;
}
std::size_t validateRecord(const std::vector<unsigned char>& b, std::uint32_t base,
    std::size_t start, std::size_t limit, const std::vector<std::size_t>& strides) {
    if (start > limit || limit > b.size() || limit - start < 20) return 0;
    const std::size_t frames = dword(b, start);
    if (!frames || frames > 65536) return 0;
    for (auto stride : strides) {
        if (frames > (limit - start - 20) / stride) continue;
        const auto end = start + 20 + frames * stride;
        bool valid = true;
        for (int k = 0; k < 4; ++k) {
            const auto pointer = dword(b, start + 4 + k * 4);
            if (pointer && (pointer < base || pointer - base < end || pointer - base > limit)) valid = false;
            if (!k && pointer && pointer - base != end) valid = false;
        }
        if (!valid) continue;
        std::uint32_t previous = 0, duration = 0;
        for (std::size_t fi = 0; fi < frames; ++fi) {
            const auto frame = start + 20 + fi * stride;
            const auto time = word(b, frame), length = word(b, frame + 2);
            if ((!fi && time != 0) || !length || (fi && time != previous + duration)) { valid = false; break; }
            previous = time; duration = length;
        }
        if (valid) return stride;
    }
    return 0;
}
AnimationClip decodeRecord(const std::vector<unsigned char>& b, std::size_t start,
    std::size_t stride, std::size_t index, std::size_t joints, const AnimationOptions& options) {
    AnimationClip clip;
    clip.sourceIndex = index;
    clip.sourceOffset = start;
    clip.rotations.resize(joints);
    const std::size_t frames = dword(b, start);
    for (std::size_t fi = 0; fi < frames; ++fi) {
        const auto frame = start + 20 + fi * stride;
        clip.times.push_back(float(word(b, frame) / options.ticksPerSecond));
        clip.rootPositions.push_back({signedWord(b, frame + 4) * .001f,
            -signedWord(b, frame + 6) * .001f, -signedWord(b, frame + 8) * .001f});
        // Movement through the world is horizontal: the game keeps characters on the floor,
        // and adding the stored vertical value would sink a leaping E10 two metres underground.
        clip.rootOffsets.push_back({signedWord(b, frame + 10) * .001f, 0.0f,
            -signedWord(b, frame + 14) * .001f});
        for (std::size_t j = 0; j < joints; ++j) {
            auto q = quaternion(b, frame + 16 + 6 * j);
            auto& track = clip.rotations[j];
            if (!track.empty()) {
                double dot = 0;
                for (int k = 0; k < 4; ++k) dot += q[k] * track.back()[k];
                if (dot < 0) for (auto& v : q) v = -v;
            }
            track.push_back(q);
        }
    }
    return clip;
}
bool compatible(std::size_t stride, std::size_t joints) {
    const auto raw = 16 + joints * 6;
    return stride == raw || stride == ((raw + 3) & ~std::size_t(3));
}
bool decodeTable(const std::vector<unsigned char>& b, std::uint32_t base,
    std::size_t table, std::size_t joints, const std::vector<std::size_t>& strides,
    const AnimationOptions& options, std::vector<AnimationClip>& clips, AnimationDiagnostics& diagnostics) {
    if (table > b.size() || b.size() - table < 4) return false;
    const auto address = dword(b, table);
    if (address < base) return false;
    const std::size_t first = address - base;
    if (first <= table || first >= b.size() || (first - table) % 4) return false;
    const std::size_t count = (first - table) / 4;
    if (count > 4096) return false;
    std::vector<std::size_t> starts;
    for (std::size_t i = 0; i < count; ++i) {
        const auto pointer = dword(b, table + i * 4);
        if (pointer < base || pointer - base < first || pointer - base >= b.size()) return false;
        starts.push_back(pointer - base);
    }
    auto unique = starts;
    std::sort(unique.begin(), unique.end());
    unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
    std::size_t recognized = 0;
    for (std::size_t ci = 0; ci < count; ++ci) {
        const auto start = starts[ci];
        const auto next = std::upper_bound(unique.begin(), unique.end(), start);
        const auto limit = next == unique.end() ? b.size() : *next;
        MotionRecord record;
        record.index = ci; record.offset = start;
        if (limit - start < 20) record.status = "malformed";
        else {
            record.frames = dword(b, start);
            if (!record.frames && std::all_of(b.begin() + start, b.begin() + limit,
                    [](unsigned char v) { return v == 0; })) {
                record.status = "empty"; ++recognized;
            } else {
                record.stride = validateRecord(b, base, start, limit, strides);
                if (!record.stride) record.status = "unrecognized_or_malformed";
                else {
                    ++recognized;
                    if (compatible(record.stride, joints)) {
                        record.status = "exported";
                        clips.push_back(decodeRecord(b, start, record.stride, ci, joints, options));
                    } else record.status = "different_rig_record";
                }
            }
        }
        diagnostics.records.push_back(record);
    }
    // A pointer-shaped graphics structure alone is not evidence of a motion table.
    return recognized && recognized * 2 >= count;
}
}

std::vector<AnimationClip> decodeAnimations(const std::vector<unsigned char>& b,
    std::uint32_t base, std::size_t joints, const AnimationOptions& options, std::string& reason,
    AnimationDiagnostics* diagnostics) {
    reason.clear();
    AnimationDiagnostics local;
    auto& report = diagnostics ? *diagnostics : local;
    report = {};
    if (!options.enabled) { report.layout = "disabled"; return {}; }
    if (!std::isfinite(options.ticksPerSecond) || options.ticksPerSecond <= 0 ||
        options.ticksPerSecond > 1000000 || b.size() < 28 || !joints || joints > 256) {
        reason = "Invalid animation options or model header"; return {};
    }
    const auto qa = dword(b, 12);
    if (qa < base) { reason = "Invalid quad-table pointer"; return {}; }
    const std::size_t q = qa - base;
    const auto quads = word(b, 18);
    if (q > b.size() || quads > (b.size() - q) / 16) {
        reason = "Quad table extends beyond model block"; return {};
    }
    const std::size_t meshEnd = q + quads * 16;
    const auto strides = recordStrides(joints);
    std::vector<AnimationClip> result;
    for (std::size_t table = (meshEnd + 3) & ~std::size_t(3); table + 4 <= b.size(); table += 4) {
        AnimationDiagnostics candidate;
        std::vector<AnimationClip> clips;
        if (decodeTable(b, base, table, joints, strides, options, clips, candidate)) {
            report = std::move(candidate);
            report.layout = "pointer_table";
            result = std::move(clips);
            break;
        }
    }
    if (report.layout == "none") {
        // Scene/CORE archives carry independent sequence headers without a pointer
        // table. Scan for fully validated timelines, then advance past each payload.
        // At least two frames are required for an unreferenced candidate.
        std::size_t index = 0;
        for (std::size_t start = (meshEnd + 3) & ~std::size_t(3); start + 20 <= b.size(); start += 4) {
            const auto frames = dword(b, start);
            if (frames < 2 || frames > 65536) continue;
            const auto stride = validateRecord(b, base, start, b.size(), strides);
            if (!stride) continue;
            MotionRecord record;
            record.index = index++; record.offset = start; record.frames = frames; record.stride = stride;
            record.status = compatible(stride, joints) ? "exported" : "different_rig_record";
            if (record.status == "exported") result.push_back(decodeRecord(b, start, stride, record.index, joints, options));
            report.records.push_back(record);
            start = ((start + 20 + frames * stride + 3) & ~std::size_t(3)) - 4;
        }
        if (!report.records.empty()) report.layout = "sequential_records";
    }
    std::size_t skipped = 0, empty = 0;
    for (const auto& r : report.records) {
        if (r.status == "empty") ++empty;
        else if (r.status != "exported") ++skipped;
    }
    if (skipped) reason = "Exported " + std::to_string(result.size()) + "; left " +
        std::to_string(skipped) + " different-rig or unrecognized records unbound (see animation report)";
    else if (empty) reason = "Ignored " + std::to_string(empty) + " empty motion slots";
    if (result.empty() && report.records.empty()) reason = "No validated motion records in this model block";
    return result;
}
}
