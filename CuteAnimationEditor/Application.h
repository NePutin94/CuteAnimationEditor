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

	class ScaleNode
	{
	public:
		sf::CircleShape c;
		int side;
		ScaleNode() = default;
		ScaleNode(sf::Vector2f pos, int _side = 0) : side(_side)
		{
			c.setRadius(5);
			c.setOrigin(5, 5);
			c.setPosition(pos);
		}
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

	class Group
	{
	private:
		std::string name;
		bool isEnable;
	public:
		std::vector<Part> parts;
		Group() = default;
		explicit Group(std::string_view _name) : name(_name), isEnable(true) {}
		const auto& getParts() { return parts; }
		auto getName() { return name; }
		auto begin() const { return parts.begin(); }
		auto end() const { return parts.end(); }
		void setVisible(bool v) { isEnable = v; }
		bool isVisible() { return isEnable; }
		void save(json& j)
		{
			j["name"] = name;
			int count = 0;
			auto& data = j["data"];
			for (auto& part : parts)
			{
				data[count]["pos"]["x"] = part.box.left;
				data[count]["pos"]["y"] = part.box.top;
				data[count]["width"] = part.box.width;
				data[count]["height"] = part.box.height;
				++count;
			}
		}

		void load(json& j)
		{
			name = j.at("name").get<std::string>();
			for (auto& part : j["data"])
			{
				sf::FloatRect r{};
				r.top = part["pos"]["y"].get<float>();
				r.left = part["pos"]["x"].get<float>();
				r.width = part["width"].get<float>();
				r.height = part["height"].get<float>();
				this->parts.emplace_back(r);
			}
		}
	};

	class AnimationAsset : public sf::Sprite
	{
	private:
		std::string texturePath;
		std::string assetPath;
		sf::Texture texture;
		std::string name;

		bool loadFromFile();
		bool saveAsset(std::string_view);
	public:
		std::vector<Group> groups;
		AnimationAsset(std::string_view _path);
		~AnimationAsset() { saveAsset(assetPath); };
		auto cbegin() const { return groups.cbegin(); }
		auto cend() const { return groups.cend(); }
		auto getName() const { return name; }
		auto getPath() { return texturePath; }
		std::pair<int, int> getWH() { return std::make_pair(texture.getSize().x, texture.getSize().y); }

		friend class Application;
	};

	class Application
	{
	private:
		AnimationAsset* currAsset;
		std::vector<AnimationAsset*> animAssets;
		enum class states
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

		sf::RenderWindow* window;
		sf::View view;
		sf::Clock deltaClock;
		sf::Clock pressClock;
		sf::Texture grid;
		sf::Sprite gridSpr;
		sf::Vector2f mPrevPose;
		sf::Vector2f mPrevMouse;
		sf::Vector2f mPrevMouse2;

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

		auto makeGrid(sf::Vector2f sz);
		void updateGrid(sf::Vector2f size);
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
		Application(sf::RenderWindow& w) : window(&w), state(states::Null), mPrevPose(), useMouse(false), zoom(1.5f)
		{
			view.setSize(w.getDefaultView().getSize());
		}
		~Application() { for (auto& it : animAssets)delete it; }

		void start();
	};
}

