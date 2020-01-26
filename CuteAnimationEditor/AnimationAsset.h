#pragma once
#include "Group.h"

namespace CAE
{
	class AnimationAsset : public sf::Sprite
	{
	private:
		std::string texturePath;
		std::string assetPath;
		std::string name;
		sf::Texture texture;
		std::vector<Group> groups;

		bool loadFromFile();
		bool saveAsset(std::string_view);
	public:
		AnimationAsset(std::string_view _path);
		~AnimationAsset() { saveAsset(assetPath); };
		auto begin()  { return groups.begin(); }
		auto end()    { return groups.end(); }
		auto getName() const { return name; }
		auto getPath() { return texturePath; }
		std::pair<int, int> getWH() { return std::make_pair(texture.getSize().x, texture.getSize().y); }

		friend class Application;
	};
}