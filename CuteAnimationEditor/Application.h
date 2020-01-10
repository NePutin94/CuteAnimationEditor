#pragma once
#include "AnimationAsset.h"

namespace CAE
{
	class Application
	{
	private:
		AnimationAsset* currAsset;
		std::vector<AnimationAsset*> animAssets;
		enum class states
		{
			Null = 0,
			CreateAsset,
			LoadAsset,
			loadedAssets,
			SaveAsset,
			Editor,
			WindowSettings,
			Exit
		} state;

		sf::RenderWindow* window;
		sf::View view;
		sf::Clock deltaClock;
		sf::Clock pressClock;
		sf::Texture grid;
		sf::Sprite gridSpr;
		sf::Vector2f m_pos;
		sf::Vector2f m_prevPos;
		sf::Vector2f mPrevPose;

		float ftStep{ 1.f }, ftSlice{ 1.f };
		float lastFt{ 1.f };
		float currentSlice{ 0.f };
		float zoom;
		char buff[256];
		char buff2[256];
		char buff3[256];
		bool LogConsole;
		bool useMouse;
		bool creatorMode;
		bool pointSelected;

		void handleEvent(sf::Event& event);
		void draw();
		void update();
		void clearBuffers();
		void loadState();
		void saveState();

		auto makeGrid(sf::Vector2f sz);
		void updateGrid(sf::Vector2f size);
		void editorUpdate();
		void loadAssets();
		void viewSettings();
		void createAssets();
		void editor();
		void mainWindow();
		void viewLoadedAssets();
		void saveAsset();
		void drawMenuBar();
		void drawUI();
	public:
		Application(sf::RenderWindow& w) : window(&w), state(states::Null), useMouse(false), zoom(1.5f)
		{
			view.setSize(w.getDefaultView().getSize());
		}
		~Application() { for (auto& it : animAssets)delete it; }

		void start();
	};
}

