#pragma once

#include "Tool.h"
#include <cmath>

namespace CAE
{
    class MovePen : public Tool
    {
    private:
        std::shared_ptr<Part> selectedPart;
        std::shared_ptr<Group> selectedGroup;
        std::shared_ptr<Part> lastSelected;
        ScaleNode* selectedNode;
        bool& useFloat;
        bool MouseLeftPressed;
        bool KeyPressed;
        bool pointSelected;
        bool partSelected;
        sf::Vector2f oldPos;
        float zoom = 1;
        int selectedPoint = -1;

        void assetUpdated() override
        {}

    public:
        MovePen(EMHolder& m, const sf::Texture& t, sf::RenderWindow& window, bool& useFloatMove) : Tool(m, t, window),
                                                                                                   useFloat(useFloatMove),
                                                                                                   selectedNode(nullptr),
                                                                                                   MouseLeftPressed(false),
                                                                                                   KeyPressed(false), pointSelected(false)
        {}

        void Enable() override
        {
            EventsHolder["MovePen"].setEnable(true);
        }

        void Disable() override
        {
            EventsHolder["MovePen"].setEnable(false);
            MouseLeftPressed = false;
            pointSelected = false;
            selectedNode = nullptr;
            selectedPoint = -1;
            partSelected = false;
        }

        void Init() override
        {
            auto& eManager = EventsHolder.addEM("MovePen", false);
            eManager.addEvent(MouseEvent::ButtonPressed(sf::Mouse::Left), [this](sf::Event& event)
            {
                MouseLeftPressed = true;
            });
            eManager.addEvent(MouseEvent::ButtonReleased(sf::Mouse::Left), [this](sf::Event& event)
            {
                MouseLeftPressed = false;
                pointSelected = false;
                selectedNode = nullptr;
                selectedPoint = -1;
                partSelected = false;
            });
            eManager.addEvent(MouseEvent(sf::Event::MouseMoved), [this](sf::Event& event)
            {
                if(MouseLeftPressed && asset != nullptr)
                {
                    for(auto group : *asset)
                    {
                        if(group->isVisible())
                            for(std::shared_ptr<Part> elem : *group)
                            {
                                for(auto& p : elem->getNode()) //always 4 iteration
                                    if(!partSelected && ((p.getGlobalBounds().contains(EventsHolder.currMousePos())) ||
                                                         (selectedPoint == p.side && &p == selectedNode)))
                                    {
                                        pointSelected = true;
                                        selectedPoint = p.side;
                                        selectedNode = &p;
                                        auto rect = elem->getRect();
                                        sf::Vector2f value = {0, 0};
                                        sf::Vector2f delta = (sf::Vector2f) EventsHolder.getDelta();
                                        zoom = EventsHolder.getZoom();
                                        delta.x *= EventsHolder.zoom;
                                        delta.y *= EventsHolder.zoom;
                                        if(!useFloat)
                                        {
                                            static sf::Vector2f delta2;
                                            delta2 += delta;
                                            auto _delta = -(sf::Vector2i) delta2;
                                            int factor = 0.9;
                                            if(abs(_delta.x) > factor || abs(_delta.y) > factor)
                                            {
                                                if(abs(_delta.x) > factor)
                                                    value.x = _delta.x;
                                                if(abs(_delta.y) > factor)
                                                    value.y = _delta.y;
                                                delta2 = {0, 0};
                                            }
                                            rect = round(rect);
                                        } else
                                            value = -delta;

                                        switch(p.side)
                                        {
                                            case 0:
                                                elem->setRect(
                                                        sf::FloatRect(rect.left, rect.top + value.y, rect.width, rect.height - value.y));
                                                break;
                                            case 1:
                                                elem->setRect(sf::FloatRect(rect.left, rect.top, rect.width + value.x, rect.height));
                                                break;
                                            case 2:
                                                elem->setRect(sf::FloatRect(rect.left, rect.top, rect.width, rect.height + value.y));
                                                break;
                                            case 3:
                                                elem->setRect(
                                                        sf::FloatRect(rect.left + value.x, rect.top, rect.width - value.x, rect.height));
                                                break;
                                        }
                                        break;
                                    }
                                if(auto rect = elem->getRect();
                                        elem->isSelected() ||
                                        (!pointSelected &&
                                         ((selectedPart != nullptr) ? selectedPart == elem : true &&
                                                                                             elem->getRect().contains(EventsHolder.currMousePos())
                                         )))
                                {
                                    selectedPart = elem;
                                    selectedGroup = group;
                                    partSelected = true;
                                    sf::Vector2f delta = (sf::Vector2f) EventsHolder.getDelta();
                                    auto delta_f = EventsHolder.getDelta_F();
                                    zoom = window->getView().getSize().x / window->getSize().x;
                                    delta.x *= EventsHolder.zoom;
                                    delta.y *= EventsHolder.zoom;

                                    if(useFloat)
                                    {
                                        rect.left = rect.left - delta.x;
                                        rect.top = rect.top - delta.y;
                                        elem->setRect(rect);
                                    } else
                                    {
                                        static sf::Vector2f delta2;
                                        delta2 += delta;
                                        auto _delta = -(sf::Vector2i) delta2;
                                        int factor = 1;
                                        if(abs(_delta.x) > factor || abs(_delta.y) > factor)
                                        {
                                            if(abs(_delta.x) > factor)
                                                rect.left = round(rect.left) + _delta.x;
                                            if(abs(_delta.y) > factor)
                                                rect.top = round(rect.top) + _delta.y;
                                            delta2 = {0, 0};
                                        }
                                        elem->setRect(rect);
                                    }

                                    selectedPart->changeColor(sf::Color::Green);
                                    //lasSelectedPart may no longer be in the array
                                    //But resource will not be deleted, in this case we will just change the color
                                    //for the essentially 'dead' object(which is still valid)
                                    //it will be released when the lastSelected pointer changes to selectetPart
                                    //If it was not deleted, we will still just change the color
                                    if(lastSelected != nullptr && lastSelected != selectedPart)
                                    {
                                        lastSelected->changeColor(sf::Color::Red);
                                    }
                                    lastSelected = selectedPart;
                                }
                            }
                    }
                }
            });
        }

        void update() override
        {
            if(!MouseLeftPressed)
            {
                if(selectedPart != nullptr)
                    lastSelected = selectedPart;
                selectedPart.reset();
            }
            if (ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered()) {
                MouseLeftPressed = false;
                pointSelected = false;
                selectedNode = nullptr;
                selectedPoint = -1;
                partSelected = false; }

        }

        /*void handleEvenet(sf::Event& e) override
        {
        }*/

        void draw(sf::RenderWindow&) override
        {
        }
    };
}
