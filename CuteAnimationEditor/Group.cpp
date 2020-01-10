#include "Group.h"

void CAE::Part::update()
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

CAE::Part::Part(sf::FloatRect _rect) : box(_rect), prior(PriorityOfDrawing::Low), quad(sf::LinesStrip, 5)
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

void CAE::Group::save(json& j)
{
	j["name"] = name;
	j["isVisible"] = isEnable;
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

void CAE::Group::load(json& j)
{
	name = j.at("name").get<std::string>();
	isEnable = j.at("isVisible").get<bool>();
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
