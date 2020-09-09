#include "Application.h"
#include <imgui/include/imgui-SFML.h>
#include <iomanip>
#include <algorithm>
#include <future>
#include <filesystem>
//#define DEBUG
#if defined (__linux__)
#define Linux
#endif
namespace fs = std::filesystem;

bool operator<(sf::Vector2i f, sf::Vector2i s)
{
	return (f.x < s.x) && (f.y < s.y);
}

bool operator>(sf::Vector2i f, sf::Vector2i s)
{
	return !(f < s);
}

//template<class T>
//sf::Rect<T> round(sf::Rect<T> value)
//{
//	return sf::Rect <T>{round(value.left), round(value.top), round(value.width), round(value.height)};
//}

template<class T>
sf::Vector2<T> abs(sf::Vector2<T> value)
{
	return sf::Vector2<T>{std::abs(value.x), std::abs(value.y)};
}

void CAE::Application::handleEvent(sf::Event& event)
{
	static bool moving = false;
	static sf::Vector2f oldPos;
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			state = states::Exit;
			break;
		}
		//eManager.updateEvent(event);
		eventManagers.updateEvent(event, *window);
		ImGui::SFML::ProcessEvent(event);
	}
	eventManagers.updatecurr(*window, view); //it must be performed before all updates for the current frame
	eventManagers.updateInput();
}

void CAE::Application::draw()
{
	sf::View unScale(view);
	unScale.setSize(unscaleView);
	window->setView(view);
	window->clear();
	window->draw(shape);
	if (currAsset != nullptr)
	{
		//window->setView(view);
		window->draw(*currAsset);
		if (creatorMode)
		{
			for (auto elem : *currAsset)
			{
				if (elem->isVisible())
					for (auto part : *elem)
					{
						window->draw(part->getVertex());
						//window->setView(unScale);
						for (auto& node : part->getNode())
							window->draw(node);
						//window->setView(view);
					}
			}
			//window->draw(sf::Sprite(magicTool.t.getTexture()));
		}
		tools_container.draw(*window);
	}

	Console::AppLog::Draw("LogConsole", &LogConsole);

	if (attTimer.getElapsedTime() < sf::seconds(2) && newMessage)
		showLog(Console::AppLog::lastLog());
	else newMessage = false;

	ImGui::SFML::Render(*window);
	window->display();
}

void CAE::Application::update()
{
	if (attTimer.getElapsedTime() > sf::seconds(2))
	{
		if (Console::AppLog::hasNewLogByTyp(Console::message))
		{
			newMessage = true;
			attTimer.restart();
		}
	}

	if (currAsset != nullptr && creatorMode)
		tools_container.update();
	//editorUpdate();
}

void CAE::Application::editorUpdate()
{
	//for (auto group : *currAsset)
	//{
	//	if (group->isVisible())
	//	{
	//		for (std::shared_ptr<Part> elem : *group)
	//		{
	//			////////////////////////////////////MOVING(FLOAT OFFSET)//////////////////////////////////////////
	//			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
	//			{
	//				if (auto rect = elem->getRect(); elem->getRect().contains(eManager.currMousePos()) && ((selectedPart != nullptr) ? selectedPart == elem : true))
	//				{
	//					selectedPart = elem;
	//					selectedGroup = group;
	//					if (useFloat)
	//					{
	//						auto delta = eManager.worldMouseDelta();
	//						rect.left -= delta.x;
	//						rect.top -= delta.y;
	//						elem->setRect(rect);
	//					}
	//					else
	//					{
	//						static sf::Vector2f delta2;
	//						delta2 += eManager.worldMouseDelta();
	//						auto delta = -(sf::Vector2i)delta2;
	//						int factor = 1;
	//						if (abs(delta.x) > factor || abs(delta.y) > factor)
	//						{
	//							if (abs(delta.x) > factor)
	//								rect.left = round(rect.left) + delta.x;
	//							if (abs(delta.y) > factor)
	//								rect.top = round(rect.top) + delta.y;
	//							delta2 = { 0,0 };
	//						}
	//						elem->setRect(rect);
	//					}

	//					selectedPart->changeColor(sf::Color::Green);
	//					//lasSelectedPart may no longer be in the array
	//					//But resource will not be deleted, in this case we will just change the color 
	//					//for the essentially 'dead' object(which is still valid)
	//					//it will be released when the lastSelected pointer changes to selectetPart
	//					//If it was not deleted, we will still just change the color
	//					if (lastSelected != nullptr && lastSelected != selectedPart)
	//					{
	//						lastSelected->changeColor(sf::Color::Red);
	//						//selectedGroup.reset();
	//					}
	//					lastSelected = selectedPart;
	//				}
	//			}
	//			else
	//			{
	//				if (selectedPart != nullptr)
	//					lastSelected = selectedPart;
	//				selectedPart.reset();
	//			}

	//			////////////////////////////////////SCALE//////////////////////////////////////////
	//			static int selectedPoint = -1;
	//			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
	//			{
	//				for (auto& p : elem->getNode())
	//					if ((p.getGlobalBounds().contains(eManager.currMousePos()) /*&& !pointSelected*/) || (/*pointSelected &&*/ selectedPoint == p.side && &p == selectedNode))
	//					{
	//						//pointSelected = true;
	//						selectedPoint = p.side;
	//						selectedNode = &p;
	//						auto rect = elem->getRect();
	//						sf::Vector2f value = { 0,0 };
	//						if (!useFloat)
	//						{

	//							static sf::Vector2f delta2;
	//							delta2 += eManager.worldMouseDelta();
	//							sf::Vector2i delta = -(sf::Vector2i)delta2;
	//							int factor = 0.2;
	//							if (abs(delta.x) > factor || abs(delta.y) > factor)
	//							{
	//								value.x += ceil(delta.x);
	//								value.y += ceil(delta.y);
	//								delta2 = { 0,0 };
	//							}
	//							rect = round(rect);
	//						}
	//						else
	//							value = -eManager.worldMouseDelta();

	//						switch (p.side)
	//						{
	//						case 0:
	//							elem->setRect(sf::FloatRect(rect.left, rect.top + value.y, rect.width, rect.height - value.y));
	//							break;
	//						case 1:
	//							elem->setRect(sf::FloatRect(rect.left, rect.top, rect.width + value.x, rect.height));
	//							break;
	//						case 2:
	//							elem->setRect(sf::FloatRect(rect.left, rect.top, rect.width, rect.height + value.y));
	//							break;
	//						case 3:
	//							elem->setRect(sf::FloatRect(rect.left + value.x, rect.top, rect.width - value.x, rect.height));
	//							break;
	//						}
	//					}
	//			}
	//		}
	//	}
	//}
}

void CAE::Application::loadAssets()
{
	ImGui::BeginChild("Load assets");
	ImGui::Spacing();
	ImGui::Text("Available assets: ");

	ImGui::Image(ico_holder.getTexture(ico_t::Refresh), { 32,32 });
	static std::vector<std::string> _files;
	constexpr std::string_view default_path = "D:\\!VisualStudioProject\\CuteAnimationEditor\\CuteAnimationEditor\\assets\\";
	if (ImGui::IsItemClicked())
	{
		_files.clear();
		if (fs::exists(default_path))
		{
			for (auto& p : fs::directory_iterator(default_path))
			{
				auto name = p.path().filename().string();
				if (p.path().extension() == ".json")
					_files.emplace_back(name);
			}
		}
		else
			Console::AppLog::addLog("directory .\"assets\" not exist", Console::error);
	}
	{
		static int selected = -1;
		int i = 0;
		for (auto& name : _files)
		{
			if (ImGui::Selectable(name.c_str()))
				selected = i;
			if (selected == i) {
				ImGui::BeginChild("Note");
				ImGui::OpenPopup("Save?");
				if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text((std::string{ "Load " } + name).c_str());
					ImGui::Separator();
					if (ImGui::Button("Yes", ImVec2(140, 0)))
					{
						loadAsset(std::string{ default_path } + name, true);
						selected = -1;
					}
					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(140, 0))) selected = -1;
					ImGui::EndPopup();
				}
				ImGui::EndChild();
			}
			++i;
		}
	}
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text("Load from other path: ");
	if (ImGui::Button("Load From"))
	{
#ifdef Windows
		char filename[MAX_PATH];
		OPENFILENAME ofn;
		ZeroMemory(&filename, sizeof(filename));
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = ("*json");
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = ("Select a File");
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&ofn))
			loadAsset(filename, true);
#endif
	}
	ImGui::EndChild();
}

void CAE::Application::tapWindow()
{
	auto io = ImGui::GetIO();
	ImVec2 window_pos = ImVec2(io.DisplaySize.x - 1.f - io.DisplaySize.x / 3, io.DisplaySize.y - 1 - window->getSize().y / 3);
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 3, io.DisplaySize.y / 3));
	ImGuiWindowFlags window_flags = 0;

	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	if (!ImGui::Begin("TAP window", NULL, window_flags))
	{
		window_pos.y = io.DisplaySize.y - 21;
		ImGui::SetWindowPos(window_pos);
		ImGui::End();
	}
	else
	{
		ImGui::SetWindowPos(window_pos, ImGuiCond_Always);
		ImGui::BeginChild("Select Animation", ImVec2(150, 0), true);
		for (auto& anim : animPlayer)
			if (ImGui::Selectable(anim.name.c_str()))
				animPlayer.setCurrentAnim(anim);
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true); // Leave room for 1 line below us
		ImGui::Text("MyObject:");
		ImGui::Separator();
		static bool antime = true;
		if (animPlayer.hasAnimation())
		{
			if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
			{
				auto curr = animPlayer.getCurrentAnimation();
				if (ImGui::BeginTabItem("Show Animation"))
				{
					sf::FloatRect rect = animPlayer.animUpdate((antime == true) ? 1 : 0);
					float d = 150 / rect.height;
					auto size = sf::Vector2f(rect.width, rect.height);
					size.x *= d;
					size.y *= d;
					ImGui::Text("Size: %.2f, %.2f", size.x, size.y);
					ImGui::Image(*currAsset->getTexture(), size, rect);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Settings"))
				{
					ImGui::DragFloat("Change animation speed", &curr->speed, 0.0002, -10, 10);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Info"))
				{
					ImGui::Text("Number of frames: %f", curr->frameCount);
					ImGui::Text("Animation name: %s", curr->name.c_str());
					if (ImGui::TreeNode("frames"))
					{
						int id = 1;
						for (auto& frame : curr->frames)
							ImGui::Text("#%d: left: %.2f top: %.2f w: %.2f h: %.2f", id, frame.left, frame.top, frame.width, frame.height);
						ImGui::TreePop();
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		else
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "the animation is not selected");
		ImGui::EndChild();
		if (ImGui::Button("Stop")) antime = false;
		ImGui::SameLine();
		if (ImGui::Button("Play")) antime = true;
		ImGui::SameLine();
		if (ImGui::Button("Parse Asset"))
			animPlayer.parseAnimationAssets(currAsset->groups);
		ImGui::EndGroup();
	}
	ImGui::End();
}

void CAE::Application::viewSettings()
{
	ImGui::BeginChild("Settings");
	if (ImGui::TreeNode("View Settings: "))
	{
		ImGui::Text("View Size: ");
		static float xSize = view.getSize().x;
		static float ySize = view.getSize().y;
		ImGui::DragFloat("Size X: ", &xSize, 1.f);
		ImGui::DragFloat("Size Y:", &ySize, 1.f);
		if (currAsset != nullptr)
			if (ImGui::Button("Set size as the texture"))
			{
				xSize = currAsset->getTexture()->getSize().x;
				ySize = currAsset->getTexture()->getSize().y;
				view.setSize(xSize, ySize);
			}
		if (ImGui::Button("Update Size"))
			view.setSize(xSize, ySize);
		ImGui::Separator();
		ImGui::Text("View Center: ");
		static float x = view.getCenter().x;
		static float y = view.getCenter().x;
		ImGui::InputFloat("position X", &x);
		ImGui::InputFloat("position Y", &y);
		if (ImGui::Button("Update Position"))
			view.setCenter(x, y);
		ImGui::Separator();
		ImGui::Checkbox("use the mouse to move", &useMouse);
		ImGui::TreePop();
	}
	ImGui::EndChild();
}

void CAE::Application::createAssets()
{
	ImGui::BeginChild("Create asset");
	ImGui::Text("Create asset: ");
	ImGui::Separator();
	ImGui::Spacing();

	//static char buff[MAX_PATH];
	ImGui::Text("Name");
	ImGui::InputText("##Name", buff, 256);
	ImGui::Spacing();

	ImGui::Text("Texture path");
	ImGui::InputText("##Texture path", buff2, IM_ARRAYSIZE(buff2));
	ImGui::SameLine();
	ImGui::Image(ico_holder.getTexture(ico_t::Folder), sf::Vector2f{ 27,18 });
	if (ImGui::IsItemClicked())
	{
		//char filename[MAX_PATH];
		/*CFileDialog dlgOpenFile(TRUE);
		dlgOpenFile.GetOFN().Flags |= OFN_ALLOWMULTISELECT;*/
#ifdef Windows
		OPENFILENAME ofn;
		ZeroMemory(&buff2, sizeof(buff2));
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;
		ofn.lpstrFilter = (".json");
		ofn.lpstrFile = buff2;
		ofn.nMaxFile = MAX_PATH;
		if (GetOpenFileName(&ofn))
		{
			char* str = ofn.lpstrFile;
			std::string directory = str;
			str += (directory.length() + 1);
			while (*str) {
				std::string filename = str;
				str += (filename.length() + 1);
				// use the filename, e.g. add it to a vector
			}
		}
#endif
		//buff
	}
	ImGui::Spacing();

	ImGui::Text("Output file");
	ImGui::InputText("##Output file", buff3, IM_ARRAYSIZE(buff3));
	ImGui::SameLine();
	ImGui::Image(ico_holder.getTexture(ico_t::Folder), sf::Vector2f{ 27,18 });
	if (ImGui::IsItemClicked())
	{
	    #ifdef Windows
		//char filename[MAX_PATH];
		OPENFILENAME ofn;
		ZeroMemory(&buff3, sizeof(buff3));
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = (".json");
		ofn.lpstrFile = buff3;
		ofn.nMaxFile = MAX_PATH;
		GetSaveFileName(&ofn);
		//buff
#endif
	}
	ImGui::Spacing();

	ImGui::Separator();
	if (ImGui::Button("Create"))
	{
		ofstream out;
		std::string t(buff3);
		if (t.find(".json") == std::string::npos)
			t += ".json";
		out.open(t);
		json j;
		auto& defaultInfo = j["defaultInfo"];
		defaultInfo["name"] = buff;
		defaultInfo["texturePath"] = buff2;
		j["Groups"];
		out << std::setw(4) << j;
		out.close();
		clearBuffers();
	}
	ImGui::EndChild();
}

void CAE::Application::editor()
{
	ImGui::BeginChild("Editor", ImVec2(window->getSize().x / 3, window->getSize().y / 2), true);
	if (currAsset != nullptr)
	{
		ImGui::Checkbox("Creator mode", &creatorMode);
		ImGui::Text(("Current asset name: " + currAsset->name).c_str());
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("anim groups: ");
		ImGui::Spacing();
		ImGui::SetNextItemWidth(32);

		if (currAsset->groups.empty())
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "groups is empty");
		else
		{
			if (tools_container.current_if(tool_type::MAGICTOOL))
			{
				auto mg = tools_container.getTool<_MagicTool>(tool_type::MAGICTOOL);
				static std::string selectedGroup;
				static bool renameState = false;
				int id = 0;
				static int _id = 0;
				for (auto& group : currAsset->groups)
				{
					ImGui::PushID(++id);
					if ((renameState && !(_id == id)) || !renameState)
					{
						if (ImGui::Selectable(group->getName().c_str(), selectedGroup == group->getName()))
						{
							selectedGroup = group->getName();
							mg->setSelectedGroup(group);
						}
						if (ImGui::IsItemClicked(1))
						{
							_id = id;
							renameState = true;
						}
					}
					else if (ImGui::InputText("Rename", buff, 256, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						group->setName(buff);
						strcpy(buff, "");
						renameState = false;
					}
					ImGui::PopID();
				}
			}
			else
				editorDragDropLogic();
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::InputText("new group name", buff, IM_ARRAYSIZE(buff));
		if (ImGui::Button("add group"))
		{
			currAsset->groups.emplace_back(std::make_shared<Group>(buff));
			clearBuffers();
		}
		tapWindow();
	}
	else
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "The current asset is not selected");
	ImGui::EndChild();
}

void CAE::Application::exit()
{
	ImGui::BeginChild("Note");
	ImGui::OpenPopup("Save?");
	if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("SAVE SESSION?");
		ImGui::Separator();

		if (ImGui::Button("Yes", ImVec2(140, 0)))
		{
			saveState();
			if (currAsset != nullptr)
				currAsset->saveAsset();
			window->close();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(140, 0))) { window->close(); }
		ImGui::EndPopup();
	}
	ImGui::EndChild();
}

void CAE::Application::editorDragDropLogic()
{
	static int global_id = 0; //just a local 'name' for part, value used only in make_text 
	auto make_text = [](Part& p) ->std::string
	{
		return "rect #" + std::to_string(p.getId()) + ": " + "w:" + std::to_string(p.getRect().width) + ", h:" + std::to_string(p.getRect().height);
	};
	auto parse_data = [](std::string d)
	{
		std::pair<int, int> p;
		auto middle = d.find_first_of(":");
		p.first = std::stoi(d.substr(0, middle));
		p.second = std::stoi(d.substr(middle + 1, d.length() - middle));
		return p;
	};
	//////////////////Erase Add Buttons//////////////////
	ImGui::Image(ico_holder.getTexture(ico_t::Editor_Delete), sf::Vector2f{ 32,32 });
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
		{
			IM_ASSERT(payload->DataSize == sizeof(int));
			const char* payload_n = (const char*)payload->Data;
			auto [group_n, part_n] = parse_data(std::string(payload_n));
			if (group_n == -1) //editorSubArray erase
			{

			}
			else if (group_n == -2) //Group erase
			{
				selectedGroups.emplace(part_n);
				int _i = 0;
				for (auto& i : selectedGroups)
				{
					currAsset->groups.erase(currAsset->groups.begin() + (i - _i));
					++_i;
				}
				selectedGroups.clear();
			}
			else //Part erase
			{
				auto& p = currAsset->groups[group_n]->getParts();
				selectedParts.emplace(part_n);
				int _i = 0;
				for (auto& i : selectedParts)
				{
					p.erase(p.begin() + (i - _i));
					++_i;
				}
				selectedParts.clear();
				//p.erase(p.begin() + part_n);
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	ImGui::Image(ico_holder.getTexture(ico_t::Editor_Add), sf::Vector2f{ 32,32 });
	if (ImGui::IsWindowHovered() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		if (ImGui::GetIO().MouseReleased[0])
			editorSubArray.emplace_back(std::make_shared<Part>(sf::FloatRect(0, 0, 20, 20), ++global_id));
	//////////////////END//////////////////
	if (ImGui::TreeNode("Editor storage"))
	{
		int sub_part_id = 0;
		for (auto iter = editorSubArray.begin(); iter != editorSubArray.end(); ++iter)
		{
			ImGui::PushID(sub_part_id);
			auto& part = *iter;
			auto r = part->getRect();
			ImGui::Selectable(make_text(*part).c_str());

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				std::string str = std::to_string(-1) + ":" + std::to_string(sub_part_id);
				const char* s = str.c_str();
				ImGui::SetDragDropPayload("DAD", s, sizeof(s));
				ImGui::Text("Swap %s", make_text(*part).c_str());
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
				{
					IM_ASSERT(payload->DataSize == sizeof(int));
					const char* payload_n = (const char*)payload->Data;
					auto [group_id, part_id] = parse_data(std::string(payload_n));
					if (group_id == -1)
						std::swap(*(editorSubArray.begin() + part_id), *iter);
					else
						std::swap(*(currAsset->groups[group_id]->getParts().begin() + part_id), *iter);
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopID();
			++sub_part_id;
		}
		ImGui::TreePop();
	}
	int group_id = 0;
	static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_None; //ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	for (auto& group : currAsset->groups)
	{
		ImGui::PushID(group_id);
		ImGuiTreeNodeFlags nd = base_flags;
		if (group->isSelected())
			nd |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
		bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)group_id, nd, group->getName().c_str());
		if (ImGui::IsItemClicked() && ImGui::GetIO().KeyCtrl)
		{
			Console::AppLog::addLog("Node " + group->getName() + " Clicked", Console::info);
			if (group->setSelected(!group->isSelected()))
				selectedGroups.emplace(group_id);
			else
				selectedGroups.erase(group_id);
		}
		if (node_open)
		{
			//////////////////Drag&Drop group(State: TreeNode open)//////////////////
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
				{
					IM_ASSERT(payload->DataSize == sizeof(int));
					const char* payload_n = (const char*)payload->Data;
					auto [group_n, part_n] = parse_data(std::string(payload_n));
					if (group_n == -1)
					{
						group->getParts().push_back(editorSubArray[part_n]);
						editorSubArray.erase(editorSubArray.begin() + part_n);
					}
					else
					{
						auto& g = currAsset->groups[group_n];
						auto& p = currAsset->groups[group_n]->getParts();

						selectedParts.emplace(part_n);

						for (auto& i : selectedParts)
						{
							group->getParts().push_back(p[i]);
						}
						int _i = 0;
						for (auto& i : selectedParts)
						{

							g->getParts().erase(g->getParts().begin() + (i - _i));
							++_i;
						}
						//group->getParts().push_back(p[part_n]);
						//g->getParts().erase(g->getParts().begin() + part_n);
					}
				}
				ImGui::EndDragDropTarget();
			}

			//////////////////END//////////////////
			static int prsId = 0;
			if (ImGui::ImageButton(ico_holder.getTexture(ico_t::Magic), { 32,32 }))
			{
				selectArea = true;
				prsId = group_id;
			}
			else
			{
				if (selectArea && prsId == group_id)
				{
					static sf::IntRect rect;
					static bool once = false;
					if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
					{
						if (!once)
						{
							once = true;
							auto pos = window->mapPixelToCoords(sf::Mouse::getPosition(*window), view);
							rect.left = pos.x;
							rect.top = pos.y;
						}
						else
						{
							sf::Vector2f p = window->mapPixelToCoords(sf::Mouse::getPosition(*window), view);
							auto delta = p - shape.getPosition();
							rect.width = delta.x;
							rect.height = delta.y;
						}
						shape.setPosition(sf::Vector2f(rect.left, rect.top));
						shape.setSize(sf::Vector2f(rect.width, rect.height));
					}
					else if (once)
					{
						std::string s = std::to_string(rect.width) + " _ " + std::to_string(rect.height) + " | " + std::to_string(rect.left) + " _ " + std::to_string(rect.top);
						Console::AppLog::addLog(s, Console::info);
						magicTool.cropSrc(rect, true);
						for (auto& r : magicTool.makeBounds())
							group->getParts().emplace_back(std::make_shared<Part>(r, ++global_id));
						Console::AppLog::addLog(std::to_string(group_id) + group->getName(), Console::info);
						selectArea = false;
						once = false;
						rect = {};
						shape.setPosition(sf::Vector2f(0, 0));
						shape.setSize(sf::Vector2f(0, 0));
					}
				}
			}

			bool isVisible = group->isVisible();
			ImGui::Checkbox("isVisible", &isVisible);
			group->setVisible(isVisible);

			int part_id = 0;
			for (auto iter = group->getParts().begin(); iter != group->getParts().end(); ++iter)
			{
				ImGui::PushID(part_id);
				auto& part = *iter;
				auto r = part->getRect();
				ImGui::Selectable(make_text(*part).c_str(), part->isSelected());
				if (ImGui::IsItemClicked() && ImGui::GetIO().KeyCtrl)
				{
					Console::AppLog::addLog("Part", Console::info);
					if (part->setSelected(!part->isSelected()))
						selectedParts.emplace(part_id);
					else
						selectedParts.erase(part_id);
				}
				//////////////////Drag&Drop parts//////////////////
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					std::string str = std::to_string(group_id) + ":" + std::to_string(part_id);
					const char* s = str.c_str();
					ImGui::SetDragDropPayload("DAD", s, sizeof(s));
					ImGui::Text("Swap %s", make_text(*part).c_str());
					ImGui::EndDragDropSource();
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
					{
						IM_ASSERT(payload->DataSize == sizeof(int));
						const char* payload_n = (const char*)payload->Data;
						auto [group_id, part_id] = parse_data(std::string(payload_n));
						if (group_id == -1)
							std::swap(*(editorSubArray.begin() + part_id), *iter);
						else
							std::swap(*(currAsset->groups[group_id]->getParts().begin() + part_id), *iter);
					}
					ImGui::EndDragDropTarget();
				}
				//////////////////END//////////////////
				ImGui::PopID();
				++part_id;
			}
			ImGui::TreePop();
		}
		else
		{
			//////////////////Drag&Drop group(State: TreeNode closed)//////////////////
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				std::string str = std::to_string(-2) + ":" + std::to_string(group_id);
				const char* s = str.c_str();
				ImGui::SetDragDropPayload("DAD", s, sizeof(int));
				ImGui::Text("Swap %s", "node");
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
				{
					IM_ASSERT(payload->DataSize == sizeof(int));
					const char* payload_n = (const char*)payload->Data;
					auto [group_n, part_n] = parse_data(std::string(payload_n));
					if (group_n == -1)
					{
						group->getParts().push_back(editorSubArray[part_n]);
						editorSubArray.erase(editorSubArray.begin() + part_n);
					}
					else
					{
						auto& g = currAsset->groups[group_n];
						auto& p = currAsset->groups[group_n]->getParts();

						selectedParts.emplace(part_n);

						for (auto& i : selectedParts)
						{
							group->getParts().push_back(p[i]);

						}
						int _i = 0;
						for (auto& i : selectedParts)
						{

							g->getParts().erase(g->getParts().begin() + (i - _i));
							++_i;
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
			//////////////////END//////////////////
		}
		ImGui::PopID();
		++group_id;
	}
}

void CAE::Application::mainWindow()
{
	ImGui::BeginChild("MainWindow");
	if (ImGui::Button("Load Last Session"))
		loadState();
	ImGui::EndChild();
}

void CAE::Application::viewLoadedAssets()
{
	ImGui::BeginChild("Loaded assets");
	ImGui::Text("Loaded assets: ");
	if (animAssets.empty())
		ImGui::Text("nothing");
	else
	{
		ImGui::Separator();
		for (auto iter = animAssets.begin(); iter != animAssets.end();)
		{
			auto a = *iter;
			bool erase = false;
			if (ImGui::TreeNode(a->getName().c_str()))
			{
				ImGui::Image(*a, { 200.f,200.f });
				ImGui::Spacing();
				ImGui::Text("Info: ");
				sf::Vector2f size = (sf::Vector2f)a->getTexture()->getSize();
				auto WH = a->getWH();
				ImGui::Text("Texture Size: %.2f %.2f", size.x, size.y);
				ImGui::Text("Texture Path: %s", a->getPath().c_str());
				ImGui::Text("step on the axis Width: %i Height: %i", WH.first, WH.second);
				if (ImGui::Button("select as current"))
					currAsset = a;
				if (ImGui::Button("free"))
					erase = true;
				ImGui::TreePop();
			}
			ImGui::Separator();
			if (erase)
			{
				if (currAsset == *iter)
					currAsset.reset();
				iter = animAssets.erase(iter);
			}
			else
				++iter;
		}
	}
	ImGui::EndChild();
}

void CAE::Application::saveAsset()
{
	ImGui::BeginChild("Save asset");
	ImGui::Text("Save current asset");
	if (currAsset != nullptr)
	{
		//ImGui::InputText("File Path", buff, IM_ARRAYSIZE(buff));
		if (ImGui::Button("Save File as"))
		{
#ifdef Windows
			char filename[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&filename, sizeof(filename));
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFilter = (".json");
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			if (GetSaveFileName(&ofn))
				currAsset->saveAsset(std::string(filename) + ".json");
			//clearBuffers();
#endif
		}
	}
	else
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "The current asset is not selected");
	ImGui::EndChild();
}

void CAE::Application::drawMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Assets"))
		{
			if (ImGui::MenuItem("Main Window", NULL, state == states::Null))
				state = states::Null;
			if (ImGui::MenuItem("Load Asset", NULL, state == states::LoadAsset))
				state = states::LoadAsset;
			if (ImGui::MenuItem("New", NULL, state == states::CreateAsset))
				state = states::CreateAsset;
			if (ImGui::MenuItem("Save Asset", NULL, state == states::SaveAsset))
				state = states::SaveAsset;
			if (ImGui::MenuItem("Edit Asset", NULL, state == states::Editor))
				state = states::Editor;
			if (ImGui::MenuItem("Loaded Assets", NULL, state == states::loadedAssets))
				state = states::loadedAssets;
			if (ImGui::MenuItem("Magic Tool", NULL, state == states::macgicTool))
				state = states::macgicTool;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::MenuItem("Window settings", "", state == states::WindowSettings))
				state = states::WindowSettings;
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void CAE::Application::drawUI()
{
	drawMenuBar();
	switch (state)
	{
	case CAE::Application::states::Exit:
		exit();
		break;
	case CAE::Application::states::Null:
		mainWindow();
		break;
	case CAE::Application::states::macgicTool:
		magicSelection();
		break;
	case CAE::Application::states::CreateAsset:
		createAssets();
		break;
	case CAE::Application::states::LoadAsset:
		loadAssets();
		break;
	case CAE::Application::states::SaveAsset:
		saveAsset();
		break;
	case CAE::Application::states::Editor:
		editor();
		break;
	case CAE::Application::states::loadedAssets:
		viewLoadedAssets();
		break;
	case CAE::Application::states::WindowSettings:
		viewSettings();
		break;
	default:
		break;
	}
}

void CAE::Application::addEventsHandler()
{
	EventManager& eManager = eventManagers.addEM("MainManager");
	eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::F1), &Application::changeMovingMode_e, this);
	eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::F2), &Application::useMouseToMove_e, this);
	eManager.addEvent(KBoardEvent::KeyReleased(sf::Keyboard::Delete), &Application::deletSelectedPart_e, this);
	eManager.addEvent(KBoardEvent::KeyReleased(sf::Keyboard::Tilde), [this](sf::Event&) {LogConsole = !LogConsole; });
	eManager.addEvent(MouseEvent::ButtonReleased(sf::Mouse::Left), [this](sf::Event&) { pointSelected = false; });
}

CAE::Application::Application(sf::RenderWindow& w)
	: tools_container(this, &w), window(&w), state(states::Null), useMouse(false), scaleFactor(1.5f), scaleSign(0),
	selectedPart(nullptr), selectedNode(nullptr), nodeSize(5), useFloat(true), eventManagers(),
      LogConsole(false)
{
	addEventsHandler();
	view.setSize(w.getDefaultView().getSize());
	scaleView = unscaleView = view.getSize();
	tools_container.Init();
}

void CAE::Application::clearBuffers()
{
	strcpy(buff, "");
	strcpy(buff2, "");
	strcpy(buff3, "");
}

void CAE::Application::loadState()
{
	if (ifstream open("config.json"); open.is_open())
	{
		json j;
		open >> j;
		try
		{
			auto& appSettings = j.at("Application Settings");
			std::string p = appSettings.at("currentAsset").get<std::string>();
			useMouse = appSettings.at("useMouse").get<bool>();
			creatorMode = appSettings.at("creatorMode").get<bool>();
			magicTool.load4Json(j.at("MagicTool Settings"));
			if (p != "Null")
				loadAsset(p, true);
		}
		catch (json::exception& e)
		{
			std::string str = e.what();
			Console::AppLog::addLog("Json throw exception, message: " + str, Console::error);
		}
		open.close();
	}
	else
	{
		Console::AppLog::addLog("File config.json can't be opened!", Console::system);
		saveState();
	}
}

void CAE::Application::saveState()
{
	ofstream open("config.json");
	if (currAsset != nullptr)
	{
		json j;
		auto& applicationMain = j["Application Settings"];
		applicationMain["currentAsset"] = currAsset->assetPath;
		applicationMain["useMouse"] = useMouse;
		applicationMain["creatorMode"] = creatorMode;
		j["MagicTool Settings"] = magicTool.save2Json();
		open << std::setw(4) << j;
	}
	open.close();
}

void CAE::Application::loadAsset(const std::string& path, bool setAsCurr)
{
	if (!path.empty())
	{
		auto task = [this, setAsCurr](const std::string& copyBuffer)
		{
            sf::Context _;  //loadFromFile() create texture
			auto ptr = std::make_shared<CAE::AnimationAsset>(copyBuffer);
			if (ptr->loadFromFile())
			{
				auto it = find_if(animAssets.begin(), animAssets.end(), [ptr](auto val)
					{
						return ptr->getName() == val->getName();
					});
				if (it != animAssets.end())
				{
					Console::AppLog::addLog("Assets loaded arleady", Console::message);
					//delete ptr;
				}
				else
					animAssets.emplace_back(ptr);
				if (setAsCurr)
				{
					currAsset = *animAssets.begin();
					tools_container.changeCurrAsset(currAsset);
					magicTool.setImage(*currAsset->getTexture());
					magicTool.makeTransformImage();
				}
			}
			//else
			//	delete ptr;
		};
		Console::AppLog::addLog("Loading asset from: " + path, Console::message);
        //task(path);
		std::thread(task, path).detach();
		clearBuffers();
	}
	else
		Console::AppLog::addLog("File does not exist or input is empty!", Console::message);
}

void CAE::Application::changeMovingMode_e(sf::Event&)
{
	useFloat = !useFloat;
	Console::AppLog::addLog(std::string("Change moving mode, floating: ") + (useFloat ? "true" : "false"), Console::message);
}

void CAE::Application::useMouseToMove_e(sf::Event&)
{
	useMouse = !useMouse;
	Console::AppLog::addLog(std::string("Using mouse to move: ") + (useMouse ? "true" : "false"), Console::message);
}

void CAE::Application::deletSelectedPart_e(sf::Event&)
{
	if (selectedGroup != nullptr && currAsset != nullptr && lastSelected != nullptr)
	{
		auto& ar = selectedGroup->getParts();
		ar.erase(std::find_if(ar.begin(), ar.end(), [this](std::shared_ptr<Part> p) {return lastSelected.get() == p.get(); }));
		lastSelected.reset();
	}
}

void CAE::Application::viewScale_e(sf::Event& event)
{
	if (!toolsWindowFocused)
	{
		//auto Nview = view.getSize();
		scaleView = view.getSize();
		if (event.mouseWheelScroll.delta < 0)
		{
			scaleView.x *= scaleFactor;
			scaleView.y *= scaleFactor;
			unscaleView.x /= scaleFactor;
			unscaleView.y /= scaleFactor;
			nodeSize += 0.3;
			scaleSign = 1;
		}
		else
		{
			scaleView.x /= scaleFactor;
			scaleView.y /= scaleFactor;

			unscaleView.x *= scaleFactor;
			unscaleView.y *= scaleFactor;
			nodeSize -= 0.3;
			scaleSign = -1;
		}

		//if (asyncNodeScale.joinable())
		//	asyncNodeScale.join();

		//asyncNodeScale = std::thread(
		//	[this]()
		//	{
		//		if (currAsset != nullptr)
		//		{
		//			for (auto group : *currAsset) //yep, one day it may fall
		//				for (auto part : *group) //yep, one day it may fall
		//					for (auto& node : part->getNode()) //yep, one day it may fall
		//						node.updateRadius(nodeSize); //yep, one day it may fall
		//		}
		//	});

		view.setSize(scaleView);
		viewUpdated();
	}
}

void CAE::Application::magicSelection()
{
	ImGui::BeginChild("Magic Selection Settings");
	ImGui::SliderInt("Type of transformation", &magicTool.mode, 0, 2);
	ImGui::Checkbox("user morph", &magicTool.useMorph);
	ImGui::Checkbox("user gray", &magicTool.gray);
	ImGui::SliderInt("thresh", &magicTool.thresh, 0, 255);
	ImGui::Checkbox("Just make the opaque color black", &magicTool.makeAllBlack);
	ImGui::SliderInt("Color add value", &magicTool.add, -255, 255);
	if (magicTool.useMorph)
	{
		ImGui::SliderInt("morphIteration", &magicTool.morph_iteration, -10, 10);
		ImGui::SliderInt("Rect w", &magicTool.kernel_rect.x, 0, 255);
		ImGui::SliderInt("Rect g", &magicTool.kernel_rect.y, 0, 255);
	}
	ImGui::Text("offset");
	ImGui::InputInt("left", &magicTool.offset.left);
	ImGui::InputInt("top", &magicTool.offset.top);
	ImGui::InputInt("rigt", &magicTool.offset.width);
	ImGui::InputInt("bottom", &magicTool.offset.height);
	if (ImGui::Button("Processed Image"))
		magicTool.makeTransformImage();
	if (ImGui::Button("Get Rects"))
	{
		magicTool.makeBounds();
		cv::imshow("th3", magicTool.source_image);
	}
	static sf::Texture texture;
	static sf::Sprite sprite;
	texture.loadFromImage(magicTool.getTransformPreview());
	sprite.setTexture(texture, true);
	ImGui::Image(sprite);
	ImGui::EndChild();
}

void CAE::Application::showLog(std::string_view txt)
{
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoInputs;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	auto io = ImGui::GetIO();
	ImVec2 window_pos = ImVec2(1.f, 1.f);
	ImGui::SetNextWindowPos(window_pos);
	ImGui::Begin("Console Message", 0, window_flags);
	ImGui::Text(txt.data());
	ImGui::End();
}

void CAE::Application::viewUpdated() {}

void CAE::Application::start()
{
	while (window->isOpen())
	{
		auto timePoint1(chrono::high_resolution_clock::now());
		sf::Event event;
		handleEvent(event);
		currentSlice += lastFt;
		ImGui::SFML::Update(*window, deltaClock.restart());
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		auto io = ImGui::GetIO();
		ImVec2 window_pos = ImVec2(io.DisplaySize.x - 1.f, 1.f);
		ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowSize(ImVec2(window->getSize().x / 3, window->getSize().y - 1.f));

		if (!ImGui::Begin("Tools Window", NULL, window_flags))
			ImGui::End();
		else
		{
			drawUI();
			toolsWindowFocused = ImGui::IsAnyWindowHovered();
			ImGui::End();
		}

#ifdef DEBUG
		ImGui::ShowDemoWindow();
		ImGui::Begin("Debug", 0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Mouse pos x: %f, y: %f", eManager.curr_mpos_f.x, eManager.curr_mpos_f.y);
		ImGui::Text("Mouse delta x: %f", eManager.worldMouseDelta());
		ImGui::Separator();
		ImGui::Text("Flags:");
		ImGui::Text("delta clock: %f", this->deltaClock.getElapsedTime().asSeconds());
		ImGui::Text("view pos %f %f", this->view.getCenter().x, this->view.getCenter().y);
		ImGui::Text("CreatorMode: %i", this->creatorMode);
		ImGui::Text("selectArea: %i", this->creatorMode);
		ImGui::Text("toolsWindowFocused: %i", this->toolsWindowFocused);
		ImGui::Text("LogConsole: %i", this->LogConsole);
		ImGui::Separator();
		ImGui::Text("state: %i", this->state);
		ImGui::Text("nodeSize: %f", this->nodeSize);
		ImGui::Separator();
		ImGui::Text("Buffers: ");
		ImGui::Text("buff: %s", this->buff);
		ImGui::Text("buff2: %s", this->buff2);
		ImGui::Text("buff3: %s", this->buff3);
		ImGui::End();
#endif // DEBUG

		update();
		draw();
		auto timePoint2(chrono::high_resolution_clock::now());
		auto elapsedTime(timePoint2 - timePoint1);
		float ft{ chrono::duration_cast<chrono::duration<float, milli>>(elapsedTime).count() };
		lastFt = ft;
	}
}
