#include "GUI.h"
#include "../AntiDbg/AntiDbg.h"
#include <iostream>
/*
    # of detections

    is being debugged
    proc list
    kernel driver list
    ntdll detections
*/ 
void GUI::Display::DrawStats()
{

    const ImVec4 statusColor = Colors::blue;  
    std::string  debugged    = "Attached Debugger Status: " + AntiDbg::GetDebuggerStatus();
    std::string  numOfDets   = "Number of Detections:" + Detections::g_detectionCount;
    std::cout << debugged << '\n';
    
    ImGui::TextColored(statusColor, "%s", debugged.c_str());
    ImGui::TextColored(statusColor, "%s", numOfDets.c_str());

    ImGui::TextColored(statusColor, "%s", banter.c_str());

}

void GUI::Display::DrawFeature(Feature e)
{
    const bool detected = e.getFlag();


    const ImVec4 statusColor = detected ? Colors::red : Colors::green;  
    const std::string label = e.getName() + " (" + e.getDesc() + ")";

    ImGui::TextColored(statusColor, "%s", detected ? "[DETECTED]" : "[UNDETECTED]");
    ImGui::SameLine();
    ImGui::TextUnformatted(label.c_str());
}


void GUI::Display::DrawAllFeatures()
{

    for (auto& e : Detections::g_features)
    {
        DrawFeature(e); // add certain flags to indicate whether or not we want to add sliders, additional descs, etc
    }

}

void GUI::Display::DrawUI()
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);

    ImGui::Begin("Main", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus                    
    );

    DrawStats();
    DrawAllFeatures();


    ImGui::End();
}
