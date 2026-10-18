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

    // setup
    const bool    debuggerAttached = AntiDbg::g_hChildState == ChildState::ALIVE;
    const ImVec4  debugColor       = debuggerAttached ? Colors::green : Colors::red;

    const bool    hasDets     = Detections::g_detectionCount > 0;
    const ImVec4  detColor    = hasDets ? Colors::red : Colors::green;

    
    std::string  debugged    = "Child Debugger";
    std::string  numOfDets   = "Number of Detections";
    std::string  debugTag    = "[" + AntiDbg::GetDebuggerStatus() + "]";
    std::string  detTag      = "[" + std::to_string(Detections::g_detectionCount) + "]";


    // display here
    ImGui::TextColored(debugColor, "%s", debugTag.c_str());
    ImGui::SameLine();
    ImGui::TextColored(Colors::white, "%s", debugged.c_str());

    ImGui::TextColored(detColor, "%s", detTag.c_str());
    ImGui::SameLine();
    ImGui::TextColored(Colors::white, "%s", numOfDets.c_str());

    ImGui::TextColored(Colors::white, "%s", banter.c_str());

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
