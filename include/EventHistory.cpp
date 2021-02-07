#include "../include/AnimationAsset.h"
#include "EventHistory.h"

std::array<CAE::Memento_data, CAE::History_data::size> CAE::History_data::curr{};
bool CAE::History_data::makeSnapshot = false;
int CAE::History_data::current = -1;
int CAE::History_data::iter = -1;
int CAE::History_data::undo_cap = -1;
int CAE::History_data::redo_cap = 0;

void CAE::History_data::update(const AnimationAsset* asset)
{
    if(makeSnapshot)
    {
        curr[incIter()] = asset->createMemento();
        makeSnapshot = false;
    }
}

void CAE::History_data::undo(CAE::AnimationAsset* asset)
{
    if(auto i = undOffset(); i >= 0)
    {
        if(!asset->reinstateMemento(curr[i])) redOffset();
        else
            Console::AppLog::addLog_("Undo success, iter %i, redo_cap %i, undo_cap %i", Console::system, iter, redo_cap, undo_cap);
    }
}

void CAE::History_data::redo(CAE::AnimationAsset* asset)
{
    if(auto i = redOffset(); i >= 0)
    {
        if(!asset->reinstateMemento(curr[i])) undOffset();
        else
            Console::AppLog::addLog_("Redo success, iter %i, redo_cap %i, undo_cap %i", Console::system, iter, redo_cap, undo_cap);
    }
}

int CAE::History_data::incIter()
{
    undo_cap = undo_cap >= CAE::History_data::size - 1 ? CAE::History_data::size - 1 : ++undo_cap;
    redo_cap = 0;

    if(current + 1 >= CAE::History_data::size)
    {
        if(iter + 1 < CAE::History_data::size)
            iter = current = iter + 1;
        else
        {
            iter = current = 0;
        }

        return current;
    }
    iter++;
    return ++current;
}

int CAE::History_data::undOffset()
{
    if(undo_cap <= 0)
        return -1;
    ++redo_cap;
    --undo_cap;
    if(iter == 0)
        return iter = CAE::History_data::size - 1;
    return --iter;
}

int CAE::History_data::redOffset()
{
    if(redo_cap <= 0)
        return -1;
    --redo_cap;
    ++undo_cap;
    ++iter;
    if(iter >= CAE::History_data::size) iter = 0;
    return iter;
}