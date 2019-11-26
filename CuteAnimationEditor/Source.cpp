#include "Application.h"
int main()
{
	sf::RenderWindow window(sf::VideoMode(1600, 800), "CAE");
	ImGui::SFML::Init(window, true);

	CAE::Application app(window);
	app.start();

	return 0;
}