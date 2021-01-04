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
        bool ctrlPress;
        bool altPress;
        bool Exit = false;
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
                                                                                                   KeyPressed(false), pointSelected(false),
                                                                                                   ctrlPress(false), altPress(false)
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
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Escape), [this](sf::Event&)
            {
                for(auto& g : *asset)
                    for(auto& part : *g)
                        part->setSelected(false);
            });
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::LControl), [this](sf::Event&)
            {
                ctrlPress = true;
            });
            eManager.addEvent(KBoardEvent::KeyReleased(sf::Keyboard::LControl), [this](sf::Event&)
            {
                ctrlPress = false;
            });
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::LAlt), [this](sf::Event&)
            {
                altPress = true;
            });
            eManager.addEvent(KBoardEvent::KeyReleased(sf::Keyboard::LAlt), [this](sf::Event&)
            {
                altPress = false;
            });
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
                if(ctrlPress)
                {
                    if(!Exit && !MouseLeftPressed && asset != nullptr)
                        asset->iterateByPart_s1([this](Part& part)
                                                {
                                                    if(part.isSelected() && part.getRect().contains(EventsHolder.currMousePos()))
                                                        part.setSelected(false);
                                                });

                }
            });

            //move rect and scale node by mouse
            eManager.addEvent(MouseEvent(sf::Event::MouseMoved), [this](sf::Event& event)
            {
                //bool endNodeUpdate = false;
                if(!Exit && MouseLeftPressed && asset != nullptr)
                {
                    for(auto group : *asset)
                    {
                        if(group->isVisible())
                            for(std::shared_ptr<Part> elem : *group)
                            {
                                for(auto& p : elem->getNode())
                                    if(bool arleadySelected = (selectedPoint == p.side && &p == selectedNode);
                                            !partSelected &&
                                            ((!pointSelected && p.getGlobalBounds().contains(EventsHolder.currMousePos())) ||
                                             arleadySelected))
                                    {
                                        pointSelected = true;
                                        selectedPoint = p.side;
                                        selectedNode = &p;
                                        //elem->setSelected(true);
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
                                        (!pointSelected && (((selectedPart != nullptr) ? selectedPart == elem : true) &&
                                                            elem->getRect().contains(EventsHolder.currMousePos()))))
                                {
                                    if(!elem->isSelected())
                                    {
                                        selectedPart = elem;
                                        selectedGroup = group;
                                        selectedPart->changeColor(sf::Color(71, 58, 255));
                                        elem->setSelected(true);
                                    }
                                    partSelected = true;
                                    sf::Vector2f delta = (sf::Vector2f) EventsHolder.getDelta();
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

                                    if(!elem->isSelected() || selectedPart == elem || lastSelected == elem)
                                    {
                                        if(lastSelected != nullptr && lastSelected != selectedPart)
                                        {
                                            //if(!selectedPart->isSelected())
                                            //{
                                            lastSelected->setSelected(false);
                                            // lastSelected->changeColor(sf::Color::Red);
                                            //}
                                        }
                                        lastSelected = selectedPart;
                                    }
                                }
                            }
                    }
                }
            });

            //move rect by arrow
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Up), [this](sf::Event& e)
            {
                if(!ctrlPress && !Exit && !MouseLeftPressed && asset != nullptr)
                    asset->iterateByPart_s2([this](Part& part)
                                            {
                                                auto rec = part.getRect();
                                                rec.top = std::round(rec.top) - 1;
                                                part.setRect(rec);
                                            });
            });
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Down), [this](sf::Event&)
            {
                if(!ctrlPress && !Exit && !MouseLeftPressed && asset != nullptr)
                    asset->iterateByPart_s2([this](Part& part)
                                            {
                                                auto rec = part.getRect();
                                                rec.top = std::round(rec.top) + 1;
                                                part.setRect(rec);
                                            });
            });
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Left), [this](sf::Event&)
            {
                if(!ctrlPress && !Exit && !MouseLeftPressed && asset != nullptr)
                    asset->iterateByPart_s2([this](Part& part)
                                            {
                                                auto rec = part.getRect();
                                                rec.left = std::round(rec.left) - 1;
                                                part.setRect(rec);
                                            });
            });
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Right), [this](sf::Event&)
            {
                if(!ctrlPress && !Exit && !MouseLeftPressed && asset != nullptr)
                    asset->iterateByPart_s2([this](Part& part)
                                            {
                                                auto rec = part.getRect();
                                                rec.left = std::round(rec.left) + 1;
                                                part.setRect(rec);
                                            });
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
            Exit = (ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered());
        }

        void draw(sf::RenderWindow&) override
        {
        }
    };
}
