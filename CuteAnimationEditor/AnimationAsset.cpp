#include "AnimationAsset.h"
#include <iomanip>

bool CAE::AnimationAsset::loadFromFile()
{
	if (std::ifstream i(assetPath.data()); i.is_open())
	{
		json j;
		i >> j;
		try
		{
			auto info = j.at("defaultInfo");
			name = info.at("name").get<std::string>();
			texturePath = info.at("texturePath").get<std::string>();
			for (auto& group : j.at("Groups"))
			{
				groups.emplace_back(Group());
				groups.back().load(group);
			}
		}
		catch (json::exception & e)
		{
			std::string str = e.what();
			Console::AppLog::addLog("Json throw exception, message: " + str, Console::error);
		}
		texture.loadFromFile(texturePath);
		setTexture(texture);
		return true;
	}
	else
		Console::AppLog::addLog("File " + texturePath + " can't be opened!", Console::error);
	return false;
}

bool CAE::AnimationAsset::saveAsset(std::string_view path)
{
	ofstream o(path.data());
	json j;
	auto& info = j["defaultInfo"];
	info["name"] = name;
	info["texturePath"] = texturePath;

	int i = 0;
	auto& g = j["Groups"];
	for (auto& group : groups)
	{
		group.save(g[i]);
		++i;
	}
	o << std::setw(4) << j;
	o.close();
	return true;
}

CAE::AnimationAsset::AnimationAsset(std::string_view _path) : assetPath(_path) {}
