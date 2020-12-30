#pragma once

#include "MovePen.h"
#include "MoveView.h"
#include "SelectionTool.h"
#include <imgui.h>

namespace CAE
{
    enum class tool_type
    {
        MOVEPART = 2,
        MAGICTOOL = 4,
        SELECTION = 3,
        MOVEVIEW = 1
    };

    class Application;

    class Tools
    {
    private:
        EMHolder& e;
        int corner = 0;
        using container = std::map<tool_type, std::shared_ptr<Tool>>;
        using iterator = container::iterator;
        container tools;
        iterator current_tool;
        using Ptr = std::shared_ptr<AnimationAsset>;

        void toolsPanel();

        void changeTool(tool_type newTool)
        {
            selectedTool = newTool;
            current_tool->second->SetActive(false);
            current_tool = tools.find(newTool);
            current_tool->second->SetActive(true);
        }

        tool_type selectedTool;
    public:
        Tools(Application* app, sf::RenderWindow* w);

        void setCorner(int c)
        { corner = c; }

        void Init()
        {
            for(auto&[_, t] : tools)
                t->Init();
            current_tool = tools.find(tool_type::MOVEVIEW);
            current_tool->second->SetActive(true);
            selectedTool = current_tool->first;
            auto& eManager = e.addEM("Tools", true);
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Num1), [this](sf::Event& e)
            { changeTool(tool_type::MOVEVIEW); });
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Num2), [this](sf::Event& e)
            { changeTool(tool_type::MOVEPART); });
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Num3), [this](sf::Event& e)
            { changeTool(tool_type::SELECTION); });
            eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::Num4), [this](sf::Event& e)
            { changeTool(tool_type::MAGICTOOL); });
        }

        template<class T>
        auto getTool(tool_type type)
        {
            return static_pointer_cast<T>(tools[type]);
        }

        void update();

        void draw(sf::RenderWindow& w);

        bool current_if(tool_type name)
        {
            if(current_tool != tools.end())
                return current_tool->first == name;
            else
                return false;
        }
        //void handleEvent(sf::Event& e);

        void changeCurrAsset(std::shared_ptr<AnimationAsset>);
    };
}