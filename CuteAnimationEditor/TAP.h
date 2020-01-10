//////////////////////////////////////////////////////////////////////////
// copied from my repository https://github.com/NePutin94/sfml_engine   //
/////////////////////////////////////////////////////////////////////////

#pragma once
#include <SFML/Graphics.hpp>

namespace CAE
{
	enum AnimationState { APlay, APause, AEnd };
	class Animation
	{
	public:
		std::string    name;
		sf::IntRect    rect;
		sf::Vector2f   origin;
		AnimationState state;
		std::vector<sf::IntRect> frames;
		float frame;
		float frameCount;
		float speed;
		bool  looped;
		float scale;

		Animation()
		{
			frame = 0;
			frameCount = 0;
			origin = { 0,0 };
			rect = { 0,0,0,0 };
			scale = 0.f;
			speed = 0.01f;
		}
		virtual sf::IntRect& tick(float time) = 0;
	};

	class CaeAnimation : public Animation
	{
	private:
		sf::IntRect rect;
		sf::Vector2f Center;
	public:
		CaeAnimation() { frame = 0; scale = 0;  speed = 0; }

		sf::IntRect& tick(float time) override
		{

			int widht = rect.width;
			int height = rect.height;
			int top = rect.top;
			int left = rect.left;
			frame += speed * time;
			if (frame > 2) frame = 0;
			if (int(frame) >= 1)
				int z = 5;
			sf::IntRect x(widht * int(frame), top, widht, height);
			return x;

		}
	};

	class TAP ///Tiny Animation Player
	{
	private:
	public:
	};
}

