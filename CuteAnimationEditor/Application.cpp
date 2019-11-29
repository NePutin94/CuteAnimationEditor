#include "Application.h"
#include <iomanip>
#include <algorithm>
#include <future>
using json = nlohmann::json;

void CAE::Application::handleEvent(sf::Event& event)
{
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			window->close();
			break;
		}
		if (event.type == sf::Event::MouseButtonReleased)
			mPrevPose = { 0,0 };

		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Space)
				mPrevPose = window->mapPixelToCoords(sf::Mouse::getPosition(), view);
		}
		if (event.type == sf::Event::KeyReleased)
		{
		}
		ImGui::SFML::ProcessEvent(event);
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		if (useMouse)
		{
			auto mCurrPose = window->mapPixelToCoords(sf::Mouse::getPosition(), view);
			auto delta = mPrevPose - mCurrPose;
			Console::AppLog::addLog(std::to_string(delta.x) + ", " + std::to_string(delta.y), Console::info);
			view.move(delta);
		}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tilde) && pressClock.getElapsedTime().asMilliseconds() > 500)
	{
		LogConsole = !LogConsole;
		pressClock.restart();
	}
}

void CAE::Application::draw()
{
	window->setView(view);
	window->clear();
	if (currAsset != nullptr)
		for (auto riter = currAsset->sheetFile.rbegin(); riter != currAsset->sheetFile.rend(); ++riter)
		{
			if ((*riter).isSelected())
			{
				sf::Vector2f m_pos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
				sf::Sprite& spr = (*riter).getSpite();
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (spr.getGlobalBounds().contains(m_pos))
					{
						if (mPrevMouse.x != 0 && mPrevMouse.y != 0)
						{
							auto delta = m_pos - mPrevMouse;
							spr.move(delta);
						}
						mPrevMouse = m_pos;
					}
				}
				else
					mPrevMouse = { 0,0 };
				window->draw(*riter);
			}
			else
				window->draw(*riter);
		}
	Console::AppLog::Draw("LogConsole", &LogConsole);
	ImGui::SFML::Render(*window);
	window->display();
}

void CAE::Application::loadAssets()
{
	ImGui::BeginChild("Load assets");
	ImGui::InputText("Texture path", buff, IM_ARRAYSIZE(buff));
	if (ImGui::Button("Load"))
	{
		auto task = [this](std::string copyBuffer)
		{
			animAssets.push_back(new CAE::AnimationAsset{ copyBuffer });
		};
		std::string copy = buff;
		std::thread(task, copy).detach(); //Dangerous
		clearBuffers();
		Console::AppLog::addLog("OK, Load", Console::logType::error);
	}
	ImGui::EndChild();
}

void CAE::Application::ViewSettings()
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
				xSize = currAsset->getSprite().getTexture()->getSize().x;
				ySize = currAsset->getSprite().getTexture()->getSize().y;
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
	ImGui::InputInt("width", &w);
	ImGui::InputInt("height", &h);
	if (ImGui::Button("Create"))
	{
		ofstream out;
		out.open(buff3);
		json j;
		j["name"] = buff;
		j["texturePath"] = buff2;
		j["width"] = w;
		j["height"] = h;
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
		if (ImGui::Button("Sort"))
			currAsset->sort();
		if (ImGui::TreeNode("Change WH for asset"))
		{
			for (auto& i : currAsset->sheetFile)
			{
				ImGui::PushID(i.getID());
				ImGui::Text("(id: %i) W: %i H: %i", i.getID(), i.getSpite().getTextureRect().width, i.getSpite().getTextureRect().height);
				if (ImGui::BeginPopupContextItem("change"))
				{
					auto changedValue = i.getSpite().getTextureRect();
					auto changedValue2 = i.getSpite().getPosition();
					ImGui::PushItemWidth(200);
					ImGui::Text("Part of sprite: ");
					ImGui::Checkbox("Select for edit", &i.isSelected());
					ImGui::DragInt("#left", &changedValue.left, 1.f);
					ImGui::DragInt("#top", &changedValue.top, 1.f);
					ImGui::DragInt("#width", &changedValue.width, 1.f);
					ImGui::DragInt("#height", &changedValue.height, 1.f);
					ImGui::DragFloat("#pos.x", &changedValue2.x, 1.f);
					ImGui::DragFloat("#pos.y", &changedValue2.y, 1.f);
					std::string item_current = PriorityOfDrawing_s[i.getPriority()].data();
					ImGui::Spacing();
					ImGui::Text("Draw Priority: ");
					if (ImGui::BeginCombo("", item_current.c_str(), ImGuiComboFlags_NoArrowButton))
					{
						for (auto& item : PriorityOfDrawing_s)
						{
							bool is_selected = (item_current == item);
							if (ImGui::Selectable(item.data(), is_selected))
							{
								int z = 0;
								for (int i = 0; i < IM_ARRAYSIZE(PriorityOfDrawing_s); ++i)
									if (PriorityOfDrawing_s[i] == item)
									{
										z = i;
										break;
									}
								i.setPriority((PriorityOfDrawing)z);
								Console::AppLog::addLog(std::to_string(z) + ", item: " + item.data(), Console::info);
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					i.getSpite().setTextureRect(changedValue);
					i.getSpite().setPosition(changedValue2);
					ImGui::PopItemWidth();
					ImGui::EndPopup();
				}
				ImGui::PopID();
				ImGui::Separator();
			}
			ImGui::TreePop();
		}
	}
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
				ImGui::Image(a->getSprite(), { 200.f,200.f });
				ImGui::Spacing();
				ImGui::Text("Info: ");
				sf::Vector2f size = (sf::Vector2f)a->getSprite().getTexture()->getSize();
				auto WH = a->getWH();
				ImGui::Text("Texture Size: %.2f %.2f", size.x, size.y);
				ImGui::Text("Texture Path: %s", a->getPath().c_str());
				ImGui::Text("step on the axis Width: %i Height: %i", WH.first, WH.second);
				if (ImGui::Button("select as current"))
					currAsset = a;
				ImGui::TreePop();
			}
			ImGui::Separator();
		}
	}
	ImGui::EndChild();
}

void CAE::Application::drawMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Assets"))
		{
			if (ImGui::MenuItem("Main Window", "", state == states::Null))
				state = states::Null;
			if (ImGui::MenuItem("Load Asset", "", state == states::LoadAsset))
				state = states::LoadAsset;
			if (ImGui::MenuItem("Create Asset", "", state == states::CreateAsset))
				state = states::CreateAsset;
			if (ImGui::MenuItem("Save Asset", "", state == states::SaveAsset))
				state = states::SaveAsset;
			if (ImGui::MenuItem("Edit Asset", "", state == states::Editor))
				state = states::Editor;
			if (ImGui::MenuItem("Loaded Assets", "", state == states::loadedAssets))
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
	case CAE::Application::Null:
		break;
	case CAE::Application::CreateAsset:
		createAssets();
		break;
	case CAE::Application::LoadAsset:
		loadAssets();
		break;
	case CAE::Application::SaveAsset:
		saveAsset();
		break;
	case CAE::Application::Editor:
		editor();
		break;
	case CAE::Application::loadedAssets:
		viewLoadedAssets();
		break;
	case CAE::Application::WindowSettings:
		ViewSettings();
		break;
	default:
		break;
	}
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
		//ImGui::ShowDemoWindow();
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

		draw();
		auto timePoint2(chrono::high_resolution_clock::now());
		auto elapsedTime(timePoint2 - timePoint1);
		float ft{ chrono::duration_cast<chrono::duration<float, milli>>(elapsedTime).count() };
		lastFt = ft;

	}
}

void CAE::AnimationAsset::sort()
{
	auto pred = [](Part& p1, Part& p2)
	{
		int sumP1 = (int)p1.getPriority() + p1.isSelected();
		int sumP2 = (int)p2.getPriority() + p2.isSelected();
		return sumP1 > sumP2;
	};
	std::sort(sheetFile.begin(), sheetFile.end(), pred);
}

CAE::AnimationAsset::AnimationAsset(std::string_view _path) : assetPath(_path)
{
	if (std::ifstream i(_path.data()); i.is_open())
	{
		json j;
		i >> j;
		try
		{
			name = j.at("name").get<std::string>();
			width = j.at("width").get<int>();
			height = j.at("height").get<int>();
			texturePath = j.at("texturePath").get<std::string>();
		}
		catch (json::exception & e)
		{
			std::string str = e.what();
			Console::AppLog::addLog("Json throw exception, message: " + str, Console::error);
		}
		texture.loadFromFile(texturePath);
		spr.setTexture(texture);
		if (width != 0 && height != 0)
			buildTileSheet();
	}
	else
		Console::AppLog::addLog("File " + texturePath + " can't be opened!", Console::error);
}

void CAE::AnimationAsset::buildTileSheet()
{
	size_t id = 0;
	int x = 100;
	int y = 100;
	for (auto i = 0; i < texture.getSize().y;)
	{
		for (auto j = 0; j < texture.getSize().x;)
		{
			sf::Sprite spr;
			spr.setTexture(texture);
			spr.setTextureRect({ j, i, width, height });
			spr.setPosition(x, y);
			sheetFile.emplace_back(spr, PriorityOfDrawing::Medium, id);
			x += 5 + width;
			j += width;
			++id;
		}
		y += 5 + height;
		x = 100;
		i += height;
	}
}
