#include "Application.h"

void CAE::Application::handleEvent(sf::Event& event)
{
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			window->close();
			break;
		}
		ImGui::SFML::ProcessEvent(event);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tilde) && pressClock.getElapsedTime().asMilliseconds() > 500)
	{
		LogConsole = !LogConsole;
		pressClock.restart();
	}
}

void CAE::Application::draw()
{
	window->clear();
	if (currAsset != nullptr)
		window->draw(*currAsset);
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
		animAssets.push_back(new CAE::AnimationAsset{ buff });
		Console::AppLog::addLog("OK, Load", Console::logType::error);
	}
	ImGui::EndChild();
}

void CAE::Application::createAssets()
{
	ImGui::BeginChild("Create asset");
	ImGui::Text("Create asset: ");
	ImGui::InputText("Name", buff, IM_ARRAYSIZE(buff));
	ImGui::InputText("Texture path", buff2, IM_ARRAYSIZE(buff2));
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