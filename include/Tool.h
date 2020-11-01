#pragma once
#include "AnimationAsset.h"
#include "EventManager.h"
#include <memory>

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
		EMHolder& EventsHolder;
		sf::RenderWindow* window;
		sf::Texture icon;
		bool Active;
		std::shared_ptr<AnimationAsset> asset;
		virtual void assetUpdated() = 0;
	public:
		Tool() = delete;
		Tool(EMHolder& m, const sf::Texture& t, sf::RenderWindow& window) : EventsHolder(m), icon(t), window(&window) {  }
		sf::Texture& getIco() { return icon; }
		//virtual void handleEvenet(sf::Event& e) = 0;
		void SetActive(bool a) { if (a) Enable(); else Disable(); Active = a; };
		void SetAsset(std::shared_ptr<AnimationAsset> asset) {
			this->asset = asset;
			assetUpdated();
		}
		virtual void Init() {}
		virtual void Enable() {}
		virtual void Disable() {}
        [[maybe_unused]] virtual void alwaysDraw() {}
		virtual void update() = 0;
		virtual void draw(sf::RenderWindow& w) = 0;
	};
}
