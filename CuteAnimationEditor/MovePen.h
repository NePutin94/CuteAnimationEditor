#pragma once
#include "Tool.h"
#include <cmath>

namespace CAE
{
	class MovePen : public Tool
	{
	private:
		std::shared_ptr<Part> selectedPart;
		std::shared_ptr<Group> selectedGroup;
		std::shared_ptr<Part> lastSelected;
		ScaleNode* selectedNode;
		bool& useFloat;

	public:
		MovePen(EventManager& m, const sf::Texture& t, sf::RenderWindow& window, bool& useFloatMove) : Tool(m, t, window), useFloat(useFloatMove), selectedNode(nullptr) {}

		void update(std::shared_ptr<AnimationAsset> data) override
		{
			for (auto group : *data)
			{
				if (group->isVisible())
				{
					for (std::shared_ptr<Part> elem : *group)
					{
						////////////////////////////////////MOVING(FLOAT OFFSET)//////////////////////////////////////////
						if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
						{
							if (auto rect = elem->getRect(); elem->getRect().contains(eManager.currMousePos()) && ((selectedPart != nullptr) ? selectedPart == elem : true))
							{
								selectedPart = elem;
								selectedGroup = group;
								if (useFloat)
								{
									auto delta = eManager.worldMouseDelta();
									rect.left -= delta.x;
									rect.top -= delta.y;
									elem->setRect(rect);
								}
								else
								{
									static sf::Vector2f delta2;
									delta2 += eManager.worldMouseDelta();
									auto delta = -(sf::Vector2i)delta2;
									int factor = 1;
									if (abs(delta.x) > factor || abs(delta.y) > factor)
									{
										if (abs(delta.x) > factor)
											rect.left = round(rect.left) + delta.x;
										if (abs(delta.y) > factor)
											rect.top = round(rect.top) + delta.y;
										delta2 = { 0,0 };
									}
									elem->setRect(rect);
								}

								selectedPart->changeColor(sf::Color::Green);
								//lasSelectedPart may no longer be in the array
								//But resource will not be deleted, in this case we will just change the color 
								//for the essentially 'dead' object(which is still valid)
								//it will be released when the lastSelected pointer changes to selectetPart
								//If it was not deleted, we will still just change the color
								if (lastSelected != nullptr && lastSelected != selectedPart)
								{
									lastSelected->changeColor(sf::Color::Red);
									//selectedGroup.reset();
								}
								lastSelected = selectedPart;
								
							}
						}
						else
						{
							if (selectedPart != nullptr)
								lastSelected = selectedPart;
							selectedPart.reset();
						}

						////////////////////////////////////SCALE//////////////////////////////////////////
						static int selectedPoint = -1;
						if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
						{
							for (auto& p : elem->getNode())
								if ((p.getGlobalBounds().contains(eManager.currMousePos()) /*&& !pointSelected*/) || (/*pointSelected &&*/ selectedPoint == p.side && &p == selectedNode))
								{
									//pointSelected = true;
									selectedPoint = p.side;
									selectedNode = &p;
									auto rect = elem->getRect();
									sf::Vector2f value = { 0,0 };
									if (!useFloat)
									{

										static sf::Vector2f delta2;
										delta2 += eManager.worldMouseDelta();
										sf::Vector2i delta = -(sf::Vector2i)delta2;
										int factor = 0.2;
										if (abs(delta.x) > factor || abs(delta.y) > factor)
										{
											value.x += ceil(delta.x);
											value.y += ceil(delta.y);
											delta2 = { 0,0 };
										}
										rect = round(rect);
									}
									else
										value = -eManager.worldMouseDelta();

									switch (p.side)
									{
									case 0:
										elem->setRect(sf::FloatRect(rect.left, rect.top + value.y, rect.width, rect.height - value.y));
										break;
									case 1:
										elem->setRect(sf::FloatRect(rect.left, rect.top, rect.width + value.x, rect.height));
										break;
									case 2:
										elem->setRect(sf::FloatRect(rect.left, rect.top, rect.width, rect.height + value.y));
										break;
									case 3:
										elem->setRect(sf::FloatRect(rect.left + value.x, rect.top, rect.width - value.x, rect.height));
										break;
									}
								}
						}
					}
				}
			}
		}

		/*void handleEvenet(sf::Event& e) override
		{
		}*/

		void draw() override
		{
		}
	};
}
