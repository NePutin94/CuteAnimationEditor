#include "../include/Application.h"
#include <imgui-SFML.h>
#include <iomanip>
#include <algorithm>
#include <future>
#include <filesystem>
#include <vector>
#include "../include/myImGui.h"

//#define DEBUG

namespace fs = std::filesystem;

bool operator<(sf::Vector2i f, sf::Vector2i s)
{
    return (f.x < s.x) && (f.y < s.y);
}

bool operator>(sf::Vector2i f, sf::Vector2i s)
{
    return !(f < s);
}

template<class T>
sf::Vector2<T> abs(sf::Vector2<T> value)
{
    return sf::Vector2<T>{std::abs(value.x), std::abs(value.y)};
}

template<class T>
sf::Vector2<T> operator/(sf::Vector2<T> f, sf::Vector2<T> s)
{
    return sf::Vector2<T>{f.x / s.x, f.y / s.y};
}

void CAE::Application::handleEvent(sf::Event& event)
{
    //static bool moving = false;
    //static sf::Vector2f oldPos;
    bool need = false;
    while(window->pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(event);
        if(event.type == sf::Event::Closed)
        {
            state = states::Exit;
            break;
        }
        if(event.type == sf::Event::GainedFocus)
        {
            need = true;
        }
        eventManagers.updateEvent(event, *window);
    }
    eventManagers.updatecurr(*window, view); //it must be performed before all updates for the current frame
    eventManagers.updateInput();
    if(need)
    {
        INPUT input;
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_LMENU;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}

void CAE::Application::draw()
{
    sf::View unScale(view);
    unScale.setSize(unscaleView);
    window->setView(view);
    window->clear();
    window->draw(shape);
    if(currAsset != nullptr)
    {
        //window->setView(view);
        window->draw(*currAsset);
        if(creatorMode)
        {
            for(const auto& elem : *currAsset)
            {
                if(elem->isVisible())
                    for(const auto& part : *elem)
                    {
                        window->draw(part->getVertex());
                        //window->setView(unScale);
                        for(auto& node : part->getNode())
                            window->draw(node);
                        //window->setView(view);
                    }
            }
            //window->draw(sf::Sprite(magicTool.t.getTexture()));
        }
        tools_container.draw(*window);
    }

    Console::AppLog::Draw("LogConsole", &LogConsole);

    if(attTimer.getElapsedTime() < sf::seconds(2) && newMessage)
        showLog(Console::AppLog::lastLog());
    else newMessage = false;

    ImGui::SFML::Render(*window);
    window->display();
}

void CAE::Application::update()
{
    if(attTimer.getElapsedTime() > sf::seconds(2))
    {
        if(Console::AppLog::hasNewLogByTyp(Console::message))
        {
            newMessage = true;
            attTimer.restart();
        }
    }

    if(currAsset != nullptr && creatorMode)
        tools_container.update();
    if(assetChanged)
    {
        assetChanged = false;
        tools_container.changeCurrAsset(currAsset);
    }
    //editorUpdate();
}

void CAE::Application::editorUpdate()
{}

void CAE::Application::loadAssets()
{
    ImGui::BeginChild("Load assets");
    ImGui::Spacing();
    ImGui::Text("Available assets: ");

    ImGui::Image(ico_holder.getTexture(ico_t::Refresh), {32, 32});
    static std::vector<std::string> _files;
    constexpr std::string_view default_path = "./assets";
    if(ImGui::IsItemClicked())
    {
        _files.clear();
        if(fs::exists(default_path))
        {
            for(auto& p : fs::directory_iterator(default_path))
            {
                auto name = p.path().filename().string();
                if(p.path().extension() == ".json")
                    _files.emplace_back(name);
            }
        } else
            Console::AppLog::addLog("Directory .\"assets\" not exist", Console::error);
    }
    {
        static int selected = -1;
        int i = 0;
        ImGui::Separator();
        for(auto& name : _files)
        {
            if(ImGui::Selectable(name.c_str()))
                selected = i;
            ImGui::Spacing();
            if(selected == i)
            {
                ImGui::BeginChild("Note");
                ImGui::OpenPopup("Open");
                if(ImGui::BeginPopupModal("Open", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("%s", (std::string{"Load "} + name).c_str());
                    ImGui::Separator();
                    if(ImGui::Button("Yes", ImVec2(140, 0)))
                    {
                        loadAsset(std::string{default_path} + "/" + name, true);
                        selected = -1;
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if(ImGui::Button("No", ImVec2(140, 0))) selected = -1;
                    ImGui::EndPopup();
                }
                ImGui::EndChild();
            }
            ++i;
        }
    }
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Load from other path: ");
    if(ImGui::Button("Load From"))
    {
        char filename[MAX_PATH];
        OPENFILENAME ofn;
        ZeroMemory(&filename, sizeof(filename));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = ("*json");
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = ("Select a File");
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
        if(GetOpenFileName(&ofn))
            loadAsset(filename, true);
    }
    ImGui::EndChild();
}

void CAE::Application::tapWindow()
{
    auto io = ImGui::GetIO();
    ImVec2 window_pos = ImVec2(io.DisplaySize.x - 1.f - io.DisplaySize.x / 3, io.DisplaySize.y - 1 - window->getSize().y / 3);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 3, io.DisplaySize.y / 3));
    ImGuiWindowFlags window_flags = 0;

    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    if(!ImGui::Begin("TAP window", NULL, window_flags))
    {
        window_pos.y = io.DisplaySize.y - 21;
        ImGui::SetWindowPos(window_pos);
        ImGui::End();
    } else
    {
        ImGui::SetWindowPos(window_pos, ImGuiCond_Always);
        ImGui::BeginChild("Select Animation", ImVec2(150, 0), true);
        for(auto& anim : animPlayer)
            if(ImGui::Selectable(anim.name.c_str()))
                animPlayer.setCurrentAnim(anim);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true); // Leave room for 1 line below us
        ImGui::Text("MyObject:");
        ImGui::Separator();
        static bool antime = true;
        if(animPlayer.hasAnimation())
        {
            if(ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                auto curr = animPlayer.getCurrentAnimation();
                if(ImGui::BeginTabItem("Show Animation"))
                {
                    sf::FloatRect rect = animPlayer.animUpdate(antime ? (1.f / fps) : 0);
                    float d = 150 / rect.height;
                    auto size = sf::Vector2f(rect.width, rect.height);
                    size.x *= d;
                    size.y *= d;
                    ImGui::Text("Size: %.2f, %.2f", size.x, size.y);
                    ImGui::Image(*currAsset->getTexture(), size, rect);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Settings"))
                {
                    ImGui::DragFloat("Change animation speed", &curr->speed, 0.0002, -10, 10);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Info"))
                {
                    ImGui::Text("Number of frames: %f", curr->frameCount);
                    ImGui::Text("Animation name: %s", curr->name.c_str());
                    if(ImGui::TreeNode("frames"))
                    {
                        int id = 1;
                        for(auto& frame : curr->frames)
                            ImGui::Text("#%d: left: %.2f top: %.2f w: %.2f h: %.2f", id++, frame.left, frame.top, frame.width,
                                        frame.height);
                        ImGui::TreePop();
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        } else
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "the animation is not selected");
        ImGui::EndChild();
        if(ImGui::Button("Stop")) antime = false;
        ImGui::SameLine();
        if(ImGui::Button("Play")) antime = true;
        ImGui::SameLine();
        if(ImGui::Button("Parse Asset"))
            animPlayer.parseAnimationAssets(currAsset->groups);
        ImGui::EndGroup();
    }
    ImGui::End();
}

void CAE::Application::viewSettings()
{
    ImGui::BeginChild("Settings");
    if(ImGui::TreeNode("View Settings: "))
    {
        ImGui::Text("View Size: ");
        static float xSize;
        static float ySize;
        xSize = view.getSize().x;
        ySize = view.getSize().y;
        ImGui::DragFloat("Size X: ", &xSize, 1.f);
        ImGui::DragFloat("Size Y:", &ySize, 1.f);
        if(currAsset != nullptr)
            if(ImGui::Button("Set size as the texture"))
            {
                xSize = currAsset->getTexture()->getSize().x;
                ySize = currAsset->getTexture()->getSize().y;
                view.setSize(xSize, ySize);
            }
        if(ImGui::Button("Update Size"))
            view.setSize(xSize, ySize);
        ImGui::Separator();
        ImGui::Text("View Center: ");
        static float x;
        static float y;
        x = view.getCenter().x;
        y = view.getCenter().x;
        ImGui::InputFloat("position X", &x);
        ImGui::InputFloat("position Y", &y);
        if(ImGui::Button("Update Position"))
            view.setCenter(x, y);
        ImGui::Separator();
        tools_container.getTool<MagicTool>(tool_type::MAGICTOOL)->settingWindow();
        ImGui::TreePop();
    }
    ImGui::EndChild();
}

void CAE::Application::createAssets()
{
    ImGui::BeginChild("Create asset");
    ImGui::Text("Create asset: ");
    ImGui::Separator();
    ImGui::Spacing();

    if(ImGui::TreeNode("Name"))
    {
        ImGui::InputText("##Name", buff, MAX_PATH);
        ImGui::TreePop();
    }
    //static
    static bool needUpdate = false;
    static bool makeByGroup = false;
    static bool SingleTex = false;
    static bool MakeTexturesToGroup = false;
    static std::multimap<int, sf::Texture> textures;
    static sf::Vector2u maxFrame;
    static sf::Texture tt;
    static json j;
    static int groupCount = 0;
    if(ImGui::TreeNode("Texture path"))
    {
        ImGui::Checkbox("Make by group", &makeByGroup);
        if(makeByGroup)
        {
            ImGui::Checkbox("Save as single texture", &SingleTex);
            ImGui::Text("group count %i", groupCount);
            ImGui::InputText("##Texture path", buff2, IM_ARRAYSIZE(buff2));
            ImGui::SameLine();
            ImGui::Image(ico_holder.getTexture(ico_t::Folder), sf::Vector2f{27, 18});
            ImGui::Spacing();
            static ImGui::FileManager_Context c("./assets");
            c.setOpen(ImGui::IsItemClicked());

            if(auto val = ImGui::FileManager(c); val.first)
            {
                std::string directory = c.getPath();
                if(groupCount == 0)
                    j["AssetCreate"] = json::array();
                json j_;
                j_["PathsToTexture"] = directory;
                auto& p = j_["Textures"];
                p = json::array();
                int start = 0;
                int end = val.second.find_first_of(';');
                while(end < val.second.length())
                {
                    p.emplace_back(val.second.substr(start, end - start));
                    start = end + 1;
                    end = val.second.find_first_of(';', start);
                }

                j["AssetCreate"].emplace_back(j_);
                AnimationAsset::ParseMultiAsset(j["AssetCreate"], textures);
                for(auto& elem : textures)
                {
                    maxFrame.x = std::max(elem.second.getSize().x, maxFrame.x);
                    maxFrame.y = std::max(elem.second.getSize().y, maxFrame.y);
                }
                strcpy_s(buff2, "");
                ofstream out("./chahe.json");
                out << std::setw(4) << j << std::endl;
                ++groupCount;
                out.close();
            }
        } else
        {
            ImGui::InputText("##Texture path", buff2, IM_ARRAYSIZE(buff2));
            ImGui::SameLine();
            ImGui::Image(ico_holder.getTexture(ico_t::Folder), sf::Vector2f{27, 18});

            if(ImGui::IsItemClicked())
            {
                OPENFILENAME ofn;
                //ZeroMemory(&buff2, sizeof(buff2));
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = NULL;
                ofn.Flags = OFN_EXPLORER;
                ofn.lpstrFilter = (".png");
                ofn.lpstrFile = buff2;
                ofn.nMaxFile = MAX_PATH;
                GetOpenFileName(&ofn);
                //buff
            }
        }
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Output file"))
    {
        ImGui::InputText("##Output file", buff3, IM_ARRAYSIZE(buff3));
        ImGui::SameLine();
        ImGui::Image(ico_holder.getTexture(ico_t::Folder), sf::Vector2f{27, 18});
        if(ImGui::IsItemClicked())
        {
            //char filename[MAX_PATH];
            OPENFILENAME ofn;
            ZeroMemory(&buff3, sizeof(buff3));
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;
            ofn.lpstrFilter = (".json");
            ofn.lpstrFile = buff3;
            ofn.nMaxFile = MAX_PATH;
            GetSaveFileName(&ofn);
            //buff
        }
        ImGui::TreePop();
    }
    if(makeByGroup && SingleTex)
    {
        if(ImGui::TreeNode("Preview"))
        {
            static sf::Vector2i padding = {0, 0};
            bool updated = ImGui::SliderInt("padding.x", &padding.x, 0, 200);
            updated = ImGui::SliderInt("padding.y", &padding.y, 0, 200) || updated;
            if(updated)
            {
                tt.loadFromImage(AnimationAsset::CreateMultiTexture(textures, maxFrame, padding));
            }
            ImGui::Image(tt);
            ImGui::TreePop();
        }
    }
    ImGui::Separator();
    if(ImGui::Button("Create"))
    {
        ofstream o;
        std::string t(buff3);
        if(t.find(".json") == std::string::npos)
            t += ".json";
        o.open(t);
        //json j;
        json _out = AnimationAsset::createEmptyJson();
        auto& defaultInfo = _out["defaultInfo"];
        defaultInfo["name"] = buff;
        if(makeByGroup)
        {
            if(MakeTexturesToGroup)
            {

            }
            if(SingleTex)
            {
                tt.copyToImage().saveToFile(defaultInfo.at("name").get<std::string>() + ".png");
                defaultInfo["texturePath"] = defaultInfo.at("name").get<std::string>() + ".png";
            } else
            {

                json _f;
                ifstream i("../chahe.json");
                i >> _f;
                defaultInfo["texturePath"] = _f["AssetCreate"];
                i.close();
            }
        } else
            defaultInfo["texturePath"] = buff2;
        j.clear();
        groupCount = 0;
        o << std::setw(4) << _out << std::endl;
        o.close();
        clearBuffers();
    }
    ImGui::EndChild();
}

void CAE::Application::editor()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.f);
    ImGui::BeginChild("Editor", ImVec2(window->getSize().x / 3, window->getSize().y / 2), true);
    if(currAsset != nullptr)
    {
        ImGui::Checkbox("Creator mode", &creatorMode);
        ImGui::Text("%s", ("Current asset name: " + currAsset->name).c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("anim groups: ");
        ImGui::Spacing();
        ImGui::SetNextItemWidth(32);

        if(currAsset->groups.empty())
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "groups is empty");
        else
        {
            if(tools_container.current_if(tool_type::MAGICTOOL))
            {
                auto mg = tools_container.getTool<MagicTool>(tool_type::MAGICTOOL);
                static std::string selectedGroup;
                int id = 0;
                for(auto& group : currAsset->groups)
                {
                    ImGui::PushID(++id);

                    if(ImGui::Selectable(group->getName().c_str(), selectedGroup == group->getName()))
                    {
                        selectedGroup = group->getName();
                        mg->setSelectedGroup(group);
                    }

                    ImGui::PopID();
                }
            } else
                editorDragDropLogic();
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::InputText("new group name", buff, IM_ARRAYSIZE(buff));
        if(ImGui::Button("add group"))
        {
            currAsset->groups.emplace_back(std::make_shared<Group>(buff));
            clearBuffers();
        }
        ImGui::PopStyleVar();
        ImGui::PushStyleColor(ImGuiCol_ChildBg,ImVec4(0.07f, 0.07f, 0.09f, 1.00f));
        tapWindow();
        ImGui::PopStyleColor();
    } else
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "The current asset is not selected");
        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
}

void CAE::Application::exit()
{
    ImGui::BeginChild("Note");
    ImGui::OpenPopup("Save?");
    if(ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("SAVE SESSION?");
        ImGui::Separator();

        if(ImGui::Button("Yes", ImVec2(140, 0)))
        {
            saveState();
            if(currAsset != nullptr)
                currAsset->saveAsset();
            window->close();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if(ImGui::Button("Cancel", ImVec2(140, 0)))
        { window->close(); }
        ImGui::EndPopup();
    }
    ImGui::EndChild();
}


void CAE::Application::editorDragDropLogic()
{
    static bool renameState = false;
    static int rename_id = 0;
    static int global_id = 0; //just a local 'name' for part, value used only in make_text
    auto make_text = [](Part& p)->std::string
    {
        return "rect #" + std::to_string(p.getId()) + ": " + "pos.x:" + std::to_string(p.getRect().left) + ", pos.y:" +
               std::to_string(p.getRect().top);
    };
    auto parse_data = [](std::string d)
    {
        std::pair<int, int> p;
        auto middle = d.find_first_of(":");
        p.first = std::stoi(d.substr(0, middle));
        p.second = std::stoi(d.substr(middle + 1, d.length() - middle));
        return p;
    };
    //////////////////Erase Add Buttons//////////////////
    ImGui::Image(ico_holder.getTexture(ico_t::Editor_Delete), sf::Vector2f{32, 32});
    if(ImGui::BeginDragDropTarget())
    {
        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
        {
            //IM_ASSERT(payload->DataSize == sizeof(int));
            const char* payload_n = (const char*) payload->Data;
            auto[group_n, part_n] = parse_data(std::string(payload_n));
            if(group_n == -1) //editorSubArray erase
            {

            } else if(group_n == -2) //Group erase
            {
                selectedGroups.emplace(part_n);
                int _i = 0;
                auto& groups = currAsset->groups;
                groups.erase(std::remove_if(groups.begin(), groups.end(), [](auto g)
                { return g->isSelected(); }), groups.end());
            } else //Part erase
            {
                for(auto& g : *currAsset)
                {
                    if(g->isSelected())
                    {
                        g->getParts().erase(std::remove_if(begin(g->getParts()), end(g->getParts()), [](auto part)
                        {
                            return part->isSelected();
                        }), end(g->getParts()));
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::SameLine();
    ImGui::Image(ico_holder.getTexture(ico_t::Editor_Add), sf::Vector2f{32, 32});
    if(ImGui::IsWindowHovered() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        if(ImGui::GetIO().MouseReleased[0])
        {
            auto pos = sf::Vector2f(currAsset->getPosition());
            editorSubArray.emplace_back(
                    std::make_shared<Part>(
                            sf::FloatRect(pos.x, pos.y, 20, 20), ++global_id
                    ));
        }
    //////////////////END//////////////////
    if(ImGui::TreeNode("Editor storage"))
    {
        int sub_part_id = 0;
        for(auto iter = editorSubArray.begin(); iter != editorSubArray.end(); ++iter)
        {
            ImGui::PushID(sub_part_id);
            auto& part = *iter;
            auto r = part->getRect();
            ImGui::Selectable(make_text(*part).c_str());

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                std::string str = std::to_string(-1) + ":" + std::to_string(sub_part_id);
                const char* s = str.c_str();
                ImGui::SetDragDropPayload("DAD", s, sizeof(s));
                ImGui::Text("Swap %s", make_text(*part).c_str());
                ImGui::EndDragDropSource();
            }
            if(ImGui::BeginDragDropTarget())
            {
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
                {
                    //IM_ASSERT(payload->DataSize == sizeof(int));
                    const char* payload_n = (const char*) payload->Data;
                    auto[group_id, part_id] = parse_data(std::string(payload_n));
                    if(group_id == -1)
                        std::swap(*(editorSubArray.begin() + part_id), *iter);
                    else
                        std::swap(*(currAsset->groups[group_id]->getParts().begin() + part_id), *iter);
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::PopID();
            ++sub_part_id;
        }
        ImGui::TreePop();
    }
    int group_id = 0;
    static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_None;

    for(auto& group : currAsset->groups)
    {
        ImGui::PushID(group_id);
        ImGuiTreeNodeFlags nd = base_flags;
        if(group->isSelected())
            nd |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
        bool node_open = false;
        if((renameState && rename_id != group_id) || !renameState)
            node_open = ImGui::TreeNodeEx((void*) (intptr_t) group_id, nd, "%s", group->getName().c_str());
        if(ImGui::IsItemClicked(1))
        {
            rename_id = group_id;
            renameState = true;
            strcpy_s(buff2, group->getName().c_str());
        }
        if(!renameState)
        {
            if(ImGui::IsItemClicked() && ImGui::GetIO().KeyCtrl)
            {
                if(group->setSelected(!group->isSelected()))
                    selectedGroups.emplace(group_id);
                else
                    selectedGroups.erase(group_id);
            }
            if(node_open)
            {
                //////////////////Drag&Drop group(State: TreeNode open)//////////////////
                if(ImGui::BeginDragDropTarget())
                {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
                    {
                        //IM_ASSERT(payload->DataSize == sizeof(int));
                        const char* payload_n = (const char*) payload->Data;
                        auto[group_n, part_n] = parse_data(std::string(payload_n));
                        if(group_n == -1)
                        {
                            group->getParts().push_back(editorSubArray[part_n]);
                            editorSubArray.erase(editorSubArray.begin() + part_n);
                        } else
                        {
                            auto& g = currAsset->groups[group_n];
                            auto& p = currAsset->groups[group_n]->getParts();

                            selectedParts.emplace(part_n);

                            for(auto& i : selectedParts)
                            {
                                group->getParts().push_back(p[i]);
                            }
                            int _i = 0;
                            for(auto& i : selectedParts)
                            {

                                g->getParts().erase(g->getParts().begin() + (i - _i));
                                ++_i;
                            }
                            //group->getParts().push_back(p[part_n]);
                            //g->getParts().erase(g->getParts().begin() + part_n);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                /////////////////////////////////////////////END/////////////////////////////////////////////////////////
                bool isVisible = group->isVisible();
                ImGui::Checkbox("isVisible", &isVisible);
                group->setVisible(isVisible);

                int part_id = 0;
                bool one_of = false;
                for(auto iter = group->getParts().begin(); iter != group->getParts().end(); ++iter)
                {
                    ImGui::PushID(part_id);
                    auto& part = *iter;
                    auto r = part->getRect();
                    ImGui::Selectable(make_text(*part).c_str(), part->isSelected());

                    if(ImGui::IsItemClicked() && ImGui::GetIO().KeyCtrl)
                    {
                        //Console::AppLog::addLog("Part", Console::info);
                        //part->setSelected(!part->isSelected());
                        if(part->setSelected(!part->isSelected()))
                        {
                            group->setSelected(true);
                            selectedParts.emplace(part_id);
                        } else
                        {
                            selectedParts.erase(part_id);
                        }
                    }
                    if(part->isSelected())
                        one_of = true;
                    //////////////////Drag&Drop parts//////////////////
                    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        std::string str = std::to_string(group_id) + ":" + std::to_string(part_id);
                        const char* s = str.c_str();
                        ImGui::SetDragDropPayload("DAD", s, sizeof(s));
                        ImGui::Text("Swap %s", make_text(*part).c_str());
                        ImGui::EndDragDropSource();
                    }
                    if(ImGui::BeginDragDropTarget())
                    {
                        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
                        {
                            //IM_ASSERT(payload->DataSize == sizeof(int));
                            const char* payload_n = (const char*) payload->Data;
                            auto[group_id, part_id] = parse_data(std::string(payload_n));
                            if(group_id == -1)
                                std::swap(*(editorSubArray.begin() + part_id), *iter);
                            else
                                std::swap(*(currAsset->groups[group_id]->getParts().begin() + part_id), *iter);
                        }
                        ImGui::EndDragDropTarget();
                    }
                    //////////////////END//////////////////
                    ImGui::PopID();
                    ++part_id;
                }
                if(!one_of)
                    group->setSelected(false);
                ImGui::TreePop();
            } else
            {
                //////////////////Drag&Drop group(State: TreeNode closed)//////////////////
                if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    std::string str = std::to_string(-2) + ":" + std::to_string(group_id);
                    group->setSelected(true);
                    const char* s = str.c_str();
                    ImGui::SetDragDropPayload("DAD", s, sizeof(int));
                    ImGui::Text("Swap %s", "node");
                    ImGui::EndDragDropSource();
                }
                if(ImGui::BeginDragDropTarget())
                {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAD"))
                    {
                        //IM_ASSERT(payload->DataSize == sizeof(int));
                        const char* payload_n = (const char*) payload->Data;
                        auto[group_n, part_n] = parse_data(std::string(payload_n));
                        if(group_n == -1)
                        {
                            group->getParts().push_back(editorSubArray[part_n]);
                            editorSubArray.erase(editorSubArray.begin() + part_n);
                        } else
                        {
                            auto& g = currAsset->groups[group_n];
                            auto& p = currAsset->groups[group_n]->getParts();

                            selectedParts.emplace(part_n);

                            for(auto& i : selectedParts)
                            {
                                group->getParts().push_back(p[i]);
                            }
                            int _i = 0;
                            for(auto& i : selectedParts)
                            {

                                g->getParts().erase(g->getParts().begin() + (i - _i));
                                ++_i;
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                //////////////////END//////////////////
            }
        } else if(renameState && rename_id == group_id)
        {
            if(ImGui::InputText("Rename", buff2, 256, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                group->setName(buff2);
                strcpy_s(buff2, "");
                renameState = false;
            }
        }
        ImGui::PopID();
        ++group_id;
    }
}

void CAE::Application::mainWindow()
{
    ImGui::BeginChild("MainWindow");
    if(ImGui::Button("Load Last Session"))
        loadState();
    ImGui::EndChild();
}

void CAE::Application::viewLoadedAssets()
{
    ImGui::BeginChild("Loaded assets");
    ImGui::Text("Loaded assets: ");
    if(animAssets.empty())
        ImGui::Text("Nothing.");
    else
    {
        ImGui::Separator();
        for(auto iter = animAssets.begin(); iter != animAssets.end();)
        {
            auto a = *iter;
            bool erase = false;
            if(ImGui::TreeNode(a->getName().c_str()))
            {
                sf::Vector2f size = (sf::Vector2f) a->getTexture()->getSize();
                ImGui::BeginChild("Image", {window->getSize().x / 3.f, 300.f}, false,
                                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Image(*a, size);
                ImGui::EndChild();
                ImGui::Spacing();
                ImGui::Text("Info: ");
                auto WH = a->getWH();
                ImGui::Text("Texture Size: %.2f %.2f", size.x, size.y);
                ImGui::Text("Texture Path: %s", a->getPath().c_str());
                ImGui::Text("Step on the axis Width: %i Height: %i", WH.first, WH.second);
                if(ImGui::Button("select as current"))
                    currAsset = a;
                if(ImGui::Button("free"))
                    erase = true;
                if(a->is_array && ImGui::Button("Store frames as group"))
                    a->convertFrame2Group();
                ImGui::TreePop();
            }
            ImGui::Separator();
            if(erase)
            {
                if(currAsset == *iter)
                    currAsset.reset();
                iter = animAssets.erase(iter);
            } else
                ++iter;
        }
    }
    ImGui::EndChild();
}

void CAE::Application::saveAsset()
{
    ImGui::BeginChild("Save asset");
    ImGui::Text("Save current asset");
    if(currAsset != nullptr)
    {
        //ImGui::InputText("File Path", buff, IM_ARRAYSIZE(buff));
        if(ImGui::Button("Save File as"))
        {
            char filename[MAX_PATH];
            OPENFILENAME ofn;
            ZeroMemory(&filename, sizeof(filename));
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;
            ofn.lpstrFilter = (".json");
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            if(GetSaveFileName(&ofn))
                currAsset->saveAsset(std::string(filename) + ".json");
            //clearBuffers();
        }
    } else
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "The current asset is not selected");
    ImGui::EndChild();
}

void CAE::Application::drawMenuBar()
{
    if(ImGui::BeginMenuBar())
    {
        if(ImGui::BeginMenu("Assets"))
        {
            if(ImGui::MenuItem("Main Window", NULL, state == states::Null))
                state = states::Null;
            if(ImGui::MenuItem("Load Asset", NULL, state == states::LoadAsset))
                state = states::LoadAsset;
            if(ImGui::MenuItem("New", NULL, state == states::CreateAsset))
                state = states::CreateAsset;
            if(ImGui::MenuItem("Save Asset", NULL, state == states::SaveAsset))
                state = states::SaveAsset;
            if(ImGui::MenuItem("Edit Asset", NULL, state == states::Editor))
                state = states::Editor;
            if(ImGui::MenuItem("Loaded Assets", NULL, state == states::loadedAssets))
                state = states::loadedAssets;
            if(ImGui::MenuItem("Magic Tool", NULL, state == states::macgicTool))
                state = states::macgicTool;
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Settings"))
        {
            if(ImGui::MenuItem("Window settings", "", state == states::WindowSettings))
                state = states::WindowSettings;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void CAE::Application::drawUI()
{
    drawMenuBar();
    switch(state)
    {
        case CAE::Application::states::Exit:
            exit();
            break;
        case CAE::Application::states::Null:
            mainWindow();
            break;
        case CAE::Application::states::macgicTool:
            magicSelection();
            break;
        case CAE::Application::states::CreateAsset:
            createAssets();
            break;
        case CAE::Application::states::LoadAsset:
            loadAssets();
            break;
        case CAE::Application::states::SaveAsset:
            saveAsset();
            break;
        case CAE::Application::states::Editor:
            editor();
            break;
        case CAE::Application::states::loadedAssets:
            viewLoadedAssets();
            break;
        case CAE::Application::states::WindowSettings:
            viewSettings();
            break;
        default:
            break;
    }
}

void CAE::Application::addEventsHandler()
{
    EventManager& eManager = eventManagers.addEM("MainManager");
    eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::F1), &Application::changeMovingMode_e, this);
    //eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::F2), &Application::useMouseToMove_e, this);
    eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::F2), [this](sf::Event&)
    {
        this->creatorMode = !this->creatorMode;
        Console::AppLog::addLog(std::string("Change mode: ") + (this->creatorMode ? "true" : "false"), Console::message);
    });
    eManager.addEvent(KBoardEvent::KeyReleased(sf::Keyboard::Tilde), [this](sf::Event&)
    { LogConsole = !LogConsole; });
}

CAE::Application::Application(sf::RenderWindow& w)
        : tools_container(this, &w), state(states::Null), window(&w),
          scaleFactor(1.5f),
          nodeSize(5), scaleSign(0), eventManagers(),
          useFloat(true), newMessage(false)
{
    setTheme();
    addEventsHandler();
    view.setSize(w.getDefaultView().getSize());
    scaleView = unscaleView = view.getSize();
    tools_container.Init();
}

void CAE::Application::clearBuffers()
{
    strcpy_s(buff, "");
    strcpy_s(buff2, "");
    strcpy_s(buff3, "");
}

void CAE::Application::loadState()
{
    if(ifstream open("config.json"); open.is_open())
    {
        json j;
        open >> j;
        try
        {
            auto& appSettings = j.at("Application Settings");
            if(appSettings.find("currentAsset") != appSettings.end())
            {
                std::string p = appSettings.at("currentAsset").get<std::string>();
                loadAsset(p, true);
            }
            creatorMode = appSettings.at("creatorMode").get<bool>();
            tools_container.getTool<MagicTool>(tool_type::MAGICTOOL)->load4Json(j.at("MagicTool Settings"));
        }
        catch (json::exception& e)
        {
            std::string str = e.what();
            Console::AppLog::addLog("Json throw exception, message: " + str, Console::error);
        }
        open.close();
    } else
    {
        Console::AppLog::addLog("File config.json can't be opened!", Console::system);
        saveState();
    }
}

void CAE::Application::saveState()
{
    ofstream open("config.json");
    json j;
    auto& applicationMain = j["Application Settings"];
    if(currAsset != nullptr)
    {
        applicationMain["currentAsset"] = currAsset->assetPath;
    }
    applicationMain["creatorMode"] = creatorMode;
    j["MagicTool Settings"] = tools_container.getTool<MagicTool>(tool_type::MAGICTOOL)->save2Json();
    open << std::setw(4) << j;
    open.close();
}

void CAE::Application::loadAsset(std::string path, bool setAsCurr)
{
    if(!path.empty())
    {
        auto task = [this, setAsCurr](std::string copyBuffer)
        {
            sf::Context _;
            auto ptr = std::make_shared<CAE::AnimationAsset>(copyBuffer);
            if(ptr->loadFromFile())
            {
                if(auto it = find_if(animAssets.begin(), animAssets.end(), [ptr](auto val)
                    {
                        return ptr->getName() == val->getName();
                    }); it != animAssets.end())
                {
                    Console::AppLog::addLog("Assets loaded arleady", Console::message);
                    //delete ptr;
                } else
                    animAssets.emplace_back(ptr);
                if(setAsCurr)
                {
                    currAsset = *animAssets.begin();
                    assetChanged = true;
                }
            }
            //else
            //	delete ptr;
        };
        Console::AppLog::addLog("Loading asset from: " + path, Console::message);
        //task(path);
        std::thread(task, path).detach();
        //  std::thread(task, path).detach();
        clearBuffers();
    } else
        Console::AppLog::addLog("File does not exist or input is empty!", Console::message);
}

void CAE::Application::changeMovingMode_e(sf::Event&)
{
    useFloat = !useFloat;
    Console::AppLog::addLog(std::string("Change moving mode, floating: ") + (useFloat ? "true" : "false"), Console::message);
}

void CAE::Application::magicSelection()
{
    tools_container.getTool<MagicTool>(tool_type::MAGICTOOL)->settingWindow();
}

void CAE::Application::showLog(std::string_view txt)
{
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoInputs;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
    auto io = ImGui::GetIO();
    ImGui::Begin("Console Message", 0, window_flags);
    ImGui::Text("%s", txt.data());
    ImVec2 window_pos = ImVec2(3.f, io.DisplaySize.y - ImGui::GetWindowHeight() - 3.f);
    ImGui::SetWindowPos(window_pos);
    ImGui::End();
}

void CAE::Application::viewUpdated()
{}

void CAE::Application::start()
{
    sf::Clock clock;
    sf::Time previousTime = clock.getElapsedTime();
    sf::Time currentTime;

    while(window->isOpen())
    {
        sf::Event event;
        handleEvent(event);
        ImGui::SFML::Update(*window, deltaClock.restart());

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_MenuBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoDecoration;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        auto io = ImGui::GetIO();
        ImGui::SetNextWindowSize(ImVec2(window->getSize().x / 3, window->getSize().y - 1.f));
        ImGui::SetNextWindowBgAlpha(bg_alpha / 10.f);
        if(corner != -1)
        {
            float DISTANCE = .2f;
            window_flags |= ImGuiWindowFlags_NoMove;
            ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE,
                                       (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
            ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        }
        if(ImGui::Begin("Tools Window", NULL, window_flags))
        {
            if(ImGui::BeginPopupContextWindow())
            {
                if(ImGui::MenuItem("Left", NULL, corner == 0))
                {
                    corner = 0;
                    tools_container.setCorner(1);
                }
                if(ImGui::MenuItem("Right", NULL, corner == 1))
                {
                    corner = 1;
                    tools_container.setCorner(0);
                }
                ImGui::DragInt("Alpha", &bg_alpha, 1, 0, 10);
                ImGui::EndPopup();
            }
            drawUI();
            toolsWindowFocused = ImGui::IsAnyWindowHovered();
            ImGui::End();
        }
        currentTime = clock.getElapsedTime();
        fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds()); // the asSeconds returns a float
        previousTime = currentTime;
        //ImGui::ShowDemoWindow();
#ifdef DEBUG
        ImGui::Begin("Debug", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("fps: %.2f", fps);
        ImGui::Text("Mouse pos x: %f, y: %f", eventManagers.currMousePos().x, eventManagers.currMousePos().y);
        //ImGui::Text("Mouse delta x: %f", eManager.worldMouseDelta());
        ImGui::Separator();
        ImGui::Text("Delta clock: %f", this->deltaClock.getElapsedTime().asSeconds());
        ImGui::Text("View pos %f %f", this->view.getCenter().x, this->view.getCenter().y);
        ImGui::Text("CreatorMode: %i", this->creatorMode);
        ImGui::Text("toolsWindowFocused: %i", this->toolsWindowFocused);
        ImGui::Text("LogConsole: %i", this->LogConsole);
        ImGui::Separator();
        ImGui::Text("State: %i", this->state);
        ImGui::Text("NodeSize: %f", this->nodeSize);
        ImGui::Separator();
        ImGui::Text("Buffers: ");
        ImGui::Text("buff: %s", this->buff);
        ImGui::Text("buff2: %s", this->buff2);
        ImGui::Text("buff3: %s", this->buff3);
        ImGui::End();
#endif // DEBUG
        update();
        draw();
    }
}

void CAE::Application::setTheme()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    style->WindowPadding = ImVec2(10, 9);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(3, 5);
    style->FrameRounding = 1.0f;
    style->ItemSpacing = ImVec2(10, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 12.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 6.0f;
    style->GrabRounding = 3.0f;
    style->AntiAliasedLinesUseTex = false;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.117f, 0.098f, 0.152f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.215f, 0.192f, 0.337f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.117f, 0.074f, 0.207f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.117f, 0.074f, 0.207f, 1.00f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.127f, 0.08f, 0.3f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.211f, 0.133f, 0.368f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.831f, 0.286f, 1.f, 0.58f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.137f, 0.086f, 0.235f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2f, 0.1f, 0.3f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.11f, 0.45f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.188f, 0.043f, 0.274f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.325f, 0.011f, 0.447f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.266f, 0.098f, 0.87f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.172f, 0.f, 1.f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}
