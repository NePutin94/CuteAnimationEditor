#pragma once
#include <SFML/Graphics.hpp>
#include "ConsoleLog.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;
namespace CAE
{
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
		void update();
		PriorityOfDrawing prior;
	public:
		sf::FloatRect box;
		Part(sf::FloatRect _rect);

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
		bool isVisible() { return isEnable; }

		void setVisible(bool v) { isEnable = v; }
		void save(json& j);
		void load(json& j);
	};
}