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
		std::vector<sf::Sprite> sheetFile;
		AnimationAsset(std::string_view _path)
		{
			std::ifstream i(_path.data());
			json j;
			i >> j;
			try
			{
				name = j.at("name").get<std::string>();
				width = j.at("width").get<int>();
				height = j.at("height").get<int>();
				path = j.at("texturePath").get<std::string>();
			}
			catch (json::exception & e)
			{
				std::string str = e.what();
				Console::AppLog::addLog("json throw exception, message: " + str, Console::error);
			}
			texture.loadFromFile(path);
			spr.setTexture(texture);
			if (width != 0 && height != 0)
				buildTileSheet();
		}

		void buildTileSheet()
		{
			int x = 100;
			int y = 100;
			//for (auto i = 0; i < texture.getSize().x;)
			//{
			//	for (auto j = 0; j < texture.getSize().y;)
			//	{
			//		sf::Sprite spr;
			//		spr.setTexture(texture);
			//		spr.setTextureRect({ i, j, width, height });
			//		spr.setPosition(x, y);
			//		sheetFile.emplace_back(spr);
			//		x += 5 + width;
			//		j += height;
			//	}
			//	y += 5 + height;
			//	x = 100;
			//	i += width;
			//}
			for (auto i = 0; i < texture.getSize().y;)
			{
				for (auto j = 0; j < texture.getSize().x;)
				{
					sf::Sprite spr;
					spr.setTexture(texture);
					spr.setTextureRect({ j, i, width, height });
					spr.setPosition(x, y);
					sheetFile.emplace_back(spr);
					x += 5 + width;
					j += width;
				}
				y += 5 + height;
				x = 100;
				i += height;
			}
		}

	/*	operator sf::Drawable& ()
		{
			return spr;
		}*/
		auto begin() { return sheetFile.begin(); }
		auto end() { return sheetFile.end(); }
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
		char buff3[256];

		void handleEvent(sf::Event& event);
		void draw();

		void loadAssets();
		void createAssets();
		void editor() {}
		void viewLoadedAssets();
		void saveAsset() {}
		void drawMenuBar();
		void drawUI();

		void clearBuffers() {
			strcpy_s(buff, "");
			strcpy_s(buff2, "");
			strcpy_s(buff3, "");
		}

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

