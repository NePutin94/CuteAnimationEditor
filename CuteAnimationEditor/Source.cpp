#include "Application.h"
using namespace std;


int main()
{
	sf::RenderWindow window(sf::VideoMode(1800, 900), "CAE");
	ImGui::SFML::Init(window, true);

	CAE::Application app(window);
	app.start();

	return 0;
}