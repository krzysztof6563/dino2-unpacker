#pragma once

#include <string>
#include "DC2Animations.h"

class DC2ModelExtractor {
public:
    static void setAnimationOptions(const dc2::AnimationOptions& options);
    static void setOutputDirectory(const std::string& directory);
    static void setSaveArchiveEntries(bool enabled);
    static int extractOtherContainer(const std::string& filename);
    static int extract(const std::string& filename);
};
