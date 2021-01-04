#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <filesystem>
#include <imgui.h>
#include <imgui-SFML.h>
#include <set>
#include <cassert>
#include <cstring>
#include "../include/IcoHolder.h"
#include "../include/AppColors.h"
#include <regex>
#include "../include/myImGui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

using namespace ImGui;
namespace fs = std::filesystem;
short ImGui::FileManager_Context::id = 0;

ImTextureID convertGLTextureHandleToImTextureID(GLuint glTextureHandle)
{
    auto textureID = (ImTextureID) NULL;
    std::memcpy(&textureID, &glTextureHandle, sizeof(GLuint));
    return textureID;
}

void ImGui::Image(const sf::Texture& texture, const sf::Vector2f& size, const sf::Vector2f& uv0, const sf::Vector2f& uv1,
                  const sf::Color& tintColor, const sf::Color& borderColor)
{
    ImTextureID textureID =
            convertGLTextureHandleToImTextureID(texture.getNativeHandle());
    ImGui::Image(textureID, size, uv0, uv1, tintColor,
                 borderColor);
}

bool ImGui::SelectableImage(const sf::Texture& texture, bool selected, const ImVec2& size)
{
    int frame_padding = 1;
    //ImGui::Selectable
    ImVec4 bg_col = sf::Color::Transparent;
    ImVec4 tint_col = CAE::Colors::Tool_us;

    sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());
    sf::FloatRect textureRect = {0, 0, textureSize.x, textureSize.y};

    ImVec2 uv0(textureRect.left / textureSize.x,
               textureRect.top / textureSize.y);
    ImVec2 uv1((textureRect.left + textureRect.width) / textureSize.x,
               (textureRect.top + textureRect.height) / textureSize.y);

    ImTextureID textureID = convertGLTextureHandleToImTextureID(texture.getNativeHandle());
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if(window->SkipItems)
        return false;

    // Default to using texture ID as ID. User can still push string/integer prefixes.
    PushID((void*) (intptr_t) textureID);
    const ImGuiID id = window->GetID("#image");
    PopID();

    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float) frame_padding, (float) frame_padding) : g.Style.FramePadding;
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
    ItemSize(bb);
    if(!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

    if(selected)
    {
        //RenderNavHighlight(bb, id);
        //RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
        tint_col =  CAE::Colors::Tool_s;
    }
    if(hovered)
        tint_col = CAE::Colors::Tool_h;

    if(bg_col.w > 0.0f)
        window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, GetColorU32(bg_col));
    window->DrawList->AddImage(textureID, bb.Min + padding, bb.Max - padding, uv0, uv1, GetColorU32(tint_col));
    return pressed;

    //ImGui::ImageButton(textureID, size, uv0, uv1, 1, sf::Color::Black, sf::Color::White);

    //ImGui::ImageButton(texture, { 32,32 });
}

std::pair<bool, std::string> ImGui::FileManager(FileManager_Context& context)
{
    if(context.open)
    {
        ImGui::Begin(("File Manager ##" + std::to_string(context.curr_id)).c_str(), &context.open);
        ImVec2 os = ImGui::GetCursorScreenPos();
        ImVec2 origin = ImGui::GetCursorPos();
        if(context.updateDir)
        {
            fs::path p = context._path;
            context.files.clear();
            if(fs::exists(p))
            {
                for(const auto& fyles : fs::directory_iterator(p))
                    context.files.emplace(fs::path(fyles).generic_string().length(), fyles);
            }
            context.updateDir = false;
        }
        //------selected area------//
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        int _s = ImGui::GetCursorPosY();
        ImVec2 upPanelSize = {0, 64};
        if(context.updateDir)
        {
            fs::path p = context._path;
            if(fs::exists(p))
            {
                for(const auto& fyles : fs::directory_iterator(p))
                    context.files.emplace(fs::path(fyles).generic_string().length(), fyles);
            } else
                context.files.clear();
        }
        ImGui::BeginChild("##Child1", upPanelSize);
        {
            char buffer[256];
            strcpy_s(buffer, context._path.c_str());

            if(ImGui::InputText("Path", buffer, 256))
            {
                context.updateDir = true;
                context._path = buffer;
            }
            if(context.Input)
            {
                context.Input = false;
                ImGui::SetKeyboardFocusHere(0);
            }
        }
        ImGui::Spacing();
        ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Arrow), sf::Vector2f(32, 32), sf::Color(106, 179, 204));
        if(ImGui::IsItemClicked())
        {
            auto s = context._path.rfind('/') + 1;
            auto e = context._path.length();
            context._path.erase(s, e - s);//, s.length();
            context.selected_nodes.clear();
            context.updateDir = true;
        }
        ImGui::SameLine();
        //ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Arrow), sf::Vector2f(32, 32), sf::Vector2f(1, 0), sf::Vector2f(0, 1), sf::Color(106, 179, 204));
        ImGui::EndChild();
        ImGui::PopStyleVar();
        os += upPanelSize;
        const int UpPadding = 20;
        float x = 0.f;
        float y = 0.f;
        const int w = 160;
        int h = 120; //115
        const int XPadding = 10;
        const int YPadding = 10;
        h += UpPadding;
        ImGui::BeginChild("##Child2");
        int i = 0;
        int z = 0;

        /*if (fs::exists(p))*/
        //for (const auto& fyles : fs::directory_iterator(p))
        for(const auto& fyles : context.files)
        {
            ImGui::PushID(++i);
            ++z;
            bool out_hovered;
            bool is_regular = fs::is_regular_file(fyles.second);
            ImVec2 mousePos = ImGui::GetMousePos();
            mousePos.x -= os.x;
            mousePos.y -= os.y;

            out_hovered = (mousePos.x > x && mousePos.x < x + w && mousePos.y > y + UpPadding - ImGui::GetScrollY() &&
                           mousePos.y < y + h - ImGui::GetScrollY());

            //---- Selection
            if(context.selected_nodes.size() <= 1 && context.selected_nodes.find(i) != context.selected_nodes.end())
            {
                ImU32 col = ImColor(84, 175, 174);
                x += os.x;
                y += os.y;
                ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true,
                                   4.f);
                x -= os.x;
                y -= os.y;
            } else if(context.selected_nodes.find(i) != context.selected_nodes.end())
            {
                ImU32 col = ImColor(78, 108, 235);
                x += os.x;
                y += os.y;
                ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true,
                                   4.f);
                x -= os.x;
                y -= os.y;
            }
            if(out_hovered)
            {
                ImU32 col = ImColor(180, 180, 180);
                x += os.x;
                y += os.y;
                ImGui::RenderFrame(ImVec2(x, y + UpPadding - ImGui::GetScrollY()), ImVec2(x + w, y + h - ImGui::GetScrollY()), col, true,
                                   4.f);
                x -= os.x;
                y -= os.y;
            }

            //---- Image
            ImGui::SetCursorPosX(x + 48);
            ImGui::SetCursorPosY(y + UpPadding + 18);
            if(fs::is_regular_file(fyles.second))
            {
                if(auto ext = fs::path(fyles.second).extension().string(); ext == ".png" || ext == ".jpg")
                    ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::File_img), {64, 64});
                else
                    ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::File_txt), {64, 64});
            } else
                ImGui::Image(CAE::IcoHolder::getTexture_s(CAE::ico_t::Folder), {64, 64});

            //---- Text
            auto str = fs::path(fyles.second).filename().string();
            if(str.length() > 16)
            {
                str.erase(16);
                str += "...";
            }
            int t_w = ImGui::CalcTextSize(str.c_str()).x;
            ImGui::SetCursorPosX(x + (w / 2 - t_w / 2));
            ImGui::SetCursorPosY(y + UpPadding + 64 + 28);
            ImGui::Text("%s", str.c_str());

            //---- Events
            if(fs::is_regular_file(fyles.second))
            {
                if(ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(22)) //4 is 'w' key id
                {
                    context.Input = true;
                }
                if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                {
                    context.selected_nodes.clear();
                }
                if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
                {
                    //show = false;
                    ImGui::PopID();
                    ImGui::EndChild();
                    ImGui::End();
                    int _i = 0;
                    std::string str;
                    if(context.selected_nodes.empty())
                        return {false, ""};
                    //for (const auto& fyles : fs::directory_iterator(p))
                    for(const auto& fyles : context.files)
                        if(context.selected_nodes.find(++_i) != context.selected_nodes.end())
                        {
                            auto f = fs::path(fyles.second).filename().string();
                            std::regex word_regex(context.filter);
                            if(std::regex_match(f, word_regex))
                                str += f + ";";
                        }
                    context.selected_nodes.clear();
                    context.open = false;
                    return {true, str};
                }
                if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                {
                    context.selected_nodes.clear();
                }
                if(out_hovered && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyCtrl)
                {
                    if(!context.selected_nodes.emplace(i).second)
                        context.selected_nodes.erase(i);
                } else if(out_hovered && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyShift)
                {
                    if(context.selected_nodes.size() > 1)
                        context.selected_nodes.clear();
                    else
                    {
                        int start = std::min(*context.selected_nodes.begin(), i);
                        int end = std::max(*context.selected_nodes.begin(), i) + 1;
                        for(int _i = start; _i != end; ++_i)
                            context.selected_nodes.emplace(_i);
                    }
                } else if(out_hovered && ImGui::IsMouseClicked(0))
                {
                    context.selected_nodes.clear();
                    context.selected_nodes.emplace(i);
                }

            } else if(!is_regular && ImGui::IsMouseDoubleClicked(0) && out_hovered)
            {
                context._path = fs::path(fyles.second).generic_string();
                context.updateDir = true;
            }
            //---- Next position
            x += w + XPadding;
            if(x + w > ImGui::GetWindowSize().x)
            {
                x = 0;
                y += h - UpPadding + YPadding;
            }
            ImGui::PopID();
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                             16.f);//just move the cursor below to increase the window size and add a little more free space at the bottom, "for beauty"
        ImGui::EndChild();
        ImGui::End();
        return {false, ""};
    }
    return {false, ""};
}
