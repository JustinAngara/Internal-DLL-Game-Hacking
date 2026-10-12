#pragma once
#include "Features.h"

// intended to be global, usable anywhere, and inline/static, whatever
// setup this, move features away from this
namespace Detections
{
    inline std::vector<Feature> g_features{};
    void PopulateFeatures();
	
}