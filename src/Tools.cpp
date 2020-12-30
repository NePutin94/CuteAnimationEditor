#include "../include/Tools.h"
#include "../include/Application.h"
#include "../include/myImGui.h"

using namespace CAE;

void CAE::Tools::toolsPanel()
{
    int pos = -47 + 8;
    for(auto tool = tools.begin(); tool != tools.end(); ++tool)
    {
        auto&[f, s] = *tool;
        ImGui::SetNextWindowBgAlpha(0);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7,8));
        if(corner != -1)
        {
            auto io = ImGui::GetIO();
            float DISTANCE = 10.f;
            window_flags |= ImGuiWindowFlags_NoMove;
            ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE,
                                       (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
            ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        }
        if(ImGui::Begin("Tools", nullptr, window_flags))
        {
            ImGui::SetCursorPosY(pos += 32 + 15);
            if(ImGui::SelectableImage(s->getIco(), f == selectedTool, {32, 32}))
            {
                selectedTool = f;
                current_tool->second->SetActive(false);
                current_tool = tool;
                current_tool->second->SetActive(true);
            }
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::End();
    }
}

CAE::Tools::Tools(Application* app, sf::RenderWindow* w) : e(app->eventManagers)
{
    tools[tool_type::MOVEPART] = std::make_shared<MovePen>(app->eventManagers, app->ico_holder.getTexture(ico_t::Move), *w, app->useFloat);
    tools[tool_type::MAGICTOOL] = std::make_shared<MagicTool>(app->eventManagers, app->ico_holder.getTexture(ico_t::Magic), *w,
                                                              app->useFloat);
    tools[tool_type::MOVEVIEW] = std::make_shared<MoveView>(app->eventManagers, app->ico_holder.getTexture(ico_t::Hand), *w, app->view,
                                                            app->useMouse);
    tools[tool_type::SELECTION] = std::make_shared<SelectionTool>(app->eventManagers, app->ico_holder.getTexture(ico_t::Selection), *w);
}

void CAE::Tools::update()
{
    current_tool->second->update();
}

void CAE::Tools::draw(sf::RenderWindow& w)
{
    current_tool->second->draw(w);
    toolsPanel();
}

void CAE::Tools::changeCurrAsset(std::shared_ptr<AnimationAsset> asset)
{
    for(auto& tool : tools)
        tool.second->SetAsset(asset);
}
