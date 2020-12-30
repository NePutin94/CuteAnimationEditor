#include "../include/Application.h"

using namespace CAE;

int main()
{
    sf::RenderWindow window(sf::VideoMode(1800, 900), "CAE");
    ImGui::SFML::Init(window, true);
    ImGuiIO& IO = ImGui::GetIO();
    IO.Fonts->Clear();
    IO.Fonts->AddFontFromFileTTF("data\\fonts\\ArialRegular.ttf", 15.f);
    ImGui::SFML::UpdateFontTexture();
    CAE::Application app(window);
    app.start();
    return 0;
}
