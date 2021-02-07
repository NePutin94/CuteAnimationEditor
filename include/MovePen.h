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
        sf::RectangleShape shape;
        sf::IntRect rect;
        ScaleNode* selectedNode;
        bool& useFloat;
        bool once;
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
                                                                                                   ctrlPress(false), altPress(false),
                                                                                                   once(false), partSelected(false)
        {
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineColor(CAE::Colors::OutLine_r);
            shape.setOutlineThickness(1);
        }

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
                altPress = (MouseLeftPressed) ? true : false;
            });
            eManager.addEvent(MouseEvent::ButtonPressed(sf::Mouse::Left), [this](sf::Event& event)
            {
                MouseLeftPressed = true;
            });
            eManager.addEvent(MouseEvent::ButtonReleased(sf::Mouse::Left), [this](sf::Event& event)
            {
                if(partSelected || pointSelected)
                {
                    History_data::NeedSnapshot();
                }
                MouseLeftPressed = false;
                pointSelected = false;
                selectedNode = nullptr;
                selectedPoint = -1;
                partSelected = false;
                once = false;
                if(ctrlPress)
                {
                    if(!Exit && !MouseLeftPressed && asset != nullptr)
                        asset->iterateByPart_s1([this](Part& part)
                                                {
                                                    if(part.isSelected() && part.getRect().contains(EventsHolder.currMousePos()))
                                                        part.setSelected(false);
                                                });

                }
                if(altPress)
                {
                    bool ok = false;
                    if(abs(rect.left) - abs(rect.width) > 0 && abs(rect.top) - abs(rect.height))
                    {
                        for(auto group : *asset)
                            if(group->isSelected())
                            {
                                group->addPart(rect);
                                ok = true;
                                break;
                            }
                    }
                    if(!ok)
                        Console::AppLog::addLog("Select the group where the rectangle will be added", Console::message);
                    rect = {};
                    shape.setPosition(sf::Vector2f(0, 0));
                    shape.setSize(sf::Vector2f(0, 0));
                    altPress = false;
                }
            });

            //move rect and scale node by mouse
            eManager.addEvent(MouseEvent(sf::Event::MouseMoved), [this](sf::Event& event)
            {
                //bool endNodeUpdate = false;
                if(!Exit && MouseLeftPressed && asset != nullptr)
                {
                    if(altPress)
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
                    } else
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
                                            auto elem_rect = elem->getRect();
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
                                                elem_rect = round(elem_rect);
                                            } else
                                                value = -delta;

                                            switch(p.side)
                                            {
                                                case 0:
                                                    elem->setRect(
                                                            sf::FloatRect(elem_rect.left, elem_rect.top + value.y, elem_rect.width,
                                                                          elem_rect.height - value.y));
                                                    break;
                                                case 1:
                                                    elem->setRect(sf::FloatRect(elem_rect.left, elem_rect.top, elem_rect.width + value.x,
                                                                                elem_rect.height));
                                                    break;
                                                case 2:
                                                    elem->setRect(sf::FloatRect(elem_rect.left, elem_rect.top, elem_rect.width,
                                                                                elem_rect.height + value.y));
                                                    break;
                                                case 3:
                                                    elem->setRect(
                                                            sf::FloatRect(elem_rect.left + value.x, elem_rect.top,
                                                                          elem_rect.width - value.x,
                                                                          elem_rect.height));
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
            if(ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered())
            {
                MouseLeftPressed = false;
            }
        }

        void draw(sf::RenderWindow& w) override
        {
            w.draw(shape);
        }
    };
}
