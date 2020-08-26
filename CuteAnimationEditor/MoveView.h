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
		bool moving;
		sf::Vector2f oldPos;
		float zoom = 1;
		float accumZoom = 1;

	public:
		MoveView(EventManager& m, const sf::Texture& t, sf::RenderWindow& window, sf::View& v, bool& useMouse) : Tool(m, t, window), viewToMove(v), useMouse(useMouse), moving(false)
		{
		}
		void Init() override
		{
			eManager.addEvent(MouseEvent::ButtonPressed(sf::Mouse::Left), [this](sf::Event& event)
				{
					if (Active)
					{
						moving = true;
						oldPos = sf::Vector2f(sf::Mouse::getPosition(*window));
					}
				});
			eManager.addEvent(MouseEvent(sf::Event::MouseButtonReleased), [this](sf::Event& event)
				{
					if (Active)
					{
						moving = false;
					}
				});
			eManager.addEvent(MouseEvent(sf::Event::MouseMoved), [this](sf::Event& event)
				{
					if (moving && Active)
					{
						const sf::Vector2f newPos = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
						sf::Vector2f deltaPos = oldPos - newPos;
						deltaPos.x *= zoom;
						deltaPos.y *= zoom;
						viewToMove.move((sf::Vector2f)deltaPos);
						window->setView(viewToMove);
						oldPos = newPos;
					}
				});
			eManager.addEvent(MouseEvent(sf::Event::MouseWheelScrolled), [this](sf::Event& event)
				{
					if (!moving && Active)
					{
						if (event.mouseWheelScroll.delta <= -1)
							zoom = std::min(2.f, zoom + .1f);
						else if (event.mouseWheelScroll.delta >= 1)
							zoom = std::max(.5f, zoom - .1f);

						viewToMove.setSize(window->getDefaultView().getSize());
						viewToMove.zoom(zoom);
						window->setView(viewToMove);
					}
				});
		}
		void update(std::shared_ptr<AnimationAsset> data) override {}

		float getZoomFactor() const { return zoom; }

		void draw() override {}
	};
}
