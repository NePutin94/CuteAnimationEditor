#pragma once
#include <filesystem>
#include <set>
#include <string.h>
namespace CAE
{
	class FileManager
	{
	private:
		std::filesystem::path path;
		std::set<int> selected_nodes;
		bool show;
		char buffer[256];
	public:
		FileManager() = delete;
		FileManager(std::string_view int_path) : show(false) { strcpy(buffer, "./assets"); }
		void open();
		std::pair<bool, std::string> operator()(bool o);
	};
}
