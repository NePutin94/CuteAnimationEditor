#include "../include/AnimationAsset.h"
#include <iomanip>


bool CAE::AnimationAsset::loadFromFile()
{
    if(std::ifstream i(assetPath.data(), ifstream::in); i.is_open())
    {
        json j;
        i >> j;
        try
        {
            auto info = j.at("defaultInfo");
            name = info.at("name").get<std::string>();
            if(info.at("texturePath").is_array())
            {
                is_array = true;
                json _t = j["defaultInfo"]["texturePath"];
                if(_t.is_array())
                {
                    std::multimap<int, sf::Texture> textures;
                    sf::Vector2u size;
                    int sz = _t.size();
                    unsigned int widht = 0;
                    unsigned int height = 0;
                    int i = 0;
                    int padding = 20;
                    draw_data.padding = padding;
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
                                return false;
                            }
                            tt.files.emplace_back(p.get<std::string>());
                            tt.textures.push_back(t);
                            textures.emplace(i, t);
                            s.x = std::max(s.x, t.getSize().x);
                            s.y = std::max(s.y, t.getSize().y);
                        }
                        size.x = std::max(s.x, size.x);
                        size.y = std::max(s.y, size.y);
                        widht = std::max((unsigned int) (s.x * elem["Textures"].size()), widht);
                        height = std::max(s.y * sz, height);
                        ++i;
                        t_data.emplace(i, tt);
                    }
                    sf::RenderTexture tx;
                    tx.create(widht, height + padding * sz);
                    int x = 0, y = 0;
                    tx.clear(sf::Color::Transparent);
                    draw_data.FrameSize = size;
                    draw_data.TextureSize = tx.getSize();
                    //draw_data.FrameCount
                    for(i = 0; i < sz; ++i)
                    {
                        auto range = textures.equal_range(i);
                        for(auto it = range.first; it != range.second; ++it)
                        {
                            sf::Sprite s(it->second);
                            s.setPosition(x, y);
                            tx.draw(s);
                            x += size.x;
                        }
                        x = 0;
                        y += size.y + 20;
                    }
                    tx.display();
                    texture = tx.getTexture();
                    setTexture(texture);
                }
            } else
            {
                is_array = false;
                texturePath = info.at("texturePath").get<std::string>();
                if(!texture.loadFromFile(texturePath))
                {
                    Console::AppLog::addLog("Can't load texture", Console::error);
                    return false;
                }
                setTexture(texture);
            }
            for(auto& group : j.at("Groups"))
            {
                groups.emplace_back(std::make_shared<Group>(Group()));
                groups.back()->load(group);
            }
            return true;
        }
        catch (json::exception& e)
        {
            Console::AppLog::addLog("Json throw exception, message: " + std::string(e.what()), Console::error);
        }
    }
    return false;
}

bool CAE::AnimationAsset::saveAsset(std::string_view path)
{
    if(path.empty())
        path = assetPath;
    ofstream o(path.data());
    json j;
    auto& info = j["defaultInfo"];
    info["name"] = name;
    if(is_array)
    {
        info["texturePath"] = json::array();
        for(auto& p : t_data)
        {
            json j;
            j["PathsToTexture"] = p.second.path;
            j["Textures"] = json::array();
            for(auto& pp : p.second.files)
            {
                j["Textures"].emplace_back(pp);
            }
            info["texturePath"].emplace_back(j);
        }
    } else
        info["texturePath"] = texturePath;

    int i = 0;
    auto& g = j["Groups"];
    for(auto& group : groups)
    {
        group->save(g[i]);
        ++i;
    }
    o << std::setw(4) << j;
    o.close();
    return true;
}

CAE::AnimationAsset::AnimationAsset(std::string_view _path) : assetPath(_path)
{}

sf::Image CAE::AnimationAsset::makeTexture(const json& j, sf::Vector2i _padding)
{
    if(j.is_array())
    {
        std::multimap<int, sf::Texture> textures;
        sf::Vector2u size;
        int sz = j.size();
        unsigned int widht = 0;
        unsigned int height = 0;
        int max = 0;
        int i = 0;
        int _i = 0;
        sf::Vector2i padding = _padding;
        for(auto& elem : j)
        {
            Textures tt;
            auto dir = elem.at("PathsToTexture").get<std::string>();
            sf::Vector2u s;
            tt.path = dir;
            for(auto& p : elem["Textures"])
            {
                sf::Texture t;
                if(!t.loadFromFile(dir + "/" + p.get<std::string>()))
                {
                    Console::AppLog::addLog("Can't open " + dir + "/" + p.get<std::string>(), Console::error);
                    return sf::Image();
                }
                tt.files.emplace_back(p.get<std::string>());
                textures.emplace(i, t);
                s.x = std::max(s.x, t.getSize().x);
                s.y = std::max(s.y, t.getSize().y);
                _i++;
            }
            max = std::max(max, _i);
            size.x = std::max(s.x, size.x);
            size.y = std::max(s.y, size.y);
            widht = std::max((unsigned int) (s.x * elem["Textures"].size()), widht);
            height = std::max(s.y * sz, height);
            ++i;
            // textures.emplace_back(tt);
        }
        sf::RenderTexture tx;
        tx.create(widht + padding.x * max, height + padding.y * sz);
        int x = 0, y = 0;
        tx.clear(sf::Color::Transparent);
        for(i = 0; i < sz; ++i)
        {
            auto range = textures.equal_range(i);
            for(auto it = range.first; it != range.second; ++it)
            {
                sf::Sprite s(it->second);
                s.setPosition(x, y);
                tx.draw(s);
                x += size.x + padding.x;
            }
            x = 0;
            y += size.y + padding.y;
        }
        tx.display();
        return tx.getTexture().copyToImage();
    } else
        Console::AppLog::addLog("AnimAsset::maketexture expected array", Console::logType::error);
}

void CAE::AnimationAsset::ParseMultiAsset(const json& j, std::multimap<int, sf::Texture>& textures)
{
    if(j.is_array())
    {
        //std::multimap<int, sf::Texture> textures;
        sf::Vector2u size;
        int sz = j.size();
        unsigned int widht = 0;
        unsigned int height = 0;
        int i = 0;
        for(auto& elem : textures)
            i = std::max(elem.first + 1, i);
        for(auto& elem : j)
        {
            Textures tt;
            auto dir = elem.at("PathsToTexture").get<std::string>();
            sf::Vector2u s;
            tt.path = dir;
            for(auto& p : elem["Textures"])
            {
                sf::Texture t;
                if(!t.loadFromFile(dir + "/" + p.get<std::string>()))
                {
                    Console::AppLog::addLog("Can't open " + dir + "/" + p.get<std::string>(), Console::error);
                    return;
                }
                tt.files.emplace_back(p.get<std::string>());
                textures.emplace(i, t);
                s.x = std::max(s.x, t.getSize().x);
                s.y = std::max(s.y, t.getSize().y);
            }
            size.x = std::max(s.x, size.x);
            size.y = std::max(s.y, size.y);
            widht = std::max((unsigned int) (s.x * elem["Textures"].size()), widht);
            height = std::max(s.y * sz, height);
            ++i;
            // textures.emplace_back(tt);
        }
    }
}
//
//std::vector<CAE::Group>
//CAE::AnimationAsset::createGroups(std::multimap<int, sf::Texture> textures, sf::Vector2u FrameSize, sf::Vector2i padding)
//{
//    int i = 0;
//    std::map<int, int> keysCount;
//    std::vector<Group> groups;
//    for(auto& elem : textures)
//        keysCount[elem.first]++;
//    sf::Vector2i countXY = {std::max_element(keysCount.begin(), keysCount.end())->second, static_cast<int>(keysCount.size()) + 1};
//    unsigned int widht = (FrameSize.x + padding.x) * countXY.x;
//    unsigned int height = (FrameSize.y + padding.y) * countXY.y;
//    int sz = textures.size();
//    int x = 0, y = 0;
//    groups.resize(keysCount.size());
//    for(auto& elem : textures)
//    {
//        if(i != elem.first)
//        {
//            i = elem.first;
//            groups[i].setName(std::to_string(i));
//            ImGui::Text("%s", groups[i].getName().c_str());
//            x = 0;
//            y += FrameSize.y + padding.y;
//        }
//        x += FrameSize.x + padding.x;
//        auto size = elem.second.getSize();
//        groups[i].getParts().emplace_back(std::make_shared<Part>(sf::FloatRect(x, y, size.x, size.y)));
//    }
//}

sf::Image
CAE::AnimationAsset::CreateMultiTexture(std::multimap<int, sf::Texture> textures, sf::Vector2u FrameSize, sf::Vector2i padding)
{
    std::map<int, int> keysCount;
    for(auto& elem : textures)
        keysCount[elem.first]++;
    sf::Vector2i countXY = {std::max_element(keysCount.begin(), keysCount.end())->second, static_cast<int>(keysCount.size()) + 1};
    unsigned int widht = (FrameSize.x + padding.x) * countXY.x;
    unsigned int height = (FrameSize.y + padding.y) * countXY.y;
    int sz = textures.size();
    sf::RenderTexture tx;
    tx.create(widht, height);
    int x = 0, y = 0;
    tx.clear(sf::Color::Transparent);
    for(int i = 0; i < sz; ++i)
    {
        auto range = textures.equal_range(i);
        for(auto it = range.first; it != range.second; ++it)
        {
            sf::Sprite s(it->second);
            s.setPosition(x, y);
            tx.draw(s);
            x += FrameSize.x + padding.x;
        }
        x = 0;
        y += FrameSize.y + padding.y;
    }
    tx.display();
    return tx.getTexture().copyToImage();
}

void CAE::AnimationAsset::editAsset()
{
    ImGui::BeginChild("Asset edit");
    ImGui::EndChild();
}
