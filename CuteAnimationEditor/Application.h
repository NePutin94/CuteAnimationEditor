#pragma once
#include "AnimationAsset.h"
#include "TAP.h"
#include <future>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui/highgui.hpp>

namespace CAE
{
	inline cv::Mat sfml2opencv(const sf::Image& img, bool toBGRA = false, bool fixZeroTransp = false)
	{
		cv::Size size(img.getSize().x, img.getSize().y);
		cv::Mat mat(size, CV_8UC4, (void*)img.getPixelsPtr(), cv::Mat::AUTO_STEP);
		if (fixZeroTransp)
			for (int i = 0; i < mat.rows; ++i)
			{
				for (int j = 0; j < mat.cols; ++j)
				{
					auto& p = mat.at<cv::Vec4b>(i, j);
					if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0)
					{
						p[0] = 255;
						p[1] = 255;
						p[2] = 255;
						p[3] = 0;
					}
				}
			}
		if (toBGRA)
			cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGRA);
		return mat.clone();
	}
	inline void printExternalContours(cv::Mat img, vector<vector<cv::Point>> const& contours, vector<cv::Vec4i> const& hierarchy, int const idx)
	{
		//for every contour of the same hierarchy level
		for (int i = idx; i >= 0; i = hierarchy[i][0])
		{
			//print it
			//cv::drawContours(img, contours, i, cv::Scalar(255));
			//out.emplace_back(cv::boundingRect(contours[i]));
			cv::rectangle(img, cv::boundingRect(contours[i]), cv::Scalar(120, 120, 255, 255));
			//for every of its internal contours
			//for (int j = hierarchy[i][2]; j >= 0; j = hierarchy[j][0])
			//{
			//	//	//recursively print the external contours of its children
			//	printExternalContours(img, contours, hierarchy, hierarchy[j][2]);
			//}
		}
		//for (auto i = 0; i < contours.size(); ++i)
		//{
		//	if (hierarchy[i][2] == -1)
		//		cv::rectangle(img, cv::boundingRect(contours[i]), cv::Scalar(60, 60, 255, 200));
		//}
	}
	class Application
	{
	private:
		AnimationAsset* currAsset;
		std::vector<AnimationAsset*> animAssets;
		std::string lastLog;
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

		struct 
		{
			cv::Mat source_image;
			cv::Mat transform_image;
			bool gray = false;
			bool useMorph = false;
			int thresh = 0;
			int add = 0;
			int morph_iteration = 0; 
			int mode = 1;
			sf::Vector2i kernel_rect = { 0,0 };
		} magicTool;

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
		sf::Vector2f mPrevPose; //used only in handleEvent
		TAP          animPlayer;
		//--------------------------used only in editorUpdate()--------------------------//
		sf::Vector2f m_c_pos;
		sf::Vector2i m_p_pos;
		sf::Vector2f m_c_prevPos;
		sf::Vector2i m_p_prevPos;
		sf::Vector2i m_p_prevPos2;
		ScaleNode* selectedNode;
		std::shared_ptr<Part> selectedPart;
		std::shared_ptr<Part> lastSelected;
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

