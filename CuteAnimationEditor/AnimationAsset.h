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
		std::vector<std::shared_ptr<Group>> groups;

		bool loadFromFile();
		bool saveAsset(std::string_view = "");
	public:
		explicit AnimationAsset(std::string_view _path);
		~AnimationAsset() = default;
		auto begin() { return groups.begin(); }
		auto end() { return groups.end(); }
		std::string getName() const { return name; }
		std::string getPath() const { return texturePath; }
		auto getWH() { return std::make_pair(texture.getSize().x, texture.getSize().y); }

		friend class Application;
	};
}