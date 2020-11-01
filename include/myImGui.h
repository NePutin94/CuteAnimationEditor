#include <imgui-SFML.h>
#include "imgui.h"
#include <set>
#include <map>
#include <filesystem>
namespace ImGui
{
	void Image(const sf::Texture& texture, const sf::Vector2f& size, const sf::Vector2f& uv0, const sf::Vector2f& uv1, const sf::Color& tintColor = sf::Color::White,
		const sf::Color& borderColor = sf::Color::Transparent);
	bool SelectableImage(const sf::Texture& texture, bool selected, const ImVec2& size);
	class FileManager_Context;
	std::pair<bool, std::string> FileManager(FileManager_Context& context);
	class FileManager_Context
	{
	private:
		bool open;
		bool updateDir;
		std::string _path;
		std::string filter;
		std::set<int> selected_nodes;
		std::multimap<size_t, std::filesystem::directory_entry> files;
		static short id;
		int curr_id;
        bool Input;
	public:
		FileManager_Context(std::string _path, std::string filter = ".+") : Input(false),_path(_path), curr_id(++id), open(false), filter(filter), updateDir(true) {}
		~FileManager_Context() { --id; }

		std::string getPath() { return _path; }
		void setOpen(bool _open) { open = (open) ? true : _open; }

		friend std::pair<bool, std::string> ImGui::FileManager(FileManager_Context&);

    };
}