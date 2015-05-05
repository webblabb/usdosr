/* A state, inherits from region */

#ifndef STATE_H
#define STATE_H

#include <string>
#include <Region.h>
#include <Farm.h>

class State : public Region
{
public:
    State(double x, double y, std::string name);
    ~State();
    virtual void set_farms(const std::vector<Farm*>& in_farms);
    virtual void add_farm(Farm* in_farm);
    std::string get_name() const; //Inlined
    double get_flow() const; //Inlined
private:
    std::string name;
    double flow;
};

inline std::string State::get_name() const
{
    return name;
}

inline double State::get_flow() const
{
    return flow;
}

#endif // STATE_H
