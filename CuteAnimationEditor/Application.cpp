#include "Application.h"
#include <iomanip>
#include <algorithm>
#include <future>
#include <filesystem>
//#define DEBUG
namespace fs = std::filesystem;

bool operator<(sf::Vector2i f, sf::Vector2i s)
{
	return (f.x < s.x) && (f.y < s.y);
}

bool operator>(sf::Vector2i f, sf::Vector2i s)
{
	return !(f < s);
}

template<class T>
sf::Rect<T> round(sf::Rect<T> value)
{
	return sf::Rect <T>{round(value.left), round(value.top), round(value.width), round(value.height)};
}

void CAE::Application::handleEvent(sf::Event& event)
{
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			state = states::Exit;
			break;
		}

		eManager.updateEvent(event);
		ImGui::SFML::ProcessEvent(event);
	}
	eManager.updateMousePosition(*window, view); //it must be performed before all updates for the current frame
	eManager.updateInput();
}

void CAE::Application::draw()
{
	window->setView(view);
	window->clear();
	if (currAsset != nullptr)
	{
		window->draw(*currAsset);
		if (creatorMode)
		{
			for (auto elem : *currAsset)
			{
				if (elem->isVisible())
					for (auto part : *elem)
					{
						window->draw(part->getVertex());
						for (auto& node : part->getNode())
							window->draw(node);
					}
			}
			//window->draw(sf::Sprite(magicTool.t.getTexture()));
		}
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
		editorUpdate();
}

void CAE::Application::editorUpdate()
{
	for (auto group : *currAsset)
	{
		if (group->isVisible())
		{
			for (std::shared_ptr<Part> elem : *group)
			{
				////////////////////////////////////MOVING(FLOAT OFFSET)//////////////////////////////////////////
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
				{
					if (auto rect = elem->getRect(); elem->getRect().contains(eManager.currMousePos()) && ((selectedPart != nullptr) ? selectedPart == elem : true))
					{
						selectedPart = elem;
						selectedGroup = group;
						if (useFloat)
						{
							auto delta = eManager.worldMouseDelta();
							rect.left -= delta.x;
							rect.top -= delta.y;
							elem->setRect(rect);
						}
						else
						{
							static sf::Vector2f delta2;
							delta2 += eManager.worldMouseDelta();
							auto delta = -(sf::Vector2i)delta2;
							int factor = 1;
							if (abs(delta.x) > factor || abs(delta.y) > factor)
							{
								if (abs(delta.x) > factor)
									rect.left = round(rect.left) + delta.x;
								if (abs(delta.y) > factor)
									rect.top = round(rect.top) + delta.y;
								delta2 = { 0,0 };
							}
							elem->setRect(rect);
						}

						selectedPart->changeColor(sf::Color::Green);
						//lasSelectedPart may no longer be in the array
						//But resource will not be deleted, in this case we will just change the color 
						//for the essentially 'dead' object(which is still valid)
						//it will be released when the lastSelected pointer changes to selectetPart
						//If it was not deleted, we will still just change the color
						if (lastSelected != nullptr && lastSelected != selectedPart)
						{
							lastSelected->changeColor(sf::Color::Red);
							//selectedGroup.reset();
						}
						lastSelected = selectedPart;
					}
				}
				else
				{
					if (selectedPart != nullptr)
						lastSelected = selectedPart;
					selectedPart.reset();
				}

				////////////////////////////////////SCALE//////////////////////////////////////////
				static int selectedPoint = -1;
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
				{
					for (auto& p : elem->getNode())
						if ((p.getGlobalBounds().contains(eManager.currMousePos()) /*&& !pointSelected*/) || (/*pointSelected &&*/ selectedPoint == p.side && &p == selectedNode))
						{
							//pointSelected = true;
							selectedPoint = p.side;
							selectedNode = &p;
							auto rect = elem->getRect();
							sf::Vector2f value = { 0,0 };
							if (!useFloat)
							{

								static sf::Vector2f delta2;
								delta2 += eManager.worldMouseDelta();
								sf::Vector2i delta = -(sf::Vector2i)delta2;
								int factor = 0.2;
								if (abs(delta.x) > factor || abs(delta.y) > factor)
								{
									value.x += ceil(delta.x);
									value.y += ceil(delta.y);
									delta2 = { 0,0 };
								}
								rect = round(rect);
							}
							else
								value = -eManager.worldMouseDelta();

							switch (p.side)
							{
							case 0:
								elem->setRect(sf::FloatRect(rect.left, rect.top + value.y, rect.width, rect.height - value.y));
								break;
							case 1:
								elem->setRect(sf::FloatRect(rect.left, rect.top, rect.width + value.x, rect.height));
								break;
							case 2:
								elem->setRect(sf::FloatRect(rect.left, rect.top, rect.width, rect.height + value.y));
								break;
							case 3:
								elem->setRect(sf::FloatRect(rect.left + value.x, rect.top, rect.width - value.x, rect.height));
								break;
							}
						}
				}
			}
		}
	}
}

void CAE::Application::loadAssets()
{
	ImGui::BeginChild("Load assets");
	ImGui::Spacing();
	ImGui::Text("Available assets: ");
	constexpr std::string_view default_path = "./assets/";
	if (fs::exists(default_path))
	{
		for (auto& p : fs::directory_iterator(default_path))
		{
			auto name = p.path().filename().string();
			if (p.path().extension() == ".json")
				if (ImGui::Selectable(name.c_str()))
				{
					loadAsset(p.path().parent_path().string() + "/" + name, true);
				}
		}
	}
	else
	{
		Console::AppLog::addLog("directory .\"assets\" not exist", Console::error);
	}
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text("Load from other path: ");
	if (ImGui::Button("Load From"))
	{
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
	ImGui::InputText("Name", buff, IM_ARRAYSIZE(buff));
	ImGui::InputText("Texture path", buff2, IM_ARRAYSIZE(buff2));
	ImGui::InputText("Output file", buff3, IM_ARRAYSIZE(buff3));
	if (ImGui::Button("Create"))
	{
		ofstream out;
		out.open(buff3);
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
			if (ImGui::Button("store coordinates as int"))
				for (auto group : *currAsset)
				{
					for (auto part : *group)
						part->coordToInt();
				}
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
	ImGui::Image(deleteSprite_ico, sf::Vector2f{ 32,32 });
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
				currAsset->groups.erase(currAsset->groups.begin() + part_n);
			else //Part erase
			{
				auto& p = currAsset->groups[group_n]->getParts();
				p.erase(p.begin() + part_n);
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	ImGui::Image(addSprite_ico, sf::Vector2f{ 32,32 });
	if (ImGui::IsWindowHovered() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
	{
		if (ImGui::GetIO().MouseReleased[0])
		{
			editorSubArray.emplace_back(std::make_shared<Part>(sf::FloatRect(0, 0, 20, 20), ++global_id));
		}
	}
	//////////////////END//////////////////
	if (ImGui::TreeNode("Local array"))
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
	for (auto& group : currAsset->groups)
	{
		ImGui::PushID(group_id);
		if (ImGui::TreeNode(group->getName().c_str()))
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
						group->getParts().push_back(p[part_n]);
						g->getParts().erase(g->getParts().begin() + part_n);
					}
				}
				ImGui::EndDragDropTarget();
			}
			//////////////////END//////////////////
			if (ImGui::Button("Get Magic Selection Rect"))
			{
				for (auto& r : magicTool.makeBounds())
					group->getParts().emplace_back(std::make_shared<Part>(r, ++global_id));
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
				ImGui::Selectable(make_text(*part).c_str());
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
						group->getParts().push_back(p[part_n]);
						g->getParts().erase(g->getParts().begin() + part_n);
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
			if (ImGui::MenuItem("Create Asset", NULL, state == states::CreateAsset))
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
		ImGui::BeginChild("Note");
		ImGui::OpenPopup("Save?");
		if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("SAVE SESSION?");
			ImGui::Separator();

			if (ImGui::Button("Yes", ImVec2(140, 0)))
			{
				saveState();
				window->close();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(140, 0))) { window->close(); }
			ImGui::EndPopup();
		}
		ImGui::EndChild();
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

void CAE::Application::clearBuffers()
{
	strcpy_s(buff, "");
	strcpy_s(buff2, "");
	strcpy_s(buff3, "");
}

void CAE::Application::loadState()
{
	if (ifstream open("config.json"); open.is_open())
	{
		json j;
		open >> j;
		try
		{
			std::string p = j.at("currentAsset").get<std::string>();
			useMouse = j.at("useMouse").get<bool>();
			creatorMode = j.at("creatorMode").get<bool>();
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
		j["currentAsset"] = currAsset->assetPath;
		j["useMouse"] = useMouse;
		j["creatorMode"] = creatorMode;
		open << std::setw(4) << j;
	}
	open.close();
}

void CAE::Application::loadAsset(std::string path, bool setAsCurr)
{
	if (!path.empty())
	{
		auto task = [this, setAsCurr](std::string copyBuffer)
		{
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
					magicTool.setImage(*currAsset->getTexture());
					magicTool.makeTransformImage();
				}
			}
			//else
			//	delete ptr;
		};
		Console::AppLog::addLog("Loading asset from: " + path, Console::message);
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
		auto prev = view.getSize();

		if (event.mouseWheelScroll.delta < 0)
		{
			prev.x *= scaleFactor;
			prev.y *= scaleFactor;
			nodeSize += 0.3;
			scaleSign = 1;
		}
		else
		{
			prev.x /= scaleFactor;
			prev.y /= scaleFactor;
			nodeSize -= 0.3;
			scaleSign = -1;
		}

		if (asyncNodeScale.joinable())
			asyncNodeScale.join();

		asyncNodeScale = std::thread(
			[this]()
			{
				if (currAsset != nullptr)
				{
					for (auto group : *currAsset) //yep, one day it may fall
						for (auto part : *group) //yep, one day it may fall
							for (auto& node : part->getNode()) //yep, one day it may fall
								node.updateRadius(nodeSize); //yep, one day it may fall
				}
			});

		view.setSize(prev);
		viewUpdated();
	}
}

void CAE::Application::magicSelection()
{
	ImGui::BeginChild("Magic Selection Settings");
	//if (magicTool.mode > 2)
	//{

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
	//}

	if (ImGui::Button("Processed Image"))
	{
		magicTool.makeTransformImage();
		/*magicTool.source_image = sfml2opencv(currAsset->getTexture()->copyToImage(), true);
		magicTool.transform_image = cv::Mat::zeros(magicTool.source_image.size(), magicTool.source_image.type());
		for (int i = 0; i < magicTool.transform_image.rows; ++i)
		{
			for (int j = 0; j < magicTool.transform_image.cols; ++j)
			{
				auto p = magicTool.source_image.at<cv::Vec4b>(i, j);
				if (p[3] != 0)
				{
					if (magicTool.makeAllBlack)
					{
						p[0] = 0;
						p[1] = 0;
						p[2] = 0;
					}
					else
					{
						p[0] += magicTool.add;
						p[1] += magicTool.add;
						p[2] += magicTool.add;
					}
				}
				magicTool.transform_image.at<cv::Vec4b>(i, j) = p;
			}
		}
		if (magicTool.gray)
			cvtColor(magicTool.transform_image, magicTool.transform_image, CV_RGB2GRAY);

		switch (magicTool.mode)
		{
		case 0:
		{
			magicTool.offset = { 1,1 ,-1,-1 };
		}
		break;
		case 1:
		{
			magicTool.offset = { 1,1 ,-1,-1 };
			cv::Mat sub_mat = cv::Mat::zeros(magicTool.transform_image.size(), CV_8UC3);
			cv::Canny(magicTool.transform_image, sub_mat, magicTool.thresh, magicTool.thresh * 2, 3);
			magicTool.transform_image = sub_mat;
		}
		break;
		case 2:
			cv::threshold(magicTool.transform_image, magicTool.transform_image, magicTool.thresh, 255, 0);
			break;
		case 3:
			break;
		}
		if (magicTool.useMorph)
		{
			auto rect_kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(magicTool.kernel_rect.x, magicTool.kernel_rect.y));
			cv::Mat sub_mat = cv::Mat::zeros(magicTool.transform_image.size(), CV_8UC3);
			cv::morphologyEx(magicTool.transform_image, sub_mat, cv::MORPH_CLOSE, rect_kernel, cv::Point(-1, -1), magicTool.morph_iteration);
			magicTool.transform_image = sub_mat;
		}
		cv::imshow("th2", magicTool.transform_image);*/
	}
	if (ImGui::Button("Get Rects"))
	{

		//vector<vector<cv::Point>> contours;
		//vector<cv::Vec4i> hierarchy;
		//findContours(magicTool.transform_image, contours, hierarchy, cv::RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

		//////addWeighted(image_2, 0.75, contours, 0.25, 0, drawing);
		//////drawContours(drawing, contours, -1, cv::Scalar(255));
		////cv::Mat rect = cv::Mat::zeros(th2.size(), CV_8UC3);
		////vector<vector<cv::Point> > contours_poly(contours.size());
		////vector<cv::Rect> boundRect(contours.size());
		////vector<cv::Point2f>centers(contours.size());
		////vector<float>radius(contours.size());
		//for (int i = 1; i >= 0; i = hierarchy[i][0])
		//{
		//	//print it
		//	//cv::drawContours(img, contours, i, cv::Scalar(255));
		//	//out.emplace_back(cv::boundingRect(contours[i]));
		//	//cv::rectangle(img, cv::boundingRect(contours[i]), cv::Scalar(120, 120, 255, 255));
		//	//for every of its internal contours
		//	//for (int j = hierarchy[i][2]; j >= 0; j = hierarchy[j][0])
		//	//{
		//	//	//	//recursively print the external contours of its children
		//	//	printExternalContours(img, contours, hierarchy, hierarchy[j][2]);
		//	//}
		//	//vector<cv::Point> poly;
		//	//approxPolyDP(contours[i], poly, 5, true);
		//	auto rect = boundingRect(contours[i]);
		//	//rect.x -= 10;
		//	//rect.y -= 10;
		//	cv::rectangle(magicTool.source_image, rect, cv::Scalar(10, 10, 255, 100));
		//	magicTool.boundRect.emplace_back(rect);
		//}
		//printExternalContours(magicTool.source_image, contours, hierarchy, 0);
		cv::imshow("th3", magicTool.source_image);
	}
	//sf::Texture t;
	//t.loadFromImage(magicTool.getTransformPreview());
	static sf::Texture texture;
	static sf::Sprite sprite;
	texture.loadFromImage(magicTool.getTransformPreview());
	sprite.setTexture(texture);
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

void CAE::Application::viewUpdated()
{
	//attention.setPosition(window->mapPixelToCoords(sf::Vector2i(0, 0), view));
	//auto scale = (scaleSign > 0) ? attention.getScale().x * scaleFactor : attention.getScale().x / scaleFactor;
	//attention.setScale({ scale, scale });
}

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
			toolsWindowFocused = ImGui::IsAnyWindowFocused();
			ImGui::End();
		}

#ifdef DEBUG
		ImGui::Begin("Debug", 0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Manager x: %f, y: %f", eManager.curr_mpos_f.x, eManager.curr_mpos_f.y);
		ImGui::Text("PrevManager x: %f, y: %f", eManager.prev_mpos_f.x, eManager.prev_mpos_f.y);
		ImGui::Text("delta x: %f", eManager.worldMouseDelta());
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
