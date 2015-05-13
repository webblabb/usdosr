/* A state, inherits from region */

#ifndef STATE_H
#define STATE_H

#include <string>
#include "Region.h"

class Farm;
class County;

class State : public Region
{
public:
    State(std::string id);
    State(std::string name, double x, double y);
    ~State();

    void set_flow(double flow);

    double get_flow(); //Inlined

private:
    double flow;
    bool state_initialized = false;
    bool is_set_flow = false;

    virtual void set_initialized(bool& parameter);
    virtual void all_initialized();
};

inline double State::get_flow()
{
    if(!state_initialized)
        not_initialized();

    return flow;
}

#endif // STATE_H
