#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui/highgui.hpp>

class MagicTool
{
private:

	std::vector<cv::Rect> boundRect;
	//bool gray;
	//bool makeAllBlack;
	//bool useMorph;
	//int thresh;
	//int add;
	//int morph_iteration;
	//int mode;
	sf::Image img;
	cv::Mat sfml2opencv(const sf::Image& img, bool toBGRA = false, bool fixZeroTransp = false);
public:
	MagicTool():
		add{ 0 }, thresh{ 0 }, mode{ 0 },
		morph_iteration{ 0 }, gray{ true }, makeAllBlack{ true }, useMorph{ false }
	{}

	bool gray; //you should always use grayscale btw
	bool makeAllBlack;
	bool useMorph;
	int thresh;
	int add;
	int morph_iteration;
	int mode;
	cv::Mat source_image;
	cv::Mat transform_image;
	sf::IntRect offset;
	sf::Vector2i kernel_rect;

	void makeTransformImage();
	std::vector<sf::FloatRect> makeBounds();
	void makeBounds(std::vector<sf::FloatRect>& b);
	auto getTransformPreview() { return img; }

	void setImage(const sf::Texture& t)
	{
		source_image = sfml2opencv(t.copyToImage(), true);
	}
};

