#pragma once
#include "Tool.h"
namespace CAE
{
	class SelectionTool : public Tool
	{
	private:
		sf::FloatRect selectionRect;
		void assetUpdated() override {}
	public:
		void update() override {}
		void draw(sf::RenderWindow&) override {}
	};

}