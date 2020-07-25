#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>

class MagicTool
{
private:

	std::vector<cv::Rect> boundRect;
	sf::Image img;
	cv::Mat sfml2opencv(const sf::Image& img, bool toBGRA = false, bool fixZeroTransp = false);
public:
	MagicTool() :
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
	cv::Mat use_image;
	cv::Mat transform_image;
	sf::IntRect offset;
	sf::IntRect cr;
	sf::Vector2i kernel_rect;

	void makeTransformImage();
	std::vector<sf::FloatRect> makeBounds();
	//void makeBounds(std::vector<sf::FloatRect>& b);
	auto getTransformPreview() { return img; }
	nlohmann::json save2Json();
	void load4Json(const nlohmann::json& j);
	void cropSrc(sf::IntRect crop, bool rebuildSrc = false)
	{
		if (crop.width < 0)
		{
			crop.left += crop.width;
			crop.width = abs(crop.width);
		}
		if (crop.height < 0)
		{
			crop.top += crop.height;
			crop.height = abs(crop.height);
		}

		int leftX = std::max(crop.left, 0);
		int rightX = std::min(crop.left + crop.width, source_image.cols);
		int topY = std::max(crop.top, 0);
		int bottomY = std::min(crop.top + crop.height, source_image.rows);
		if (leftX < rightX && topY < bottomY) {
			cr = { leftX ,topY,rightX - leftX, bottomY - topY };
			use_image = source_image(cv::Rect(cr.left, cr.top, cr.width, cr.height));
			if (rebuildSrc)
				makeTransformImage();
			return;
		}
		else {
			use_image = source_image;
		}
	}
	void setImage(const sf::Texture& t, sf::IntRect crop = {})
	{
		source_image = sfml2opencv(t.copyToImage(), true, false);
		cropSrc(crop);
	}
};

