#include "../include/Tools.h"
#include "../include/Application.h"
#include "../include/myImGui.h"
using namespace CAE;

void CAE::Tools::toolsPanel()
{
	int pos = -47 + 8;
	for (auto tool = tools.begin(); tool != tools.end(); ++tool)
	{
		auto& [f, s] = *tool;
		static tool_type selectedTool = tool_type::MOVEPART;
		ImGui::SetNextWindowBgAlpha(0);
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
		if (ImGui::Begin("Tools", nullptr, window_flags))
		{
			ImGui::SetCursorPosY(pos += 32 + 15);
			//ImGui::ImageButton(s->getIco(), { 32,32 });
			if (SelectableImage(s->getIco(), f == selectedTool, { 32,32 }))
			{
				selectedTool = f;
				current_tool->second->SetActive(false);
				current_tool = tool;
				current_tool->second->SetActive(true);
			}
			//ImGui::Image(s->getIco(), { 32,32 });
			//if (ImGui::IsItemClicked())
			//{
			//	current_tool = tool;
			//}
		}
		ImGui::PopStyleVar();
		ImGui::End();
	}
}

CAE::Tools::Tools(Application* app, sf::RenderWindow* w) : current_tool(tools.end())
{
	tools[tool_type::MOVEPART] = std::make_shared<MovePen>(app->eventManagers, app->ico_holder.getTexture(ico_t::Move), *w, app->useFloat);
	tools[tool_type::MAGICTOOL] = std::make_shared<_MagicTool>(app->eventManagers, app->ico_holder.getTexture(ico_t::Magic), *w, app->useFloat);
	tools[tool_type::MOVEVIEW] = std::make_shared<MoveView>(app->eventManagers, app->ico_holder.getTexture(ico_t::Hand), *w, app->view, app->useMouse);
	current_tool = tools.find(tool_type::MOVEPART);
}

void CAE::Tools::update()
{
	current_tool->second->update();
	/*for (auto& [_, t] : tools)
		t->update();*/
}

void CAE::Tools::draw(sf::RenderWindow& w)
{
	current_tool->second->draw(w);
	toolsPanel();
}
//
//void CAE::Tools::handleEvent(sf::Event& e)
//{
//	if (current_tool != tools.end())
//		current_tool->second->handleEvenet(e);
//	/*for (auto& [_, t] : tools)
//		t->handleEvenet();*/
//}

void CAE::Tools::changeCurrAsset(std::shared_ptr<AnimationAsset> asset)
{
	//data = asset;
	for (auto& tool : tools)
		tool.second->SetAsset(asset);
}
