#pragma once
#include <SFML/Graphics.hpp>
#include "ConsoleLog.h"
#include <chrono>
namespace CAE
{
	class Application
	{
	private:
		sf::RenderWindow* window;
		sf::Clock deltaClock;
		sf::Clock pressClock;

		float ftStep{ 1.f }, ftSlice{ 1.f };
		float lastFt{ 1.f };
		float currentSlice{ 0.f };
		bool LogConsole = false;
		bool needToChangeState = false;
		bool changeWithLoading = false;
	public:
		Application(sf::RenderWindow& w) : window(&w) {}
		void handleEvent(sf::Event& event)
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
		}
		void draw()
		{
			window->clear();
			Console::AppLog::Draw("LogConsole", &LogConsole);
			ImGui::SFML::Render(*window);
			window->display();
		}

		void start()
		{
			while (window->isOpen())
			{
				auto timePoint1(chrono::high_resolution_clock::now());
				sf::Event event;
				handleEvent(event);
				currentSlice += lastFt;
				ImGui::SFML::Update(*window, deltaClock.restart());
				draw();
				auto timePoint2(chrono::high_resolution_clock::now());
				auto elapsedTime(timePoint2 - timePoint1);
				float ft{
					chrono::duration_cast<chrono::duration<float, milli>>(elapsedTime).count() };
				lastFt = ft;
			}
		}
	};
}

