#pragma once

#include "AnimationAsset.h"
#include "TAP.h"
#include "EventManager.h"
#include "MagicTool.h"
#include "Tools.h"
#include <future>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <set>
#include <list>
#include "IcoHolder.h"

namespace CAE
{

    class Application
    {
    private:
        IcoHolder ico_holder;
        Tools tools_container;
        TAP animPlayer;
        //MagicTool    magicTool;
        //EventManager eManager;
        EMHolder eventManagers;

        std::shared_ptr<AnimationAsset> currAsset;
        std::list<std::shared_ptr<AnimationAsset>> animAssets;
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
        bool assetChanged = false;
        sf::RenderWindow* window;
        sf::View view;
        sf::Vector2f scaleView;
        sf::Vector2f unscaleView;
        sf::Clock deltaClock;
        sf::Clock attTimer;

//        ScaleNode* selectedNode;
//        std::shared_ptr<Part> selectedPart;
//        std::shared_ptr<Part> lastSelected;
//        std::shared_ptr<Group> selectedGroup;
        //--------------------------end--------------------------//
        sf::RectangleShape shape;

        std::set<int> selectedParts;
        std::set<int> selectedGroups;

        //std::thread asyncNodeScale;
        float ftStep{1.f}, ftSlice{1.f}, lastFt{1.f}, currentSlice{0.f};
        float scaleFactor; //global scale factor, set as a constant
        float nodeSize;
        int scaleSign;   //can only be positive(1) and negative(-1)
        char buff[256]{};
        char buff2[2048]{};
        char buff3[256]{};
        bool LogConsole;
        bool useMouse;
        bool creatorMode;
        bool toolsWindowFocused;
        //bool pointSelected;
        bool useFloat;
        bool newMessage;
        bool selectArea;

        void handleEvent(sf::Event& event);

        void draw();

        void update();

        void clearBuffers();

        void loadState();

        void saveState();

        void loadAsset(std::string path, bool setAsCurr = false);

        void changeMovingMode_e(sf::Event&);

        void useMouseToMove_e(sf::Event&);

        [[deprecated]] void deletSelectedPart_e(sf::Event&);

        [[deprecated]] void viewScale_e(sf::Event& event);

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

    public:
        Application(sf::RenderWindow& w);

        ~Application()
        {}

        void start();

        friend class Tools;
    };

}

