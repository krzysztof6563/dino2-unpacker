#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace dc2 {
struct AnimationOptions {
    bool enabled = true;
    bool inPlace = false;
    double ticksPerSecond = 60.0;
};
struct AnimationClip {
    std::size_t sourceIndex = 0;
    std::size_t sourceOffset = 0;
    std::vector<float> times;
    std::vector<std::vector<std::array<float, 4>>> rotations; // joint, key, XYZW
    std::vector<std::array<float, 3>> rootPositions;          // first root's position per key
    std::vector<std::array<float, 3>> rootOffsets;            // cumulative world movement per key
};
struct MotionRecord {
    std::size_t index = 0, offset = 0, frames = 0, stride = 0;
    std::string status;
};
struct AnimationDiagnostics {
    std::string layout = "none";
    std::vector<MotionRecord> records;
};
// Each record is validated independently; records for a different rig are reported,
// never assigned to the current skeleton by truncating or guessing bone indices.
std::vector<AnimationClip> decodeAnimations(const std::vector<unsigned char>& block,
    std::uint32_t base, std::size_t jointCount, const AnimationOptions& options,
    std::string& reason, AnimationDiagnostics* diagnostics = nullptr);
}
