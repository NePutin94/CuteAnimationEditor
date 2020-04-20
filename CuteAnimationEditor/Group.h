#pragma once
#include <SFML/Graphics.hpp>
#include "ConsoleLog.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;
namespace CAE
{
	/*enum PriorityOfDrawing
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
	};*/


	class ScaleNode : public sf::CircleShape
	{
	public:
		int side;

		ScaleNode() = default;
		ScaleNode(sf::Vector2f pos, int _side = 0) : side(_side)
		{
			updateRadius(5);
			setPosition(pos);
		}
		void updateRadius(float r)
		{
			setRadius(r);
			setOrigin(r, r);
		}
	};

	class Part
	{
	private:
		sf::VertexArray quad;
		std::array<ScaleNode, 4> node;
		sf::Color color;
		bool isSelected;
		int id; //just local value, is specified at runtime

		void update();
	public:
		sf::FloatRect box;
		explicit Part(sf::FloatRect _rect);
		explicit Part(sf::FloatRect _rect, int id);
		~Part() = default;
		auto& getNode() { return node; }
		auto& getVertex() { return quad; }
		void changeColor(sf::Color c);
		auto getRect() { return box; }

		auto getId() { return id; }
		void coordToInt() { box = sf::FloatRect(floor(box.left), floor(box.top), floor(box.width), floor(box.height)); update(); }
		void setRect(sf::FloatRect rect);
	};

	class Group
	{
	private:
		bool isEnable;
		bool isLooped;
		float animSpeed;
		float scale;

		std::string name;
		std::vector<std::shared_ptr<Part>> parts;
	public:
		Group() = default;	
		explicit Group(std::string_view _name) : name(_name), isEnable(true), animSpeed(0.5f), isLooped(false), scale(1.f) {}
		~Group() = default;

		void setSpeed(float sp) { animSpeed = sp; }
		void setScale(float sc) { scale = sc; }
		void setVisible(bool v) { isEnable = v; }

		auto getName()   const { return name; }
		auto getScale()  const { return scale; }
		auto getSpeed()  const { return animSpeed; }
		auto begin()     const { return parts.begin(); }
		auto end()       const { return parts.end(); }
		bool isVisible() const { return isEnable; }
		auto& getParts() { return parts; }
		void save(json& j);
		void load(json& j);
	};
}