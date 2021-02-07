#pragma once

#include <SFML/Graphics.hpp>
#include "ConsoleLog.h"
#include "AppColors.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
#include "EventHistory.h"

using json = nlohmann::json;
namespace CAE
{
    class ScaleNode : public sf::CircleShape
    {
    public:
        int side;

        ScaleNode() = default;

        ScaleNode(sf::Vector2f pos, int _side = 0) : side(_side)
        {
            updateRadius(3);
            setPosition(pos);
            setFillColor(CAE::Colors::Node);
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
        bool IsSelected;
        unsigned int id;
        static unsigned int _id;

        void update();

    public:
        sf::FloatRect box;

        Part(sf::FloatRect _rect);

        Part(sf::IntRect _rect);

        ~Part() = default;

        auto& getNode()
        { return node; }

        auto& getVertex()
        { return quad; }

        void changeColor(sf::Color c);

        auto getColor()
        { return color; }

        auto getRect()
        { return box; }

        auto getId()
        { return id; }

        void setId(int id)
        { this->id = id; }

        bool setSelected(bool s)
        {
            if(s) changeColor(CAE::Colors::Rect_s);
            else
                changeColor(CAE::Colors::Rect_us);
            return IsSelected = s;
        }

        bool isSelected()
        { return IsSelected; }

        void coordToInt()
        {
            box = sf::FloatRect(floor(box.left), floor(box.top), floor(box.width), floor(box.height));
            update();
        }

        void setRect(sf::FloatRect rect);
    };

    class Group
    {
    private:
        bool isEnable;
        bool isLooped;
        bool IsSelected;
        float animSpeed;
        float scale;
        unsigned int id;
        History_data* his;

        std::string name;
        std::vector<std::shared_ptr<Part>> parts;
    public:
        Group() = default;

        explicit Group(std::string_view _name) : name(_name), isEnable(true), animSpeed(30.f), isLooped(false),
                                                 scale(1.f),
                                                 IsSelected(false), id(0)
        {}

        Group(Group&& g) = default;

        ~Group() = default;

        void setName(std::string sp)
        { name = sp;  History_data::NeedSnapshot(); }

        void addPart(sf::FloatRect p)
        { addPart(Part(p)); }

        void addPart(sf::IntRect p)
        { addPart(Part(p)); }

        void addPart(Part p)
        {
            parts.emplace_back(std::make_shared<Part>(p));
            History_data::NeedSnapshot();
        }

        void setSpeed(float sp)
        { animSpeed = sp; }

        void setScale(float sc)
        { scale = sc; }

        void setVisible(bool v)
        { isEnable = v; }

        bool setSelected(bool s)
        { return IsSelected = s; }

        bool isSelected()
        { return IsSelected; }

        auto getName() const
        { return name; }

        auto getScale() const
        { return scale; }

        auto getSpeed() const
        { return animSpeed; }

        auto begin() const
        { return parts.begin(); }

        auto end() const
        { return parts.end(); }

        bool isVisible() const
        { return isEnable; }

        auto& getParts()
        { return parts; }

        int getId()
        { return id; }

        void save(json& j);

        void load(json& j);
    };
}