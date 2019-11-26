#pragma once
#include <SFML/Graphics.hpp>
#include "ConsoleLog.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;
namespace CAE
{
	class AnimationAsset
	{
	private:
		std::string path;
		sf::Texture texture;
		sf::Sprite spr;
		std::string name;

		int width;
		int height;
	public:
		AnimationAsset(std::string_view _path)
		{
			std::ifstream i(_path.data());
			json j;
			i >> j;
			name = j.at("name").get<std::string>();
			texture.loadFromFile(j.at("texturePath").get<std::string>());
			spr.setTexture(texture);
		}
		operator sf::Drawable& ()
		{
			return spr;
		}
		auto getName() { return name; }
		sf::Sprite getSprite() { return spr; }
	};

	class Application
	{
	private:
		sf::RenderWindow* window;
		sf::Clock deltaClock;
		sf::Clock pressClock;
		enum states
		{
			Null = 0,
			CreateAsset,
			LoadAsset,
			loadedAssets,
			SaveAsset,
			Editor
		} state;
		std::vector<AnimationAsset*> animAssets;
		AnimationAsset* currAsset;

		float ftStep{ 1.f }, ftSlice{ 1.f };
		float lastFt{ 1.f };
		float currentSlice{ 0.f };
		bool LogConsole;
		char buff[256];
		char buff2[256];

		void handleEvent(sf::Event& event);
		void draw();

		void loadAssets();
		void createAssets();
		void editor() {}
		void viewLoadedAssets();
		void saveAsset() {}
		void drawMenuBar();
		void drawUI();

	public:
		Application(sf::RenderWindow& w) : window(&w), state(Null) {}
		~Application()
		{
			for (auto& it : animAssets)
				delete it;
		}

		void start();
	};
}

