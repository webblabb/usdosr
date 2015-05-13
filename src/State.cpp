#include "State.h"
#include "Farm.h"
#include "County.h"

State::State(std::string id) :
    Region(id)
{
    type = "state";
}

State::State(std::string id, double x, double y) :
    Region(id, x, y)
{
    type = "state";
}

State::~State() {}

void State::set_flow(double flow)
{
    this->flow = flow;
    set_initialized(is_set_flow);
}

void State::set_initialized(bool& parameter)
{
    parameter = true;
    all_initialized();
}

void State::all_initialized()
{
    if(is_set_id and is_set_position and is_set_flow)
    {
        state_initialized = true;
    }
}
