#pragma once
#include <imgui.h>
#include <imgui-SFML.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
using namespace std;
namespace CAE
{
	namespace Console
	{
		enum logType
		{
			error = 1,
			info,
			fatal,
			system,
			script,
			message
		};

		constexpr std::string_view logType_s[] =
		{
			 "all", "error", "info", "fatal", "system" ,"script", "message"
		};

		struct Log
		{
			Log(std::string s, logType t);
			std::string text;
			std::string pervText;
			void count_update(int count);
			int log_count;
			ImVec4 color;
			logType type;
		};

		class AppLog
		{
		private:
			static bool ScrollToBottom; ///< Flag, true if the scrollbar
			static std::vector<std::string> current_input;
			static vector<Log> Buffer;
			static bool newLog;

		public:
			AppLog() = default;
			~AppLog() { saveLog("Data/log.txt"); }
			static void Clear()
			{
				Buffer.clear();
				Buffer.shrink_to_fit();
			}
			static bool hasNewLog() { bool prev = newLog; newLog = false; return prev; }
			static bool hasNewLogByTyp(logType t);
			static void addLog(Log log);
			static void addLog(std::string s, logType t);
			static void saveLog(std::string_view path);
			static void Draw(const char* title, bool* p_open);
			static std::string_view lastLog()
			{
				return Buffer.back().pervText;
			}
		};
	} // namespace Console
}