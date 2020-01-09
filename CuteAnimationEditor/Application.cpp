#include "Application.h"
#include <iomanip>
#include <algorithm>
#include <future>

void CAE::Application::handleEvent(sf::Event& event)
{
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			state = states::Exit;
			break;
		}

		if (event.type == sf::Event::MouseButtonReleased)
			if (event.key.code == sf::Mouse::Button::Left)
				pointSelected = false;
		if (event.type == sf::Event::MouseWheelScrolled)
		{
			if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {}
			else if (event.mouseWheelScroll.wheel == sf::Mouse::HorizontalWheel) {}

			auto prev = view.getSize();
			if (event.mouseWheelScroll.delta < 0)
			{
				prev.x *= zoom;
				prev.y *= zoom;
			}
			else
			{
				prev.x /= zoom;
				prev.y /= zoom;
			}
			view.setSize(prev);
		}
		ImGui::SFML::ProcessEvent(event);
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		if (useMouse)
		{
			auto mCurrPose = window->mapPixelToCoords(sf::Mouse::getPosition(), view);
			auto delta = mPrevPose - mCurrPose;
			view.move(delta);
		}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tilde) && pressClock.getElapsedTime().asMilliseconds() > 500)
	{
		LogConsole = !LogConsole;
		pressClock.restart();
	}

	mPrevPose = window->mapPixelToCoords(sf::Mouse::getPosition(), view);
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
			for (auto& elem : currAsset->groups)
			{
				if (elem.isVisible())
					for (auto& part : elem.parts)
					{
						window->draw(part.getVertex());
						for (auto& node : part.getNode())
							window->draw(node);
					}
			}
		}
		//window->draw(gridSpr);
	}
	Console::AppLog::Draw("LogConsole", &LogConsole);
	ImGui::SFML::Render(*window);
	window->display();
}

void CAE::Application::update()
{
	if (currAsset != nullptr)
	{
		if (creatorMode)
		{
			auto m_pos = window->mapPixelToCoords(sf::Mouse::getPosition(*window), view);
			static ScaleNode* selectedNode = nullptr;
			static Part* selectedPart = nullptr;
			for (auto& group : currAsset->groups)
			{
				if (group.isVisible())
				{
					for (auto& elem : group.parts)
					{
						if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
						{
							if (auto rect = elem.getRect(); elem.getRect().contains(m_pos) && ((selectedPart != nullptr) ? selectedPart == &elem : true))
							{
								if (mPrevMouse.x != 0 && mPrevMouse.y != 0)
								{
									auto delta = m_pos - mPrevMouse;
									rect.left += delta.x;
									rect.top += delta.y;
									elem.setRect(rect);
									selectedPart = &elem;
								}
							}
						}
						else
							selectedPart = nullptr;

						static int selectedPoint = -1;
						for (auto& p : elem.getNode())
						{
							if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
							{
								if ((p.c.getGlobalBounds().contains(m_pos) && !pointSelected) || (pointSelected && selectedPoint == p.side && &p == selectedNode))
								{
									pointSelected = true;
									selectedPoint = p.side;
									selectedNode = &p;
									if (auto rect = elem.getRect(); mPrevMouse2.x != 0 && mPrevMouse2.y != 0)
									{
										auto delta = sf::Vector2f{ m_pos - mPrevMouse2 };
										switch (p.side)
										{
										case 0:
											elem.setRect(sf::FloatRect(rect.left, rect.top + delta.y, rect.width, rect.height + delta.y * -1));
											break;
										case 1:
											elem.setRect(sf::FloatRect(rect.left, rect.top, rect.width + delta.x, rect.height));
											break;
										case 2:
											elem.setRect(sf::FloatRect(rect.left, rect.top, rect.width, rect.height + delta.y));
											break;
										case 3:
											elem.setRect(sf::FloatRect(rect.left + delta.x, rect.top, rect.width + delta.x * -1, rect.height));
											break;
										}
									}
									mPrevMouse2 = m_pos;
								}
							}
							else
								mPrevMouse2 = { 0,0 };
						}

					}
					mPrevMouse = m_pos;
				}
			}
		}
	}
}
void CAE::Application::updateGrid(sf::Vector2f size)
{
	if (currAsset != nullptr)
	{
		//grid.loadFromImage(makeGrid(size));
		gridSpr.setTexture(grid);
		gridSpr.setPosition(currAsset->getPosition());
	}
}
void CAE::Application::loadAssets()
{
	ImGui::BeginChild("Load assets");
	ImGui::InputText("Texture path", buff, IM_ARRAYSIZE(buff));
	if (ImGui::Button("Load"))
	{
		if (buff != NULL)
		{
			auto task = [this](std::string copyBuffer)
			{
				auto ptr = new CAE::AnimationAsset{ copyBuffer };
				if (ptr->loadFromFile())
					animAssets.push_back(ptr);
				else
					delete ptr;
			};
			std::string copy = buff;
			std::thread(task, copy).detach();
			clearBuffers();
		}
		else
			Console::AppLog::addLog("Cannot be loaded now", Console::logType::error);
	}
	ImGui::EndChild();
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
	static int w;
	static int h;
	static int s;
	ImGui::InputInt("width", &w);
	ImGui::InputInt("height", &h);
	ImGui::InputInt("space", &s);
	if (ImGui::Button("Create"))
	{
		ofstream out;
		out.open(buff3);
		json j;
		auto& defaultInfo = j["defaultInfo"];
		defaultInfo["name"] = buff;
		defaultInfo["texturePath"] = buff2;
		defaultInfo["width"] = w;
		defaultInfo["height"] = h;
		defaultInfo["space"] = s;
		out << std::setw(4) << j;
		out.close();
		w = 0;
		h = 0;
		clearBuffers();
	}
	ImGui::EndChild();
}

void CAE::Application::editor()
{
	ImGui::BeginChild("Editor");
	if (currAsset != nullptr)
	{
		ImGui::Checkbox("Creator mode", &creatorMode);
		ImGui::Text(("Current asset name: " + currAsset->name).c_str());
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("anim groups: ");
		ImGui::Spacing();
		int unique_id = 1;
		if (currAsset->groups.empty())
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "groups is empty");
		else
		{
			for (auto& group : currAsset->groups)
			{
				ImGui::PushID(unique_id);

				if (ImGui::TreeNode(group.getName().c_str()))
				{
					if (ImGui::Button("add rect"))
						group.parts.emplace_back(sf::FloatRect(0, 0, 20, 20));

					bool isVisible = group.isVisible();
					ImGui::Checkbox("isVisible", &isVisible);
					group.setVisible(isVisible);

					int i = 0;
					for (auto& part : group.parts)
					{
						ImGui::PushID(i);
						auto r = part.getRect();
						ImGui::Text("rect #%i: %.2f %.2f %.2f %.2f", i, r.left, r.top, r.width, r.height);
						if (ImGui::BeginPopupContextItem("change"))
						{
							auto changedValue = r;
							ImGui::DragFloat("#left", &changedValue.left, 0.5f, -FLT_MAX, FLT_MAX);
							ImGui::DragFloat("#top", &changedValue.top, 0.5f, -FLT_MAX, FLT_MAX);
							ImGui::DragFloat("#width", &changedValue.width, 0.5f, 0.f, FLT_MAX);
							ImGui::DragFloat("#height", &changedValue.height, 0.5f, 0.f, FLT_MAX);
							part.setRect(changedValue);
							ImGui::EndPopup();
						}
						ImGui::PopID();
						++i;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
				++unique_id;
			}
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::InputText("new group name", buff, IM_ARRAYSIZE(buff));
		if (ImGui::Button("add group"))
		{
			currAsset->groups.emplace_back(buff);
			clearBuffers();
		}
	}
	else
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "The current asset is not selected");
	ImGui::EndChild();
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
		for (auto a : animAssets)
		{
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
				{
					currAsset = a;
					updateGrid({ 80,80 });
				}
				ImGui::TreePop();
			}
			ImGui::Separator();
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
		ImGui::InputText("File Path", buff, IM_ARRAYSIZE(buff));
		if (ImGui::Button("Save"))
		{
			currAsset->saveAsset(buff);
			clearBuffers();
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
		ImGui::PushItemWidth(50.f);
		ImGui::Text("SAVE SESSION?");
		ImGui::Separator();
		if (ImGui::Button("Yes"))
		{
			saveState();
			window->close();
		}
		ImGui::SameLine();
		if (ImGui::Button("NO")) window->close();
		ImGui::PopItemWidth();
		ImGui::EndChild();
		break;
	case CAE::Application::states::Null:
		mainWindow();
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
		std::string p = j["currentAsset"].get<std::string>();
		useMouse = j["useMouse"].get<bool>();
		open.close();
		if (p != "Null")
		{
			auto task = [this](const std::string path)
			{
				auto ptr = new CAE::AnimationAsset{ path };
				if (ptr->loadFromFile())
				{
					animAssets.push_back(ptr);
					currAsset = *animAssets.begin();
					updateGrid({ 80,80 });
				}
				else
					delete ptr;
			};
			std::thread(task, p).detach();
		}
	}
	else
		Console::AppLog::addLog("File config.json can't be opened!", Console::error);
}

void CAE::Application::saveState()
{
	if (ofstream open("config.json"); open.is_open())
	{
		json j;
		j["currentAsset"] = (currAsset == nullptr) ? "Null" : currAsset->assetPath;
		j["useMouse"] = useMouse;
		open << std::setw(4) << j;
		open.close();
	}
	else
		Console::AppLog::addLog("File config.json can't be opened!", Console::error);
}

auto CAE::Application::makeGrid(sf::Vector2f sz)
{
	sf::RenderTexture t;
	t.create(currAsset->texture.getSize().x, currAsset->texture.getSize().y);
	t.clear(sf::Color(0, 0, 0, 0));
	auto deltaX = currAsset->texture.getSize().x / sz.x;
	for (int i = 0; i < sz.x; ++i)
	{
		sf::Vertex line[] =
		{
			sf::Vertex(sf::Vector2f(i * deltaX, 0)),
			sf::Vertex(sf::Vector2f(i * deltaX,  currAsset->texture.getSize().y))
		};
		t.draw(line, 2, sf::Lines);
	}
	auto deltaY = currAsset->texture.getSize().y / sz.y;
	for (int i = 0; i < sz.y; ++i)
	{
		sf::Vertex line[] =
		{
			sf::Vertex(sf::Vector2f(0, i * deltaY)),
			sf::Vertex(sf::Vector2f(currAsset->texture.getSize().x,  i * deltaY))
		};
		t.draw(line, 2, sf::Lines);
	}
	return sf::Image(t.getTexture().copyToImage());
}

void CAE::Application::start()
{
	Console::AppLog::addLog("Application::start()", Console::logType::info);
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
			ImGui::End();
		}
		update();
		draw();
		auto timePoint2(chrono::high_resolution_clock::now());
		auto elapsedTime(timePoint2 - timePoint1);
		float ft{ chrono::duration_cast<chrono::duration<float, milli>>(elapsedTime).count() };
		lastFt = ft;

	}
}

bool CAE::AnimationAsset::loadFromFile()
{
	if (std::ifstream i(assetPath.data()); i.is_open())
	{
		json j;
		i >> j;
		try
		{
			auto info = j.at("defaultInfo");
			name = info.at("name").get<std::string>();
			texturePath = info.at("texturePath").get<std::string>();
			for (auto& group : j.at("Groups"))
			{
				groups.emplace_back(Group());
				groups.back().load(group);
			}
		}
		catch (json::exception & e)
		{
			std::string str = e.what();
			Console::AppLog::addLog("Json throw exception, message: " + str, Console::error);
		}
		texture.loadFromFile(texturePath);
		setTexture(texture);
		return true;
	}
	else
		Console::AppLog::addLog("File " + texturePath + " can't be opened!", Console::error);
	return false;
}

bool CAE::AnimationAsset::saveAsset(std::string_view path)
{
	ofstream o(path.data());
	json j;
	auto& info = j["defaultInfo"];
	info["name"] = name;
	info["texturePath"] = texturePath;

	int i = 0;
	auto& g = j["Groups"];
	for (auto& group : groups)
	{
		group.save(g[i]);
		++i;
	}
	o << std::setw(4) << j;
	o.close();
	return true;
}

CAE::AnimationAsset::AnimationAsset(std::string_view _path) : assetPath(_path) {}
