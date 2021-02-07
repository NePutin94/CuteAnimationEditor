#ifndef CAE_EVENTHISTORY_H
#define CAE_EVENTHISTORY_H

#include <stack>
#include <SFML/Graphics.hpp>
#include <memory>

namespace CAE
{
    class AnimationAsset;

    struct Memento_data
    {
        Memento_data() : is_valid(false)
        {}

        bool is_valid;
        std::string asset_path;
        struct group_data
        {
            std::string name;
            int group_id;
            bool isEnable;
            bool isLooped;
            bool IsSelected;
            struct _part_data
            {
                int part_id;
                sf::Color color;
                bool IsSelected;
                sf::FloatRect rect;
            };
            std::vector<_part_data> part_data;
        };
        std::vector<group_data> groups;
    };

    class History_data
    {
        static bool makeSnapshot;
        static constexpr short size = 4;
        static int incIter();
        static int undOffset();
        static int redOffset();

    public:
        static int current; //index of the free-to-write cell
        static int iter; //index of the cell where the current state is stored
        static int redo_cap;
        static int undo_cap;
        static std::array<Memento_data, size> curr;

        History_data() = default;

        static void undo(AnimationAsset*);

        static void redo(AnimationAsset*);

        static void NeedSnapshot()
        { makeSnapshot = true; }

        static void update(const AnimationAsset*);
    };
}
#endif //CAE_EVENTHISTORY_H
