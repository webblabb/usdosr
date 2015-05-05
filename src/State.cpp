#include <State.h>

State::State(double x, double y, std::string name) :
    Region(x, y), name(name) {}

State::~State() {}

void State::set_farms(const std::vector<Farm*>& in_farms)
{
    member_farms = in_farms;
    for(auto farm : member_farms)
    {
        farm->set_parent_state(this);
    }
}

//Adds one single farm that belongs to this state by passing a pointer to it.
void State::add_farm(Farm* in_farm)
{
    member_farms.push_back(in_farm);
    in_farm->set_parent_state(this);
}
