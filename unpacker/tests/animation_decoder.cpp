#include "../unpacker/DC2Animations.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <cmath>

void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "Usage: animation_decoder_test E30.decompressed\n"; return 2; }
    std::ifstream input(argv[1], std::ios::binary);
    std::vector<unsigned char> b((std::istreambuf_iterator<char>(input)), {});
    dc2::AnimationOptions options;
    std::string reason;
    const auto clips = dc2::decodeAnimations(b, 0x640000, 20, options, reason);
    check(clips.size() == 11 && reason.empty(), "Expected 11 E30 clips");
    const std::size_t counts[] = {15,46,45,45,67,67,67,67,67,35,22};
    for (std::size_t i = 0; i < 11; ++i) {
        check(clips[i].times.size() == counts[i], "Wrong frame count");
        check(clips[i].rotations.size() == 20, "Wrong joint count");
        for (const auto& track : clips[i].rotations) for (const auto& q : track) {
            float norm = 0;
            for (auto v : q) norm += v*v;
            check(std::abs(norm-1) < 1e-5f, "Nonunit quaternion");
        }
    }
    options.ticksPerSecond = 30;
    const auto slower = dc2::decodeAnimations(b, 0x640000, 20, options, reason);
    check(slower.size() == 11 && std::abs(slower[1].times.back() - 9) < 1e-6, "Tick-rate option failed");
    options.ticksPerSecond = 60;
    for (auto length : {std::size_t(0),std::size_t(27),std::size_t(0x2984),std::size_t(0x3000)}) {
        const std::vector<unsigned char> shortBlock(b.begin(), b.begin() + length);
        check(dc2::decodeAnimations(shortBlock,0x640000,20,options,reason).empty(), "Truncated input accepted");
    }
    // Corrupt a frame's duration: unaffected records must remain exportable.
    auto broken = b;
    broken[0x29b0+22] = broken[0x29b0+23] = 0;
    dc2::AnimationDiagnostics diagnostics;
    auto partial = dc2::decodeAnimations(broken,0x640000,20,options,reason,&diagnostics);
    check(partial.size() == 10, "A bad record discarded unaffected clips");
    check(!reason.empty(), "Partial recovery was not reported");
    options.enabled = false;
    check(dc2::decodeAnimations(b,0x640000,20,options,reason).empty(), "Disabled export ignored");
    std::cout << "Animation decoder checks passed\n";
}
