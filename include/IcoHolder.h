#pragma once
#include <SFML/Graphics.hpp>
#include <map>

namespace CAE
{
	enum class ico_t
	{
		Editor_Delete = 0,
		Editor_Add,
		Folder,
		Refresh,
		Magic,
		Save,
		Create,
		Move,
		Hand
	};
	class IcoHolder
	{
	private:
		static std::map<ico_t, sf::Texture> ico;
		sf::Texture create(std::string_view path)
		{
			sf::Texture t;
			t.loadFromFile(path.data());
			return t;
		};
	public:
		
		IcoHolder() 
		{
			ico[ico_t::Editor_Add]	  = create("data/ico/addico.png");
			ico[ico_t::Editor_Delete] = create("data/ico/trash.png");
			ico[ico_t::Folder]		  = create("data/ico/folder.png");
			ico[ico_t::Refresh]		  = create("data/ico/refresh.png");
			ico[ico_t::Magic]		  = create("data/ico/magic.png");
			ico[ico_t::Save]		  = create("data/ico/save.png");
			ico[ico_t::Create]		  = create("data/ico/create.png");
			ico[ico_t::Move]		  = create("data/ico/move.png");
			ico[ico_t::Hand] = create("data/ico/hand.png");
		}
		const sf::Texture& getTexture(ico_t type) const
		{
			return ico[type];
		}
	};
}