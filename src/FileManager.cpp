#include "../include/FileManager.h"
#include <filesystem>
#include <imgui.h>
#include <iostream>
#include <imgui-SFML.h>
#include <set>
#include "../include/IcoHolder.h"
#include "../include/myImGui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
namespace fs = std::filesystem;

void CAE::FileManager::open()
{
	if (show)
	{
		ImGui::Begin("File Manager", &show);
		ImVec2 os = ImGui::GetCursorScreenPos();
		ImVec2 origin = ImGui::GetCursorPos();
		//------selected area------//

		int _s = ImGui::GetCursorPosY();
		ImGui::InputText("Path", buffer, 256);
		ImGui::Spacing();
		ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Arrow), sf::Vector2f{ 32,32 }, sf::Color(106, 179, 204)); ImGui::SameLine();
		ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Arrow), sf::Vector2f(32, 32), sf::Vector2f(1, 0), sf::Vector2f(0, 1), sf::Color::Red);
		const int UpPadding = ImGui::GetCursorPosY() - _s;
		float x = 0.f;
		float y = 0.f;
		const int w = 160;
		int h = 115; //115
		const int XPadding = 10;
		const int YPadding = 10;
		h += UpPadding;

		ImGui::BeginChild("File View");
		int i = 0;
		int z = 0;
		fs::path p = buffer;
		if (fs::exists(p))
			for (const auto& fyles : fs::directory_iterator(p))
			{
				ImGui::PushID(++i);
				++z;
				bool out_hovered;
				bool is_regular = fs::is_regular_file(fyles);
				ImVec2 mousePos = ImGui::GetMousePos();
				mousePos.x -= os.x;
				mousePos.y -= os.y;

				out_hovered = (mousePos.x > x && mousePos.x < x + w && mousePos.y > y + UpPadding - ImGui::GetScrollY() && mousePos.y < y + h - ImGui::GetScrollY());

				//---- Selection
				if (selected_nodes.size() <= 1 && selected_nodes.find(i) != selected_nodes.end())
				{
					ImU32 col = ImColor(84, 175, 174);
					x += os.x;
					y += os.y;
					ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true, 4.f);
					x -= os.x;
					y -= os.y;
				}
				else if (selected_nodes.find(i) != selected_nodes.end())
				{
					ImU32 col = ImColor(78, 108, 235);
					x += os.x;
					y += os.y;
					ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true, 4.f);
					x -= os.x;
					y -= os.y;
				}
				if (out_hovered)
				{
					ImU32 col = ImColor(180, 180, 180);
					x += os.x;
					y += os.y;
					ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true, 4.f);
					x -= os.x;
					y -= os.y;
				}

				//---- Image
				ImGui::SetCursorPosX(x + 48);
				ImGui::SetCursorPosY(y + origin.y - 16);
				if (fs::is_regular_file(fyles))
				{
					if (auto ext = fs::path(fyles).extension().string(); ext == ".png" || ext == ".jpg")
						ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::File_img), { 64,64 });
					else
						ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::File_txt), { 64,64 });
				}
				else
					ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Folder), { 64,64 });

				//---- Text
				auto str = fs::path(fyles).filename().string();
				if (str.length() > 16) {
					str.erase(16);
					str += "...";
				}
				int t_w = ImGui::CalcTextSize(str.c_str()).x;
				ImGui::SetCursorPosX(x + (w / 2 - t_w / 2));
				ImGui::SetCursorPosY(y + origin.y + 60);
				ImGui::Text(str.c_str());

				//---- Events
				if (fs::is_regular_file(fyles))
				{
					if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
					{
						selected_nodes.clear();
					}
					if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
					{
						show = false;
						ImGui::PopID();
						ImGui::EndChild();
						ImGui::End();
						int _i = 0;
						std::string str;
						for (const auto& fyles : fs::directory_iterator(p))
							if (selected_nodes.find(++_i) != selected_nodes.end())
								str += fs::path(fyles).filename().string() + "; ";
						selected_nodes.clear();
					}
					if (out_hovered && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyCtrl)
					{
						if (!selected_nodes.emplace(i).second)
							selected_nodes.erase(i);
					}
					else if (out_hovered && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyShift)
					{
						if (selected_nodes.size() > 1)
							selected_nodes.clear();
						else
						{
							int start = std::min(*selected_nodes.begin(), i);
							int end = std::max(*selected_nodes.begin(), i) + 1;
							for (int _i = start; _i != end; ++_i)
								selected_nodes.emplace(_i);
						}
					}
					else if (out_hovered && ImGui::IsMouseClicked(0))
					{
						selected_nodes.clear();
						selected_nodes.emplace(i);
					}

				}
				else if (!is_regular && ImGui::IsMouseDoubleClicked(0) && out_hovered)
				{
					strcpy_s(buffer, fs::path(fyles).string().c_str());
				}

				//---- Next position
				x += w + XPadding;
				if (x + w > ImGui::GetWindowSize().x)
				{
					x = 0;
					y += h - UpPadding + YPadding;
				}
				ImGui::PopID();
			}
		ImGui::EndChild();
		ImGui::End();
	}
}

std::pair<bool, std::string> CAE::FileManager::operator()(bool o)
{
	if (show)
	{
		ImGui::Begin("File Manager", &show);
		ImVec2 os = ImGui::GetCursorScreenPos();
		ImVec2 origin = ImGui::GetCursorPos();
		//------selected area------//

		int _s = ImGui::GetCursorPosY();
		ImGui::InputText("Path", buffer, 256);
		ImGui::Spacing();
		ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Arrow), sf::Vector2f{ 32,32 }, sf::Color(106, 179, 204)); ImGui::SameLine();
		ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Arrow), sf::Vector2f(32, 32), sf::Vector2f(1, 0), sf::Vector2f(0, 1), sf::Color::Red);
		const int UpPadding = ImGui::GetCursorPosY() - _s;
		float x = 0.f;
		float y = 0.f;
		const int w = 160;
		int h = 115; //115
		const int XPadding = 10;
		const int YPadding = 10;
		h += UpPadding;

		ImGui::BeginChild("File View");
		int i = 0;
		int z = 0;
		fs::path p = buffer;
		if (fs::exists(p))
			for (const auto& fyles : fs::directory_iterator(p))
			{
				ImGui::PushID(++i);
				++z;
				bool out_hovered;
				bool is_regular = fs::is_regular_file(fyles);
				ImVec2 mousePos = ImGui::GetMousePos();
				mousePos.x -= os.x;
				mousePos.y -= os.y;

				out_hovered = (mousePos.x > x && mousePos.x < x + w && mousePos.y > y + UpPadding - ImGui::GetScrollY() && mousePos.y < y + h - ImGui::GetScrollY());

				//---- Selection
				if (selected_nodes.size() <= 1 && selected_nodes.find(i) != selected_nodes.end())
				{
					ImU32 col = ImColor(84, 175, 174);
					x += os.x;
					y += os.y;
					ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true, 4.f);
					x -= os.x;
					y -= os.y;
				}
				else if (selected_nodes.find(i) != selected_nodes.end())
				{
					ImU32 col = ImColor(78, 108, 235);
					x += os.x;
					y += os.y;
					ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true, 4.f);
					x -= os.x;
					y -= os.y;
				}
				if (out_hovered)
				{
					ImU32 col = ImColor(180, 180, 180);
					x += os.x;
					y += os.y;
					ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true, 4.f);
					x -= os.x;
					y -= os.y;
				}

				//---- Image
				ImGui::SetCursorPosX(x + 48);
				ImGui::SetCursorPosY(y + origin.y - 16);
				if (fs::is_regular_file(fyles))
				{
					if (auto ext = fs::path(fyles).extension().string(); ext == ".png" || ext == ".jpg")
						ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::File_img), { 64,64 });
					else
						ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::File_txt), { 64,64 });
				}
				else
					ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Folder), { 64,64 });

				//---- Text
				auto str = fs::path(fyles).filename().string();
				if (str.length() > 16) {
					str.erase(16);
					str += "...";
				}
				int t_w = ImGui::CalcTextSize(str.c_str()).x;
				ImGui::SetCursorPosX(x + (w / 2 - t_w / 2));
				ImGui::SetCursorPosY(y + origin.y + 60);
				ImGui::Text(str.c_str());

				//---- Events
				if (fs::is_regular_file(fyles))
				{
					if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
					{
						selected_nodes.clear();
					}
					if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
					{
						show = false;
						ImGui::PopID();
						ImGui::EndChild();
						ImGui::End();
						int _i = 0;
						std::string str;
						for (const auto& fyles : fs::directory_iterator(p))
							if (selected_nodes.find(++_i) != selected_nodes.end())
								str += fs::path(fyles).filename().string() + "; ";
						selected_nodes.clear();
						return { true, str };
					}
					if (out_hovered && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyCtrl)
					{
						if (!selected_nodes.emplace(i).second)
							selected_nodes.erase(i);
					}
					else if (out_hovered && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyShift)
					{
						if (selected_nodes.size() > 1)
							selected_nodes.clear();
						else
						{
							int start = std::min(*selected_nodes.begin(), i);
							int end = std::max(*selected_nodes.begin(), i) + 1;
							for (int _i = start; _i != end; ++_i)
								selected_nodes.emplace(_i);
						}
					}
					else if (out_hovered && ImGui::IsMouseClicked(0))
					{
						selected_nodes.clear();
						selected_nodes.emplace(i);
					}

				}
				else if (!is_regular && ImGui::IsMouseDoubleClicked(0) && out_hovered)
				{
					strcpy_s(buffer, fs::path(fyles).string().c_str());
				}

				//---- Next position
				x += w + XPadding;
				if (x + w > ImGui::GetWindowSize().x)
				{
					x = 0;
					y += h - UpPadding + YPadding;
				}
				ImGui::PopID();
			}
		ImGui::EndChild();
		ImGui::End();
		return { false, "" };
	}
	return { false, "" };
}
