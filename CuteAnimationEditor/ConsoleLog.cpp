#include "ConsoleLog.h"
#include <cmath>
#include <iostream>
#include <fstream>
using namespace CAE;

vector<Console::Log>       Console::AppLog::Buffer = {};
bool                       Console::AppLog::ScrollToBottom = 0;
std::vector<std::string>   Console::AppLog::current_input = {};
bool Console::AppLog::newLog = false;

bool CAE::Console::AppLog::hasNewLogByTyp(logType t)
{
	if (Buffer.empty())
		return false;
	bool prev = newLog; newLog = false;
	if (Buffer.back().type == t && prev)
		return true;
	else
		return false;
}

void CAE::Console::AppLog::addLog(Log log)
{
	if (!Buffer.empty())
	{
		if (Buffer.back().pervText != log.pervText)
			Buffer.emplace_back(log);
		else
			Buffer.back().count_update(++Buffer.back().log_count);
	}
	else
		Buffer.emplace_back(log);
	newLog = true;
}

void CAE::Console::AppLog::addLog(std::string s, logType t)
{
	if (!Buffer.empty())
	{
		if (Buffer.back().pervText != s)
			Buffer.emplace_back(s, t);
		else
			Buffer.back().count_update(++Buffer.back().log_count);
	}
	else
		Buffer.emplace_back(s, t);
	newLog = true;
}

void Console::AppLog::saveLog(std::string_view path)
{
	std::ofstream out;
	out.open(path.data());
	if (out.is_open())
	{
		for (auto log : Buffer)
			out << log.text << std::endl;
	}
}
void CAE::Console::Log::count_update(int count)
{
	auto b = text.find_first_of(')');
	auto a = text.find_first_of('(');
	auto offset = b - a - 1;
	text.erase(a + 1, b - a - 1);
	text.insert(a + 1, std::to_string(count));
}

void Console::AppLog::Draw(const char* title, bool* p_open)
{
	if (*p_open)
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y * 0.3));
		ImGui::SetNextWindowBgAlpha(0.8f);
		ImGui::Begin(title, p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
		if (ImGui::Button("Clear")) Clear();
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		static bool find = false;
		static int itemCount = 0;
		ImGui::Checkbox("Find By Type", &find);
		static std::string item_current = logType_s[0].data();
		ImGui::PushItemWidth(45);
		if (ImGui::BeginCombo("", item_current.c_str(), ImGuiComboFlags_NoArrowButton))
		{
			for (auto& item : logType_s)
			{
				bool is_selected = (item_current == item);
				if (ImGui::Selectable(item.data(), is_selected))
					item_current = item;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		char buff_search[256] = { 0 };
		char buff_input[256] = { 0 };

		ImGui::PushItemWidth(150);
		bool value_changed = ImGui::InputText("Filter", buff_search, IM_ARRAYSIZE(buff_search));
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(300);
		bool input_change = ImGui::InputText("Input", buff_input, IM_ARRAYSIZE(buff_input), ImGuiInputTextFlags_EnterReturnsTrue);
		std::string last_result;
		if (input_change)
		{
			ScrollToBottom = true;
			if (buff_input[0] == '/')
			{

			}
		}
		ImGui::PopItemWidth();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (copy) ImGui::LogToClipboard();
		if (value_changed)
		{
			std::string input = buff_search;
			for (auto item : Buffer)
			{
				std::string found = "";
				std::size_t start_pos = -1;
				start_pos = item.text.find_first_of(" ");
				for (char i : input)
				{
					std::size_t pos_begin = -1;

					pos_begin = item.text.find_first_of(i, start_pos);
					if (pos_begin != -1)
					{
						found += item.text.substr(pos_begin, 1);
					}
				}
				if (found == input)
				{
					ImGui::TextColored(item.color, item.text.c_str());
				}
			}
		}
		else if (find && item_current != "all")
		{
			for (auto item : Buffer)
			{
				if (item_current == "error")
				{
					if (item.type == logType::error)
						ImGui::TextColored(item.color, item.text.c_str());
				}
				else if (item_current == "info")
				{
					if (item.type == logType::info)
						ImGui::TextColored(item.color, item.text.c_str());
				}
				else if (item_current == "fatal")
				{
					if (item.type == logType::fatal)
						ImGui::TextColored(item.color, item.text.c_str());
				}
				else if (item_current == "system")
				{
					if (item.type == logType::system)
						ImGui::TextColored(item.color, item.text.c_str());
				}
				else if (item_current == "script")
				{
					if (item.type == logType::script)
						ImGui::TextColored(item.color, item.text.c_str());
				}
				else if (item_current == "message")
				{
					if (item.type == logType::message)
						ImGui::TextColored(item.color, item.text.c_str());
				}
			}
		}
		else
			for (auto i : Buffer)
				ImGui::TextColored(i.color, i.text.c_str());
		//if (ScrollToBottom)
		ImGui::SetScrollHere(1.f);
		ScrollToBottom = false;
		ImGui::EndChild();
		ImGui::End();
	}

}

Console::Log::Log(std::string clearText, logType t)
{
	type = t;
	log_count = 1;
	double time = clock();
	std::string ti = std::to_string(std::round(time / 10) / 100);
	ti.erase(ti.find_first_of('.') + 3, ti.size());
	std::string FormattedText = "[sec:" + ti;
	switch (t)
	{
	case logType::error:
		color = ImVec4(1, 0.35f, 0, 1);
		FormattedText += " type:error";
		break;
	case logType::info:
		color = ImVec4(0, 1, 0.3f, 1);
		FormattedText += " type:info";
		break;
	case logType::fatal:
		color = ImVec4(1, 0, 0, 1);
		FormattedText += " type:fatal";
		break;
	case logType::system:
		color = ImVec4(1, 0, 0.8, 1);
		FormattedText += " type:system";
		break;
	case logType::script:
		color = ImVec4(0.1f, 0.5f, 0.1f, 1);
		FormattedText += " type:script";
		break;
	case logType::message:
		color = ImVec4(0.32f, 0.65f, 1.f, 1);
		FormattedText += " type:message";
		break;
	}
	FormattedText += " (1)]: ";
	FormattedText += clearText;
	FormattedText += " \n";
	text = FormattedText;
	pervText = clearText;
}
