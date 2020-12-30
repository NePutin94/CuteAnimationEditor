//////////////////////////////////////////////////////////////////////////
// copied from my repository https://github.com/NePutin94/sfml_engine   //
/////////////////////////////////////////////////////////////////////////

#pragma once
#include <SFML/Graphics.hpp>
#include <list>
#include "AnimationAsset.h"
namespace CAE
{
	enum AnimationState { APlay, APause, AEnd };

	class Animation
	{
	public:
		std::string    name;
		sf::Vector2f   origin;
		AnimationState state;
		std::vector<sf::FloatRect> frames;
		float frame;
		float frameCount;
		float speed;
		float scale;
		bool  looped;

		Animation()
		{
			frame = 0;
			frameCount = 0;
			origin = { 0,0 };
			//rect = { 0,0,0,0 };
			scale = 0.f;
			speed = 0.01f;
		}
		virtual sf::FloatRect& tick(float time) = 0;
	};

	class CaeAnimation : public Animation
	{
	private:
		sf::Vector2f Center;
	public:
		CaeAnimation() = default;

		sf::FloatRect& tick(float time) override
		{
			frame += speed * time;
			if (frame > frameCount)
			{
				frame = 0;
				if (!looped)
					state = AEnd;
			}
			origin.x = frames[static_cast<int>(frame)].width / 2;
			origin.y = frames[static_cast<int>(frame)].height;
			return frames[static_cast<int>(frame)];
		}
	};

	/*------------------------------------------------------------------Tiny Animation Player------------------------------------------------------------------*/
	class TAP : public std::list<CaeAnimation>
	{
	private:
		CaeAnimation* currAnim;
	public:
		TAP() : currAnim(nullptr){}

		void setCurrentAnim(CaeAnimation& anim) { currAnim = &anim;}
		auto getCurrentAnimation() { return currAnim; }
		bool hasAnimation() { return currAnim != nullptr; }

		sf::FloatRect animUpdate(float t)
		{
			return (hasAnimation()) ? currAnim->tick(t) : sf::FloatRect{0,0,0,0};
		}

		void parseAnimationAssets(std::vector<std::shared_ptr<Group>> groups)
		{
			clear();
			currAnim = nullptr;
			for (auto group : groups)
			{
				CaeAnimation anim;
				anim.name = group->getName();
				anim.speed = group->getSpeed();
				anim.frameCount = group->getParts().size() - 1; //numbering starts from zero
				for (auto& part : group->getParts())
					anim.frames.push_back(part->getRect());
				this->emplace_back(anim);
				if (this->empty())
					currAnim = &(*this->begin());
			}
		}
	};
}

