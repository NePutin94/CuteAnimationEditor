#pragma once

#include "Group.h"
#include "math.h"
#include "EventHistory.h"
#include <atomic>

namespace CAE
{
    class AnimationAsset : public sf::Sprite
    {
    private:
        std::atomic_int8_t data_ready;

        struct Textures
        {
            std::string path;
            std::vector<std::string> files;
            std::vector<sf::Texture> textures;
        };

        struct _d_data
        {
            _d_data() = default;

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

        void clear()
        {
            groups.clear();
            t_data.clear();
            draw_data = _d_data();
        }

        void loadFromFile();

        static void ParseMultiAsset(const json& j, std::multimap<int, sf::Texture>& con);

        static sf::Image
        CreateMultiTexture(std::multimap<int, sf::Texture>, sf::Vector2u size, sf::Vector2i _padding);

        static sf::Image makeTexture(const json& j, sf::Vector2i padding = {20, 20});

    public:
        using groupIter = std::vector<std::shared_ptr<Group>>::iterator;

        bool saveAsset(std::string_view = "");

        explicit AnimationAsset(std::string_view _path);

        ~AnimationAsset() = default;

        void loadFromJsom(const json& j)
        {
            json _t = j;
            if(_t.is_array())
            {
                is_array = true;
                sf::Vector2u size;
                int sz = _t.size();
                unsigned int widht = 0;
                unsigned int height = 0;
                int i = draw_data.FrameCount;
                int padding = draw_data.padding;
                for(auto& elem : _t)
                {
                    Textures tt;
                    auto dir = elem.at("PathsToTexture").get<std::string>();
                    sf::Vector2u s;
                    tt.path = dir;
                    for(auto& p : elem["Textures"])
                    {
                        sf::Texture t;
                        texturePath += p.get<std::string>();
                        if(!t.loadFromFile(dir + "/" + p.get<std::string>()))
                        {
                            Console::AppLog::addLog("Can't open " + dir + "/" + p.get<std::string>(), Console::error);
                        }
                        tt.files.emplace_back(p.get<std::string>());
                        tt.textures.push_back(t);
                        //textures.emplace(i, t);
                        s.x = std::max(s.x, t.getSize().x);
                        s.y = std::max(s.y, t.getSize().y);
                    }
                    size.x = std::max(s.x, size.x);
                    size.y = std::max(s.y, size.y);
                    widht = std::max((unsigned int) (s.x * elem["Textures"].size()), widht);
                    height = std::max(s.y * sz, height);
                    t_data.emplace(i, tt);
                    ++i;
                }

                draw_data.FrameSize = max(size, draw_data.FrameSize);
                draw_data.TextureSize = sf::Vector2u(widht, height + padding * sz) + draw_data.TextureSize;
                draw_data.FrameCount = i;
                sf::RenderTexture tx;
                tx.create(draw_data.TextureSize.x, draw_data.TextureSize.y);
                int x = 0, y = 0;
                tx.clear(sf::Color::Transparent);
                for(i = 0; i < draw_data.FrameCount; ++i)
                {
                    auto t = t_data[i];
                    for(auto& tex : t.textures)
                    {
                        sf::Sprite s(tex);
                        //auto test2 = it->second;
                        s.setPosition(x, y);
                        tx.draw(s);
                        x += draw_data.FrameSize.x;
                    }
                    x = 0;
                    y += draw_data.FrameSize.y + 20;
                }
                tx.display();
                texture = tx.getTexture();
                setTexture(texture);
            } else
            {
                is_array = false;
                texturePath = j.at("texturePath").get<std::string>();
                if(!texture.loadFromFile(texturePath))
                {
                    Console::AppLog::addLog("Can't load texture", Console::error);
                }
                setTexture(texture);
            }
        }

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
                    groups.back()->getParts().emplace_back(std::make_shared<Part>(r));
                }
                x = 0;
                y += draw_data.FrameSize.y + draw_data.padding;
                groups.back()->setName("#" + std::to_string(++c));
                groups.back()->setVisible(true);
            }
        }

        auto getGroups()
        { return groups; }

        void iterateByPart(std::function<void(Part&)> func)
        {
            for(auto& g : groups)
            {
                if(g->isVisible() && g->isSelected())
                {
                    for(auto& part : *g)
                    {
                        func(*part);
                    }
                }
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

        Memento_data createMemento() const
        {
            Memento_data d;
            d.is_valid = true;
            d.asset_path = assetPath;
            for(auto& g : groups)
            {
                Memento_data::group_data group;
                group.name = g->getName();
                group.group_id = g->getId();
                group.IsSelected = g->isSelected();
                group.isEnable = g->isVisible();
                for(auto& p : *g)
                {
                    Memento_data::group_data::_part_data part;
                    part.IsSelected = p->isSelected();
                    part.part_id = p->getId();
                    part.rect = p->getRect();
                    part.color = p->getColor();
                    group.part_data.emplace_back(part);
                }
                d.groups.emplace_back(group);
            }
            return d;
        }

        bool reinstateMemento(Memento_data mem)
        {
            if(mem.is_valid)
            {
                if(mem.asset_path != assetPath)
                {
                    assetPath = mem.asset_path;
                    //loadFromFile();
                } else
                {
                    groups.clear();
                    for(auto& g : mem.groups)
                    {
                        Group gr(g.name);
                        for(auto& p : g.part_data)
                        {
                            Part _p(p.rect);
                            _p.setId(p.part_id);
                            //_p.changeColor(p.color);
                            _p.setSelected(p.IsSelected);
                            gr.getParts().emplace_back(std::make_shared<Part>(_p));
                        }
                        groups.emplace_back(std::make_shared<Group>(std::move(gr)));
                    }
                }
                return true;
            } else
                return false;
        }

        int getStatus()
        {
            return data_ready.load();
        }

        friend class Application;
    };
}