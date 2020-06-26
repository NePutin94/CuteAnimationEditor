#pragma once
#include "AnimationAsset.h"
#include "TAP.h"
#include "EventManager.h"
#include "MagicTool.h"
#include <future>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui/highgui.hpp>

namespace CAE
{
	class Application
	{
	private:
		std::shared_ptr<AnimationAsset> currAsset;
		std::list<std::shared_ptr<AnimationAsset>> animAssets;
		//std::string lastLog;
		std::vector<std::shared_ptr<Part>> editorSubArray;
		enum class states
		{
			Null = 0,
			CreateAsset,
			LoadAsset,
			loadedAssets,
			SaveAsset,
			macgicTool,
			Editor,
			WindowSettings,
			Exit
		} state;

		sf::RenderWindow* window;
		sf::View     view;
		sf::Clock    deltaClock;
		sf::Clock    pressClock; //used only in handleEvent, not global time
		sf::Clock    attTimer;
		sf::Clock    attDelta;
		sf::Texture  deleteTexture_ico;
		sf::Sprite   deleteSprite_ico;
		sf::Texture  addTexture_ico;
		sf::Sprite   addSprite_ico;
		//sf::Vector2f mPrevPose; //used only in handleEvent
		//sf::Vector2f mCurrPose;
		TAP          animPlayer;
		ScaleNode* selectedNode;
		std::shared_ptr<Part> selectedPart;
		std::shared_ptr<Part> lastSelected;
		std::shared_ptr<Group> selectedGroup;
		//--------------------------end--------------------------//

		MagicTool magicTool;
		EventManager eManager;

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
		bool toolsWindowFocused;
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

		void changeMovingMode_e(sf::Event&);
		void useMouseToMove_e(sf::Event&);
		void deletSelectedPart_e(sf::Event&);
		void viewScale_e(sf::Event& event);


		void magicSelection();
		void showLog(std::string_view txt);
		void viewUpdated();
		void editorUpdate();
		void loadAssets();
		void tapWindow();
		void viewSettings();
		void createAssets();
		void editor();
		void editorDragDropLogic();
		void mainWindow();
		void viewLoadedAssets();
		void saveAsset();
		void drawMenuBar();
		void drawUI();

		void addEventsHandler()
		{
			eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::F1), &Application::changeMovingMode_e, this);
			eManager.addEvent(KBoardEvent::KeyPressed(sf::Keyboard::F2), &Application::useMouseToMove_e, this);
			eManager.addEvent(KBoardEvent::KeyReleased(sf::Keyboard::Delete), &Application::deletSelectedPart_e, this);
			eManager.addEvent(KBoardEvent::KeyReleased(sf::Keyboard::Tilde), [this](sf::Event&) {LogConsole = !LogConsole; });

			eManager.addInput({ sf::Keyboard::Space, sf::Mouse::Button::Left },
				[this]() {
					if (useMouse)
					{
						view.move(eManager.worldMouseDelta());
						eManager.updateMousePosition(*window, view);//because the view position has changed, the mouse position (even if it did not move in the next frame) 
																    //will be different on the next frame. to avoid re-shifting the view, 
																	//you need to update the mouse coordinates again, already in the new view
						//attention.setPosition(window->mapPixelToCoords(sf::Vector2i(0, 0), view));
					} });
			eManager.addEvent(MouseEvent::WheelScrolled(), &Application::viewScale_e, this);
			eManager.addEvent(MouseEvent::ButtonReleased(sf::Mouse::Left), [this](sf::Event&) { pointSelected = false; });
		}
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
			addEventsHandler();
			view.setSize(w.getDefaultView().getSize());
			//attention.setPosition(window->mapPixelToCoords(sf::Vector2i(0, 0), view));
		}
		~Application() { if (asyncNodeScale.joinable()) asyncNodeScale.join(); }

		void start();
	};
}

