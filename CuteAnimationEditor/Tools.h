#pragma once
#include "MovePen.h"
#include "MoveView.h"
#include <imgui.h>

namespace CAE
{
	enum class tool_type
	{
		MOVEPART = 1,
		MAGICTOOL,
		SELECTION,
		MOVEVIEW
	};

	class Application;

	class Tools
	{
	private:
		using container = std::map<tool_type, std::shared_ptr<Tool>>;
		using iterator = container::iterator;
		container tools;
		iterator current_tool;
		using Ptr = std::shared_ptr<AnimationAsset>;
		Ptr  data;
		void toolsPanel();

	public:
		Tools(Application* app, sf::RenderWindow* w);

		void Init()
		{
			for (auto& [_, t] : tools)
				t->Init();
		}

		template<class T>
		auto getTool(tool_type type)
		{
			return static_pointer_cast<T>(tools[type]);
		}

		void update();

		void draw(sf::RenderWindow& w);

		void handleEvent(sf::Event& e);

		void changeCurrAsset(std::shared_ptr<AnimationAsset>);
	};
}