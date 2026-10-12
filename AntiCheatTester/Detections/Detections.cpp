#include "Detections.h"


void Detections::PopulateFeatures()
{
    Feature f1{ };
    Feature f2{ "","",true };

    g_features.emplace_back(f1);
    g_features.emplace_back(f2);
}