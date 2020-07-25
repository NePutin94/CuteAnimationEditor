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
		sf::RectangleShape shape;
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
		void exit();
		void editorDragDropLogic();
		void mainWindow();
		void viewLoadedAssets();
		void saveAsset();
		void drawMenuBar();
		void drawUI();
		void addEventsHandler();
		//sf::FloatRect selectArea();
		bool selectArea;
	public:
		Application(sf::RenderWindow& w);
		~Application() { if (asyncNodeScale.joinable()) asyncNodeScale.join(); }

		void start();
	};
}

