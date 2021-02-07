#pragma once

#include "AnimationAsset.h"
#include "TAP.h"
#include "EventManager.h"
#include "MagicTool.h"
#include "Tools.h"
#include "IcoHolder.h"
#include <set>
#include <list>
#include <deque>

namespace CAE
{
    class Application
    {
    private:
        static constexpr short file_history_sz = 5;
        int corner = 1;
        int bg_alpha = 8;
        float fps;
        IcoHolder ico_holder;
        Tools tools_container;
        std::deque<std::string> fileHistory;
        TAP animPlayer;
        EMHolder eventManagers;
        std::string lastOpenFile;
        std::shared_ptr<AnimationAsset> currAsset;
        std::list<std::shared_ptr<AnimationAsset>> animAssets;
        std::vector<std::shared_ptr<Part>> editorSubArray;
        std::shared_ptr<AnimationAsset> load;
        void historyAddFile(std::string f)
        {
            if(std::find_if(fileHistory.begin(),fileHistory.end(),[f](auto& file) { return f == file;}) != fileHistory.end())
                return;
            fileHistory.push_front(f);
            if(fileHistory.size() >= file_history_sz)
            {
                fileHistory.pop_back();
                fileHistory.push_front(f);
            }
        }

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

        //--------------------------end--------------------------//
        sf::RectangleShape shape;

        std::set<int> selectedParts;
        std::set<int> selectedGroups;

        float scaleFactor; //depr
        float nodeSize;
        int scaleSign;   //drpt
        char buff[256]{};
        char buff2[2048]{};
        char buff3[256]{};
        bool LogConsole;
        bool creatorMode;
        bool toolsWindowFocused;
        bool useFloat;
        bool newMessage;

        void handleEvent(sf::Event& event);

        void draw();

        void openRecent();

        void update();

        void clearBuffers();

        void loadState();

        void saveState();

        void loadAsset(std::string path, bool setAsCurr = false);

        void changeMovingMode_e(sf::Event&);

        void setTheme();

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

        ~Application() = default;

        void start();

        friend class Tools;
    };
}

