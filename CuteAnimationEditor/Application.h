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

	struct ScaleNode
	{
		ScaleNode() = default;
		ScaleNode(sf::Vector2f pos, int _side = 0) : side(_side)
		{
			c.setRadius(5);
			c.setOrigin(5, 5);
			c.setPosition(pos);
		}
		sf::CircleShape c;
		int side;
		operator sf::Drawable& ()
		{
			return c;
		}
	};

	class Part
	{
	private:
		sf::VertexArray quad;
		std::array<ScaleNode, 4> node;
		bool isSelected;
		void update()
		{
			auto [x, y, w, h] = box;
			quad[0].position = sf::Vector2f(x, y);
			quad[1].position = sf::Vector2f(x + w, y);
			quad[2].position = sf::Vector2f(x + w, y + h);
			quad[3].position = sf::Vector2f(x, y + h);
			quad[4].position = sf::Vector2f(x, y);
			node[0].c.setPosition({ x + w / 2, y });
			node[1].c.setPosition({ x + w,y + h / 2 });
			node[2].c.setPosition({ x + w / 2,y + h });
			node[3].c.setPosition({ x, y + h / 2 });
		}
		PriorityOfDrawing prior;
	public:
		sf::FloatRect box;
		Part(sf::FloatRect _rect) : box(_rect), prior(PriorityOfDrawing::Low), quad(sf::LinesStrip, 5)
		{
			auto [x, y, w, h] = box;
			node[0] = ScaleNode({ x + w / 2, y }, 0);
			node[1] = ScaleNode({ x + w,y + h / 2 }, 1);
			node[2] = ScaleNode({ x + w / 2,y + h }, 2);
			node[3] = ScaleNode({ x, y + h / 2 }, 3);
			update();
			for (int i = 0; i < 5; ++i)
				quad[i].color = sf::Color::Red;
		}
		auto& getNode() { return node; }
		auto& getVertex() { return quad; }
		auto getRect() { return box; }
		void setRect(sf::FloatRect rect)
		{
			box = rect;
			update();
		}
	};

	class AnimationAsset : public sf::Sprite
	{
	private:
		std::string texturePath;
		std::string assetPath;
		sf::Texture texture;
		std::string name;

		int width;
		int height;

		bool loadFromFile();
		bool saveAsset(std::string_view);
	public:
		std::vector<Part> sheetFile;
		AnimationAsset(std::string_view _path);

		auto cbegin() const { return sheetFile.cbegin(); }
		auto cend() const { return sheetFile.cend(); }
		auto getName() const { return name; }
		auto getPath() { return texturePath; }
		std::pair<int, int> getWH() { return std::make_pair(width, height); }

		friend class Application;
	};

	class Application
	{
	private:
		sf::RenderWindow* window;
		sf::View view;
		AnimationAsset* currAsset;
		std::vector<AnimationAsset*> animAssets;

		sf::Clock deltaClock;
		sf::Clock pressClock;

		sf::Vector2f mPrevPose;
		sf::Vector2f mPrevMouse;
		sf::Vector2f mPrevMouse2;
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
		float zoom;
		char buff[256];
		char buff2[256];
		char buff3[256];

		bool LogConsole;
		bool useMouse;
		bool creatorMode;
		bool pointSelected;

		void handleEvent(sf::Event& event);
		void draw();
		void update();
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
		Application(sf::RenderWindow& w) : window(&w), state(Null), mPrevPose(), useMouse(false), zoom(1.5f) { view.setSize(w.getDefaultView().getSize()); }
		~Application() { for (auto& it : animAssets)delete it; }

		void start();
	};
}

