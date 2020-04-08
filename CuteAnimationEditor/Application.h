#pragma once
#include "AnimationAsset.h"
#include "TAP.h"
#include <future>
namespace CAE
{
	class Application
	{
	private:
		AnimationAsset* currAsset;
		std::vector<AnimationAsset*> animAssets;

		std::vector<Part> editorSubArray;
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
		sf::Texture  deleteTexture_ico;
		sf::Sprite   deleteSprite_ico;
		sf::Texture  addTexture_ico;
		sf::Sprite   addSprite_ico;
		sf::Vector2f mPrevPose; //used only in handleEvent
		TAP          animPlayer;
		//--------------------------used only in editorUpdate()--------------------------//
		sf::Vector2f m_c_pos;
		sf::Vector2i m_p_pos;
		sf::Vector2f m_c_prevPos;
		sf::Vector2i m_p_prevPos;
		ScaleNode* selectedNode;
		Part* selectedPart;
		//--------------------------end--------------------------//

		std::thread asyncNodeScale;
		float ftStep{ 1.f }, ftSlice{ 1.f }, lastFt{ 1.f }, currentSlice{ 0.f };
		float scaleFactor; //global scale factor, set as a constant
		float nodeSize;
		int   scaleSign;   //can only be positive(1) and negative(-1)
		char buff[256];
		char buff2[256];
		char buff3[256];
		bool LogConsole;
		bool useMouse;
		bool creatorMode;
		bool pointSelected;
		bool useFloat;
		bool newMessage;

		void handleEvent(sf::Event& event);
		void draw();
		void update();
		void clearBuffers();
		void loadState();
		void saveState();
		void loadAsset(std::string path, bool setAsCurr = false);

		void showLog(std::string_view txt);
		void viewUpdated();
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
		Application(sf::RenderWindow& w) : window(&w), state(states::Null), useMouse(false), scaleFactor(1.5f), scaleSign(0), selectedPart(nullptr), selectedNode(nullptr), nodeSize(5), useFloat(true)
		{
			deleteTexture_ico.loadFromFile("attention.png");
			deleteTexture_ico.setSmooth(true);
			deleteSprite_ico.setTexture(deleteTexture_ico);
			//deleteSprite_ico.setScale({ 0.05f,0.05f });

			addTexture_ico.loadFromFile("addico.png");
			addTexture_ico.setSmooth(true);
			addSprite_ico.setTexture(addTexture_ico);
			//addSprite_ico.setScale({ 0.05f,0.05f });
			
			view.setSize(w.getDefaultView().getSize());
			//attention.setPosition(window->mapPixelToCoords(sf::Vector2i(0, 0), view));
		}
		~Application() { for (auto& it : animAssets) delete it; if (asyncNodeScale.joinable())asyncNodeScale.join(); }

		void start();
	};
}

