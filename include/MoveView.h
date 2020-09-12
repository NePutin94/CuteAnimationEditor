#pragma once

#include "Tool.h"
#include <cmath>

namespace CAE
{
	class MoveView : public Tool
	{
	private:
		sf::View& viewToMove;
		bool& useMouse;
		bool keyPress;
		bool ImGuiDisabled;
		float zoom = 1;
		void assetUpdated() override {}
	public:
		MoveView(EMHolder& m, const sf::Texture& t, sf::RenderWindow& window, sf::View& v, bool& useMouse) : Tool(m, t, window), viewToMove(v), useMouse(useMouse), keyPress(false){}

		void Enable() override
		{
			EventsHolder["MoveView"].setEnable(true);
		}

		void Disable() override
		{
			EventsHolder["MoveView"].setEnable(false);
			keyPress = false;
		}

		void Init() override
		{
			auto& eManager = EventsHolder.addEM("MoveView", false);
			eManager.addEvent(MouseEvent::ButtonPressed(sf::Mouse::Left), [this](sf::Event& event)
				{
					keyPress = true;
					//oldPos = sf::Vector2f(sf::Mouse::getPosition(*window));
				});
			eManager.addEvent(MouseEvent(sf::Event::MouseButtonReleased), [this](sf::Event& event)
				{
					keyPress = false;
				});
			eManager.addEvent(MouseEvent(sf::Event::MouseMoved), [this](sf::Event& event)
				{
					if (keyPress && ImGuiDisabled)
					{
						auto delta = (sf::Vector2f)EventsHolder.getDelta();
						delta.x *= zoom;
						delta.y *= zoom;
						viewToMove.move(delta);	
					}
				});
			eManager.addEvent(MouseEvent(sf::Event::MouseWheelScrolled), [this](sf::Event& event)
				{
					if (!keyPress && ImGuiDisabled)
					{
						if (event.mouseWheelScroll.delta <= -1)
							zoom = std::min(2.f, zoom + .1f);
						else if (event.mouseWheelScroll.delta >= 1)
							zoom = std::max(.1f, zoom - .1f);

						viewToMove.setSize(window->getDefaultView().getSize());
						viewToMove.zoom(zoom);
						EventsHolder.zoom = zoom;
					}
				});
		}
		void update() override 
		{
			window->setView(viewToMove); 
			if (ImGui::IsAnyWindowHovered()) ImGuiDisabled = false; else ImGuiDisabled = true;
		}

		void draw(sf::RenderWindow&) override {}
	};
}
