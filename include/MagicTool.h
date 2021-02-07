#pragma once

#include "Tool.h"
#include "AppColors.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Texture.hpp>
#include "Application.h"

namespace CAE
{
    class MagicTool : public Tool
    {
    private:
        std::vector<cv::Rect> outputRect;
        sf::Image workImage;

        static cv::Mat sfml2opencv(const sf::Texture& tex, bool toBGRA = false, bool fixZeroTransp = false);

        //bool gray; //you should always use grayscale btw
        //bool makeAllBlack;
        // bool useMorph;
        bool ButtonPressed;
        //int thresh;
        //int add;
        //int morph_iteration;
        //int mode;
        sf::IntRect offset;
        //sf::Vector2i kernel_rect;
        History_data* his;
        sf::IntRect cr;
        cv::Mat source_image;
        cv::Mat use_image;
        cv::Mat transform_image;
        cv::Vec4b transp_color;
        cv::Vec4b transp_color2;
        sf::RectangleShape shape;
        std::shared_ptr<Group> group;
        sf::IntRect rect;
        bool once = false;
        bool AnyHovered;

        void assetUpdated() override
        {
            setImage(*asset->getTexture());
            makeTransformImage();
        }

    public:
        MagicTool(EMHolder& m, const sf::Texture& t, sf::RenderWindow& window, bool useFloatMove = false) : Tool(m, t, window),
                                                                                                            ButtonPressed(false),
                                                                                                            AnyHovered{false},
                                                                                                            transp_color()
        {
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineColor(CAE::Colors::OutLine_r);
            shape.setOutlineThickness(1);
        }

        void Enable() override
        {
            EventsHolder["MagicTool"].setEnable(true);
        }

        void Disable() override
        {
            EventsHolder["MagicTool"].setEnable(false);
            ButtonPressed = false;
        }

        void setSelectedGroup(std::shared_ptr<Group> g)
        { group = g; }

        void Init() override
        {
            auto& eManager = EventsHolder.addEM("MagicTool", false);
            eManager.addEvent(MouseEvent::ButtonPressed(sf::Mouse::Left), [this](sf::Event&)
            {
                ButtonPressed = true;
            });
            eManager.addEvent(MouseEvent::ButtonReleased(sf::Mouse::Left), [this](sf::Event&)
            {
                if(once && group != nullptr && !AnyHovered)
                {
                    ButtonPressed = false;
                    once = false;
                    cropSrc(rect, true);
                    for(auto& r : makeBounds())
                        group->addPart(r);
                        //group->getParts().emplace_back(std::make_shared<Part>(r,*his, 0));
                    rect = {};
                    shape.setPosition(sf::Vector2f(0, 0));
                    shape.setSize(sf::Vector2f(0, 0));
                }
            });
            eManager.addEvent(MouseEvent(sf::Event::MouseMoved), [this](sf::Event&)
            {
                if(ButtonPressed && !AnyHovered)
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
            AnyHovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered();
            if(AnyHovered)
            {
                rect = {};
                shape.setPosition(sf::Vector2f(0, 0));
                shape.setSize(sf::Vector2f(0, 0));
                once = ButtonPressed = false;
            }
        }

        void draw(sf::RenderWindow& w) override
        { if(!AnyHovered) w.draw(shape); }

        void makeTransformImage();

        std::vector<sf::FloatRect> makeBounds();

        //void makeBounds(std::vector<sf::FloatRect>& b);
        auto getTransformPreview()
        { return workImage; }

        auto getTransformImage()
        { return transform_image; }

        auto getSource()
        { return source_image; }

        void settingWindow();

        nlohmann::json save2Json();

        void load4Json(const nlohmann::json& j);

        void cropSrc(sf::IntRect crop, bool rebuildSrc = false);

        void setImage(const sf::Texture& t, sf::IntRect crop = {});
    };
}
