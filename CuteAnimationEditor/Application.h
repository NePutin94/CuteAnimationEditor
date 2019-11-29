#pragma once
#include <SFML/Graphics.hpp>
#include "ConsoleLog.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>

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
		bool _isSelected;
		size_t id;
	public:
		Part(sf::Sprite s, PriorityOfDrawing p, size_t _id) : sprite(s), priority(p), _isSelected(false), id(_id) {}
		sf::Sprite& getSpite() { return sprite; }
		auto getPriority() { return priority; }
		auto getID() { return id; }
		void setPriority(PriorityOfDrawing p) { priority = p; }
		bool& isSelected() { return _isSelected; }
		operator sf::Drawable& ()
		{
			return sprite;
		}
	};

	class AnimationAsset
	{
	private:
		std::string texturePath;
		std::string assetPath;
		sf::Texture texture;
		sf::Sprite spr;
		std::string name;

		int width;
		int height;
		void sort();
	public:
		std::vector<Part> sheetFile;
		AnimationAsset(std::string_view _path);

		auto getPath() { return texturePath; }

		std::pair<int, int> getWH() { return std::make_pair(width, height); }

		void buildTileSheet();

		auto cbegin() const { return sheetFile.cbegin(); }

		auto cend() const { return sheetFile.cend(); }

		auto getName() const { return name; }

		const sf::Sprite& getSprite() const { return spr; }
		friend class Application;
	};

	class Application
	{
	private:
		sf::RenderWindow* window;
		sf::Clock deltaClock;
		sf::Clock pressClock;
		sf::View view;
		enum states
		{
			Null = 0,
			CreateAsset,
			LoadAsset,
			loadedAssets,
			SaveAsset,
			Editor,
			WindowSettings
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
		void ViewSettings();
		void createAssets();
		void editor();
		void viewLoadedAssets();
		void saveAsset() {}
		void drawMenuBar();
		void drawUI();

		void clearBuffers() {
			strcpy_s(buff, "");
			strcpy_s(buff2, "");
			strcpy_s(buff3, "");
		}
		bool useMouse;
		sf::Vector2f mPrevPose;
		sf::Vector2f mPrevMouse;
		//sf::Vector2f deltaForMoude;
	public:
		Application(sf::RenderWindow& w) : window(&w), state(Null), mPrevPose(), useMouse(false)
		{
			view.setSize(w.getDefaultView().getSize());
		}

		~Application()
		{
			for (auto& it : animAssets)
				delete it;
		}

		void start();
	};
}

