#pragma once
#include "AnimationAsset.h"
#include "TAP.h"

namespace CAE
{
	class Application
	{
	private:
		AnimationAsset*              currAsset;
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
		sf::View     view;
		sf::Clock    deltaClock;
		sf::Clock    pressClock; //used only in handleEvent, not global time
		sf::Clock    attTimer;
		sf::Texture  grid;
		sf::Sprite   gridSpr;
		sf::Texture  t;
		sf::Sprite   attention;
		sf::Vector2f mPrevPose; //used only in handleEvent
		TAP          animPlayer;
		//--------------------------used only in editorUpdate()--------------------------//
		sf::Vector2f m_c_pos;
		sf::Vector2i m_p_pos;
		sf::Vector2f m_c_prevPos; 
		sf::Vector2i m_p_prevPos;
		ScaleNode*   selectedNode;
		Part*        selectedPart;
		//--------------------------end--------------------------//

		float ftStep{ 1.f }, ftSlice{ 1.f }, lastFt{ 1.f }, currentSlice{ 0.f };
		float scaleFactor; //global scale factor, set as a constant
		int   scaleSign;   //can only be positive(1) and negative(-1)
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
		
		void viewUpdated();
		auto makeGrid(sf::Vector2f sz);
		void updateGrid(sf::Vector2f size);
		void editorUpdate();
		void loadAssets();
		void tapWindow();
		void viewSettings();
		void createAssets();
		void editor();
		void mainWindow();
		void viewLoadedAssets();
		void saveAsset();
		void drawMenuBar();
		void drawUI();
	public:
		Application(sf::RenderWindow& w) : window(&w), state(states::Null), useMouse(false), scaleFactor(1.5f), scaleSign(0), selectedPart(nullptr), selectedNode(nullptr)
		{
			t.loadFromFile("attention.png");
			attention.setTexture(t);
			view.setSize(w.getDefaultView().getSize());
			attention.setPosition(window->mapPixelToCoords(sf::Vector2i(0, 0), view));
		}
		~Application() { for (auto& it : animAssets) delete it; }

		void start();
	};
}

