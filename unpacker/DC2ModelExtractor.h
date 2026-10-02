#pragma once

#include <string>
#include "DC2Animations.h"

class DC2ModelExtractor {
public:
    static int extract(const std::string& filename);
    static void setAnimationOptions(const dc2::AnimationOptions& options);
    // Also keep each model block's compressed and decompressed bytes.
    static void setSaveArchiveEntries(bool enabled);
    // A second folder to look in for companion files (e.g. WP00A.DAT for WEP_P000.DAT).
    static void setSourceDirectory(const std::string& directory);
};
