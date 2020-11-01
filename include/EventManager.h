#pragma once
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp> 
#include <algorithm>
#include <functional>
#include <map>
#include <iostream>
namespace CAE
{
	class Application;
	struct KBoardEvent
	{
		sf::Event::EventType type = (sf::Event::EventType)0;
		sf::Keyboard::Key key = (sf::Keyboard::Key)0;
		//KBoardEvent() : type(), key() {}

		KBoardEvent(sf::Event::EventType type, sf::Keyboard::Key key) :type(type), key(key) {}
		KBoardEvent(sf::Keyboard::Key key) : key(key) {}

		bool operator==(const KBoardEvent& other) const { return key == other.key && type == other.type; }
		bool operator<(const KBoardEvent& o)  const {
			return type < o.type || (type == o.type && key < o.key);
		}
		static KBoardEvent KeyPressed(sf::Keyboard::Key k)
		{
			return KBoardEvent{ sf::Event::KeyPressed,k };
		}
		static KBoardEvent KeyReleased(sf::Keyboard::Key k)
		{
			return KBoardEvent{ sf::Event::KeyReleased,k };
		}
	};

	struct MouseEvent
	{
		sf::Event::EventType type = (sf::Event::EventType)0;
		sf::Mouse::Button    key = (sf::Mouse::Button)0;

		MouseEvent(sf::Event::EventType type, sf::Mouse::Button b) : key(b), type(type) {}
		MouseEvent(sf::Event::EventType type) : type(type) {}
		MouseEvent(sf::Mouse::Button b) : key(b) {}

		bool operator==(const MouseEvent& other) const { return key == other.key && type == other.type; }
		bool operator<(const MouseEvent& o)  const {
			return (type < o.type || (type == o.type && key < o.key));
		}
		static MouseEvent ButtonPressed(sf::Mouse::Button b)
		{
			return MouseEvent{ sf::Event::MouseButtonPressed, b };
		}
		static MouseEvent ButtonReleased(sf::Mouse::Button b)
		{
			return MouseEvent{ sf::Event::MouseButtonReleased, b };
		}
		static MouseEvent WheelScrolled()
		{
			return MouseEvent{ sf::Event::MouseWheelScrolled, sf::Mouse::ButtonCount };
		}
	};

	struct MultiEvent
	{
		KBoardEvent k;
		MouseEvent m;
		bool operator==(const MultiEvent& other) const { return k == other.k && m == other.m; }
		bool operator<(const MultiEvent& o)  const {
			return k < o.k || (k == o.k && m < o.m);
		}
	};

	class EventManager
	{
	private:
		std::multimap<KBoardEvent, std::function<void(sf::Event&)>> KeyboardEvents;
		std::multimap<MouseEvent, std::function<void(sf::Event&)>> MouseEvents;
		std::multimap<MultiEvent, std::function<void()>> MultiEvents;
		std::vector<std::function<void(sf::Event&)>> cutomManagers;
		typedef  void(Application::* AppFunc)(sf::Event&);
		typedef  void(Application::* AppFuncInput)();
		bool enable;
	public:
		EventManager(bool enable = true) : enable(enable) {};
		void addEvent(KBoardEvent e, std::function<void(sf::Event&)> callback)
		{
			KeyboardEvents.emplace(e, callback);
		}
		void addEvent(KBoardEvent e, AppFunc m, Application* appInstance)
		{
			addEvent(e, std::bind(m, appInstance, std::placeholders::_1));
		}

		void addEvent(MouseEvent e, std::function<void(sf::Event&)> callback)
		{
			MouseEvents.emplace(e, callback);
		}
		void addEvent(MouseEvent e, AppFunc m, Application* appInstance)
		{
			addEvent(e, std::bind(m, appInstance, std::placeholders::_1));
		}

		void addInput(MultiEvent e, std::function<void()> callback)
		{
			MultiEvents.emplace(e, callback);
		}
		void addInput(MultiEvent e, AppFuncInput m, Application* appInstance)
		{
			addInput(e, std::bind(m, appInstance));
		}

		void addCustomManager(std::function<void(sf::Event&)> func)
		{
			cutomManagers.emplace_back(func);
		}

		void updateInput()
		{
			for (auto& me : MultiEvents)
			{
				auto& kEvent = me.first.k;
				auto& mEvent = me.first.m;
				if ((sf::Keyboard::isKeyPressed(kEvent.key)) && (sf::Mouse::isButtonPressed(mEvent.key)))
					me.second();
			}
		}

		void updateEvent(sf::Event& e)
		{
			switch (e.type)
			{
			case sf::Event::KeyPressed:
			{
				auto range = KeyboardEvents.equal_range(KBoardEvent{ e.type, e.key.code });
				for (auto it = range.first; it != range.second; ++it)
					it->second(e);
			}
			break;
			case sf::Event::KeyReleased:
			{
				auto range = KeyboardEvents.equal_range(KBoardEvent{ e.type, e.key.code });
				for (auto it = range.first; it != range.second; ++it)
					it->second(e);
			}
			break;
			case sf::Event::MouseButtonPressed:
			{
				auto range = MouseEvents.equal_range(MouseEvent{ e.type, e.mouseButton.button });
				for (auto it = range.first; it != range.second; ++it)
					it->second(e);
			}
			break;
			case sf::Event::MouseButtonReleased:
			{
				auto range = MouseEvents.equal_range(MouseEvent{ e.type, e.mouseButton.button });
				for (auto it = range.first; it != range.second; ++it)
					it->second(e);
			}
			break;
			case sf::Event::MouseWheelScrolled:
			{
				auto range = MouseEvents.equal_range(MouseEvent{ e.type });
				for (auto it = range.first; it != range.second; ++it)
					it->second(e);
			}
			break;
			case sf::Event::MouseMoved:
			{
				auto range = MouseEvents.equal_range(MouseEvent{ e.type });
				for (auto it = range.first; it != range.second; ++it)
					it->second(e);
			}
			break;
			}
			/*for (auto& func : cutomManagers)
				func(e);*/
		}

		void setEnable(bool e) { enable = e; }
		bool isEneable() const { return enable; }
	};

	class EMHolder
	{
	private:
		std::map<std::string, EventManager> events;
		sf::Vector2f curr_mpos_f;
		sf::Vector2f prev_mpos_f;
		sf::Vector2i oldPos;
		sf::Vector2f oldpos_f;
		sf::Vector2i deltaPos;
		sf::Vector2f deltaPos_f;
		sf::Vector2f newPos_f;
	public:
		auto& addEM(std::string_view name, bool Enable = true)
		{
			return events.emplace(name, EventManager(Enable)).first->second;
		}
		float zoom = 1.f;
		auto& operator[](std::string name)
		{
			return events.find(name)->second;
		}

		void updateEvent(sf::Event& e, const sf::RenderWindow& w)
		{
			switch (e.type)
			{
			case sf::Event::MouseWheelScrolled:
			{
			}
			break;
			case sf::Event::MouseMoved:
			{
				const sf::Vector2i newPos = sf::Vector2i(e.mouseMove.x, e.mouseMove.y);
				newPos_f = w.mapPixelToCoords(newPos);

				deltaPos = oldPos - newPos;
				deltaPos_f = oldpos_f - newPos_f;

				//deltaPos.x *= zoom;
				//deltaPos.y *= zoom;

				//deltaPos_f.x *= zoom;
				//deltaPos_f.y *= zoom;

				oldPos = newPos;
				oldpos_f = newPos_f;
			}
			break;
			}
			for (auto& [_, em] : events)
				if (em.isEneable())
					em.updateEvent(e);
		}

		sf::Vector2i getDelta() { return deltaPos; }

		sf::Vector2f getDelta_F() { return deltaPos_f; }

		float getZoom() { return zoom; }

		void updateInput()
		{
			for (auto& [_, em] : events)
				if (em.isEneable())
					em.updateInput();
		}

		void updateMousePosition(sf::RenderWindow& w, sf::View& v)
		{
			prev_mpos_f = curr_mpos_f;
			curr_mpos_f = w.mapPixelToCoords(sf::Mouse::getPosition(w), v);
		}

		void updatecurr(sf::RenderWindow& w, sf::View& v)
		{
			curr_mpos_f = w.mapPixelToCoords(sf::Mouse::getPosition(w), v);
		}
		sf::Vector2f get()
		{
			return curr_mpos_f;
		}
		sf::Vector2f worldMouseDelta()
		{
			return prev_mpos_f - curr_mpos_f;
		}

		sf::Vector2f currMousePos()
		{
			return newPos_f;
		}
	};
};
