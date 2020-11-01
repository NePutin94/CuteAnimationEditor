#pragma once

#include "Tool.h"
#include "AnimationAsset.h"

namespace CAE
{
    class SelectionTool : public Tool
    {
    private:
        // sf::FloatRect selectionRect;
        sf::FloatRect second_draw;
        sf::RectangleShape shape;
        bool ButtonPressed;
        bool once;
        sf::IntRect rect;

        void assetUpdated() override
        {}

        void calcRect(sf::Rect<int> selectionRect)
        {
            if(selectionRect.width < 0)
            {
                selectionRect.left += selectionRect.width;
                selectionRect.width = abs(selectionRect.width);
            }
            if(selectionRect.height < 0)
            {
                selectionRect.top += selectionRect.height;
                selectionRect.height = abs(selectionRect.height);
            }
            sf::FloatRect selection;
            bool f = false;
            for(auto& g : *asset)
            {
                g->setSelected(false);
                for(auto& part : *g)
                {
                    part->setSelected(false);
                    if(auto r = part->getRect();
                            selectionRect.left <= r.left &&
                            selectionRect.top <= r.top &&
                            (selectionRect.left + selectionRect.width) >= (r.left + r.width) &&
                            (selectionRect.top + selectionRect.height) >= (r.top + r.height))
                    {
                        part->setSelected(true);
                        g->setSelected(true);
                        selection.width = std::max(r.left + r.width, selection.left + selection.width);
                        selection.height = std::max(r.top + r.height, selection.top + selection.height);
                        if(!f)
                        {
                            selection.left = r.left;
                            selection.top = r.top;
                            f = true;
                        } else
                        {
                            selection.left = std::min(r.left, selection.left);
                            selection.top = std::min(r.top, selection.top);
                        }
                        if(selection.left < 0)
                            selection.width += abs(selection.left);
                        else
                            selection.width -= selection.left;
                        if(selection.top < 0)
                            selection.height += abs(selection.top);
                        else
                            selection.height -= selection.top;
                    }
                }
            }
            shape.setPosition(sf::Vector2f(selection.left, selection.top));
            shape.setSize(sf::Vector2f(selection.width, selection.height));
        }

    public:
        SelectionTool(EMHolder& m, const sf::Texture& t, sf::RenderWindow& window) : Tool(m, t, window)
        {
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineColor(sf::Color::Blue);
            shape.setOutlineThickness(1);
        }

        void Enable() override
        {
            EventsHolder["SelectionTool"].setEnable(true);
        }

        void Disable() override
        {
            EventsHolder["SelectionTool"].setEnable(false);
        }

        void Init() override
        {
            auto& eManager = EventsHolder.addEM("SelectionTool", false);
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Escape), [this](sf::Event&)
            {
                for(auto& g : *asset)
                    for(auto& part : *g)
                        part->setSelected(false);
                shape.setPosition(sf::Vector2f(0, 0));
                shape.setSize(sf::Vector2f(0, 0));
            });
            eManager.addEvent(MouseEvent::ButtonPressed(sf::Mouse::Left), [this](sf::Event&)
            {
                ButtonPressed = true;
                shape.setPosition(sf::Vector2f(0, 0));
                shape.setSize(sf::Vector2f(0, 0));
            });
            eManager.addEvent(MouseEvent::ButtonReleased(sf::Mouse::Left), [this](sf::Event&)
            {
                if(once)
                {
                    ButtonPressed = false;
                    once = false;
                    calcRect(rect);
                }
            });
            eManager.addEvent(MouseEvent(sf::Event::MouseMoved), [this](sf::Event&)
            {
                if(ButtonPressed)
                {
                    if(!once)
                    {
                        once = true;
                        rect.left = EventsHolder.currMousePos().x;
                        rect.top = EventsHolder.currMousePos().y;
                    } else
                    {
                        auto delta = sf::Vector2f(rect.left, rect.top) - EventsHolder.currMousePos();
                        rect.width = -delta.x;
                        rect.height = -delta.y;
                    }
                    shape.setPosition(sf::Vector2f(rect.left, rect.top));
                    shape.setSize(sf::Vector2f(rect.width, rect.height));
                }
            });
        }

        void update() override
        {
            if(ImGui::IsAnyWindowHovered())
            {
                //rect = {};
                // shape.setPosition(sf::Vector2f(0, 0));
                //shape.setSize(sf::Vector2f(0, 0));
                once = ButtonPressed = false;
            }
        }

        void draw(sf::RenderWindow& w) override
        { w.draw(shape); }
    };

}