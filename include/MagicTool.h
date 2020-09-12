#pragma once
#include "Tool.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <imgui/include/imgui-SFML.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Texture.hpp>
namespace CAE
{
	class _MagicTool : public Tool
	{
	private:
		std::vector<cv::Rect> outputRect;
		sf::Image workImage;
		cv::Mat sfml2opencv(const sf::Image& img, bool toBGRA = false, bool fixZeroTransp = false);
		bool gray; //you should always use grayscale btw
		bool makeAllBlack;
		bool useMorph;
		bool ButtonPressed;
		std::string selected_group;
		int thresh;
		int add;
		int morph_iteration;
		int mode;
		sf::IntRect offset;
		sf::Vector2i kernel_rect;
		sf::IntRect cr;
		cv::Mat source_image;
		cv::Mat use_image;
		cv::Mat transform_image;
		sf::RectangleShape shape;
		std::shared_ptr<Group> group;
		sf::IntRect rect;
		bool once = false;

		void assetUpdated() override 
		{
			setImage(*asset->getTexture());
			makeTransformImage();
		}
	public:
		_MagicTool(EMHolder& m, const sf::Texture& t, sf::RenderWindow& window, bool useFloatMove = false) : Tool(m, t, window), ButtonPressed(false),
			add{ 0 }, thresh{ 0 }, mode{ 0 }, morph_iteration{ 0 },
			gray{ true }, makeAllBlack{ true }, useMorph{ false }
		{
			shape.setFillColor(sf::Color::Transparent);
			shape.setOutlineColor(sf::Color::Blue);
			shape.setOutlineThickness(1);
		}

		void Enable() override
		{
			EventsHolder["MagicTool"].setEnable(true);
		}

		void Disable() override
		{
			EventsHolder["MagicTool"].setEnable(false);
			ButtonPressed = false;
		}

		void setSelectedGroup(std::shared_ptr<Group> g) { group = g; }

		void Init() override
		{
			auto& eManager = EventsHolder.addEM("MagicTool", false);
			eManager.addEvent(MouseEvent::ButtonPressed(sf::Mouse::Left), [this](sf::Event& event)
				{
					ButtonPressed = true;
				});
			eManager.addEvent(MouseEvent::ButtonReleased(sf::Mouse::Left), [this](sf::Event& event)
				{
					if (once && group != nullptr)
					{
						ButtonPressed = false;
						once = false;
						cropSrc(rect, true);
						for (auto& r : makeBounds())
							group->getParts().emplace_back(std::make_shared<Part>(r, 0));
						rect = {};
						shape.setPosition(sf::Vector2f(0, 0));
						shape.setSize(sf::Vector2f(0, 0));
					}
				});
			eManager.addEvent(MouseEvent(sf::Event::MouseMoved), [this](sf::Event& event)
				{
					if (ButtonPressed)
					{
						if (!once)
						{
							once = true;
							rect.left = EventsHolder.currMousePos().x;
							rect.top = EventsHolder.currMousePos().y;
						}
						else
						{
							auto delta = EventsHolder.getDelta();
							rect.width -= delta.x;
							rect.height -= delta.y;
						}
						shape.setPosition(sf::Vector2f(rect.left, rect.top));
						shape.setSize(sf::Vector2f(rect.width, rect.height));
					}
				});
		}

		void update() override
		{
			if (ImGui::IsAnyWindowHovered())
				once = ButtonPressed = false;
		}

		void draw(sf::RenderWindow& w) override { w.draw(shape); }

		void makeTransformImage();
		std::vector<sf::FloatRect> makeBounds();
		//void makeBounds(std::vector<sf::FloatRect>& b);
		auto getTransformPreview() { return workImage; }
		nlohmann::json save2Json();
		void load4Json(const nlohmann::json& j);
		void cropSrc(sf::IntRect crop, bool rebuildSrc = false);
		void setImage(const sf::Texture& t, sf::IntRect crop = {});
	};
}

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

