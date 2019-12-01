#pragma once
#include <SFML/Graphics.hpp>
#include "ConsoleLog.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;

namespace CAE
{
	class Application;
	enum PriorityOfDrawing
	{
		Lowest = 0,
		Low,
		Medium,
		High,
		Highest
	};

	constexpr std::string_view PriorityOfDrawing_s[] =
	{
		 "Lowest",
		 "Low",
		 "Medium",
		 "High",
		 "Highest"
	};

	class Part
	{
	private:
		sf::Sprite sprite;
		PriorityOfDrawing priority;
		size_t id;
		bool _isSelected;
	public:
		Part(sf::Sprite s, PriorityOfDrawing p, size_t _id) : sprite(s), priority(p), _isSelected(false), id(_id) {}

		void setPriority(PriorityOfDrawing p) { priority = p; }
		sf::Sprite& getSpite() { return sprite; }
		auto getPriority() { return priority; }
		auto getID() { return id; }
		bool& isSelected() { return _isSelected; }

		operator sf::Drawable& () { return sprite; }
	};

	class AnimationAsset
	{
	private:
		std::string texturePath;
		std::string assetPath;
		sf::Texture texture;
		sf::Sprite spr;
		std::string name;

		int space;
		int width;
		int height;

		void sort();
	public:
		std::vector<Part> sheetFile;
		AnimationAsset(std::string_view _path);

		void buildTileSheet();
		auto cbegin() const { return sheetFile.cbegin(); }
		auto cend() const { return sheetFile.cend(); }
		auto getName() const { return name; }
		auto getPath() { return texturePath; }
		const sf::Sprite& getSprite() const { return spr; }
		std::pair<int, int> getWH() { return std::make_pair(width, height); }

		friend class Application;
	};

	class Application
	{
	private:
		sf::RenderWindow* window;
		sf::Clock deltaClock;
		sf::Clock pressClock;
		sf::View view;
		sf::Vector2f mPrevPose;
		sf::Vector2f mPrevMouse;
		AnimationAsset* currAsset;
		std::vector<AnimationAsset*> animAssets;
		enum states
		{
			Null = 0,
			CreateAsset,
			LoadAsset,
			loadedAssets,
			SaveAsset,
			Editor,
			WindowSettings,
			Exit
		} state;

		float ftStep{ 1.f }, ftSlice{ 1.f };
		float lastFt{ 1.f };
		float currentSlice{ 0.f };
		char buff[256];
		char buff2[256];
		char buff3[256];
		bool LogConsole;
		bool useMouse;

		void handleEvent(sf::Event& event);
		void draw();
		void clearBuffers();
		void loadState();
		void saveState();

		void loadAssets();
		void viewSettings();
		void createAssets();
		void editor();
		void mainWindow();
		void viewLoadedAssets();
		void saveAsset();
		void drawMenuBar();
		void drawUI();
	public:
		Application(sf::RenderWindow& w) : window(&w), state(Null), mPrevPose(), useMouse(false) { view.setSize(w.getDefaultView().getSize()); }
		~Application() { for (auto& it : animAssets)delete it; }

		void start();
	};
}

