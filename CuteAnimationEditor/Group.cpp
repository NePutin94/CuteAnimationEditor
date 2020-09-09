#include "Group.h"

void CAE::Part::update()
{
	auto [x, y, w, h] = box;
	quad[0].position = sf::Vector2f(x, y);
	quad[1].position = sf::Vector2f(x + w, y);
	quad[2].position = sf::Vector2f(x + w, y + h);
	quad[3].position = sf::Vector2f(x, y + h);
	quad[4].position = sf::Vector2f(x, y);
	node[0].setPosition({ x + w / 2, y });
	node[1].setPosition({ x + w,y + h / 2 });
	node[2].setPosition({ x + w / 2,y + h });
	node[3].setPosition({ x, y + h / 2 });
}

CAE::Part::Part(sf::FloatRect _rect) : box(_rect), /*prior(PriorityOfDrawing::Low),*/ quad(sf::LinesStrip, 5), color(sf::Color::Transparent), IsSelected(false)
{
	auto [x, y, w, h] = box;
	node[0] = ScaleNode({ x + w / 2, y }, 0);
	node[1] = ScaleNode({ x + w,y + h / 2 }, 1);
	node[2] = ScaleNode({ x + w / 2,y + h }, 2);
	node[3] = ScaleNode({ x, y + h / 2 }, 3);
	update();
	changeColor(sf::Color::Red);
}

CAE::Part::Part(sf::FloatRect _rect, int id) : id(id), box(_rect), /*prior(PriorityOfDrawing::Low),*/ quad(sf::LinesStrip, 5), color(sf::Color::Transparent), IsSelected(false)
{
	auto [x, y, w, h] = box;
	node[0] = ScaleNode({ x + w / 2, y }, 0);
	node[1] = ScaleNode({ x + w,y + h / 2 }, 1);
	node[2] = ScaleNode({ x + w / 2,y + h }, 2);
	node[3] = ScaleNode({ x, y + h / 2 }, 3);
	update();
	changeColor(sf::Color::Red);
}

void CAE::Part::changeColor(sf::Color c)
{
	if (c != color)
	{
		for (int i = 0; i < 5; ++i)
			quad[i].color = c;
		color = c;
	}
}

void CAE::Part::setRect(sf::FloatRect rect)
{
	box = rect;
	update();
}

void CAE::Group::save(json& j)
{
	j["name"] = name;
	j["isVisible"] = isEnable;
	j["animSpeed"] = animSpeed;
	j["scale"] = scale;
	int count = 0;
	auto& data = j["data"];
	for (auto& part : parts)
	{
		data[count]["pos"]["x"] = part->box.left;
		data[count]["pos"]["y"] = part->box.top;
		data[count]["width"] = part->box.width;
		data[count]["height"] = part->box.height;
		++count;
	}
}

void CAE::Group::load(json& j)
{
	name = j.at("name").get<std::string>();
	isEnable = j.at("isVisible").get<bool>();
	animSpeed = j.at("animSpeed").get<float>();
	scale = j.at("scale").get<float>();
	int id = 1;
	for (auto& part : j["data"])
	{
		sf::FloatRect r{};
		r.top = part["pos"]["y"].get<float>();
		r.left = part["pos"]["x"].get<float>();
		r.width = part["width"].get<float>();
		r.height = part["height"].get<float>();

		this->parts.emplace_back(std::make_shared<Part>(r, id));
		++id;
	}
}
