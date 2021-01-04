#pragma once

#include "Group.h"

namespace CAE
{
    class AnimationAsset : public sf::Sprite
    {
    private:
        struct Textures
        {
            std::string path;
            std::vector<std::string> files;
            std::vector<sf::Texture> textures;
        };
        struct
        {
            int padding;
            size_t FrameCount;
            sf::Vector2u TextureSize;
            sf::Vector2u FrameSize;
        } draw_data;
        bool is_array = false;
        std::map<int, Textures> t_data;
        std::string texturePath;
        std::string assetPath;
        std::string name;
        sf::Texture texture;
        std::vector<std::shared_ptr<Group>> groups;

        bool loadFromFile();

        bool saveAsset(std::string_view = "");

        static void ParseMultiAsset(const json& j, std::multimap<int, sf::Texture>& con);

        static sf::Image
        CreateMultiTexture(std::multimap<int, sf::Texture>, sf::Vector2u size, sf::Vector2i _padding);

        static sf::Image makeTexture(const json& j, sf::Vector2i padding = {20, 20});

    public:
        using groupIter = std::vector<std::shared_ptr<Group>>::iterator;

        explicit AnimationAsset(std::string_view _path);

        ~AnimationAsset() = default;

        auto begin()
        { return groups.begin(); }

        auto end()
        { return groups.end(); }

        std::string getName() const
        { return name; }

        std::string getPath() const
        { return texturePath; }

        auto getWH()
        { return std::make_pair(texture.getSize().x, texture.getSize().y); }

        void editAsset();

        static json createEmptyJson()
        {
            json j;
            auto& info = j["defaultInfo"];
            info["name"] = "";
            info["texturePath"] = "";
            j["Groups"] = json::array();
            return j;
        }

        void convertFrame2Group()
        {
            int x = 0, y = 0;
            int c = 0;
            for(auto& f : t_data)
            {
                groups.emplace_back(std::make_shared<Group>(Group()));
                for(auto& t : f.second.textures)
                {
                    sf::FloatRect r{};
                    r.left = x;
                    r.top = y;
                    r.width = t.getSize().x;
                    r.height = t.getSize().y;
                    x += r.width;
                    groups.back()->addPart(r);
                }
                x = 0;
                y += draw_data.FrameSize.y + draw_data.padding;
                groups.back()->setName("#" + std::to_string(++c));
                groups.back()->setVisible(true);
            }
        }

        void iterateByPart_s1(std::function<void(Part&)> func)
        {
            for(auto& g : groups)
            {
                if(g->isVisible() && g->isSelected())
                {
                    for(auto& part : *g)
                    {
                        if(part->isSelected())
                            func(*part);
                    }
                }
            }
        }

       void iterateByPart_s2(std::function<void(Part&)> func)
        {
            for(auto& g : groups)
            {
                if(g->isVisible())
                {
                    for(auto& part : *g)
                    {
                        if(part->isSelected())
                            func(*part);
                    }
                }
            }
        }

        void iterateByPart_us(std::function<void(Part&)> func)
        {
            for(auto& g : groups)
            {
                if(g->isVisible() && !g->isSelected())
                {
                    for(auto& part : *g)
                    {
                        if(!part->isSelected())
                            func(*part);
                    }
                }
            }
        }
        friend class Application;
    };
}