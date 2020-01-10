#pragma once
#include "Group.h"

namespace CAE
{
	class AnimationAsset : public sf::Sprite
	{
	private:
		std::string texturePath;
		std::string assetPath;
		sf::Texture texture;
		std::string name;

		bool loadFromFile();
		bool saveAsset(std::string_view);
	public:
		std::vector<Group> groups;
		AnimationAsset(std::string_view _path);
		~AnimationAsset() { saveAsset(assetPath); };
		auto cbegin() const { return groups.cbegin(); }
		auto cend() const { return groups.cend(); }
		auto getName() const { return name; }
		auto getPath() { return texturePath; }
		std::pair<int, int> getWH() { return std::make_pair(texture.getSize().x, texture.getSize().y); }

		friend class Application;
	};
}