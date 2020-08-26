#pragma once
#include "AnimationAsset.h"
#include "EventManager.h"

template<class T>
sf::Rect<T> round(sf::Rect<T> value)
{
	return sf::Rect <T>{round(value.left), round(value.top), round(value.width), round(value.height)};
}

namespace CAE
{
	class Tool
	{
	protected:
		EventManager& eManager;
		sf::RenderWindow* window;
		sf::Texture icon;
		bool Active;
	public:
		Tool() = delete;
		Tool(EventManager& m, const sf::Texture& t, sf::RenderWindow& window) : eManager(m), icon(t), window(&window), Active(false) {  }
		sf::Texture& getIco() { return icon; }
		//virtual void handleEvenet(sf::Event& e) = 0;
		void SetActive(bool a) { Active = a;  };
		virtual void Init() {}
		virtual void update(std::shared_ptr<AnimationAsset>) = 0;
		virtual void draw() = 0;
	};
}
