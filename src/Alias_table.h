/* Discrete probability distribution with O(1) lookup time.
Initialization time is O(n).
Template classes need to be defined completely in the .h file. */

#ifndef ATABLE_H
#define ATABLE_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <deque>
#include <random>
#include <chrono>
#include <cmath>

template <typename T>
class Alias_table
{
public:
    Alias_table();
    Alias_table(std::vector<T> in_outcomes, std::vector<double> in_probabilities);
    ~Alias_table();

    void init(std::vector<T> in_outcomes, std::vector<double> in_probabilities);
    void clear();
    T generate(); //Returns one random draw from the alias table.

private:
    int n_elements;
    std::vector<double> prob_v;
    std::vector<T> prob_outcomes;
    std::deque<T> small, large;
    std::unordered_map<T, double> scaled_prob_map;
    std::unordered_map<T, T> alias;
    std::mt19937 MT_generator;
    std::uniform_real_distribution<double> unif_distribution;

    double rand(); //Return random number [0.0, 1.0]
};


template<typename T>
Alias_table<T>::Alias_table() {}

template<typename T>
Alias_table<T>::Alias_table(std::vector<T> in_outcomes, std::vector<double> in_probabilities) :
    MT_generator(std::chrono::system_clock::now().time_since_epoch().count()),
    unif_distribution(0.0, 1.0)
{
    init(in_outcomes, in_probabilities);
}

template<typename T>
void Alias_table<T>::init(std::vector<T> in_outcomes, std::vector<double> in_probabilities)
{
    n_elements = in_probabilities.size();

    for(size_t i = 0; i < in_outcomes.size(); i++)
    {
        double scaled_p = in_probabilities[i] * n_elements;
        scaled_prob_map.emplace(in_outcomes[i], scaled_p);
    }

    for(auto it = scaled_prob_map.begin(); it != scaled_prob_map.end(); it++)
    {
        if(it->second < 1.0)
            small.push_back(it->first);
        else
            large.push_back(it->first);
    }

    while(!small.empty() and !large.empty())
    {
        T s = small.front();
        small.pop_front();
        T l = large.front();
        large.pop_front();

        alias[s] = l;

        prob_outcomes.push_back(s);
        prob_v.push_back(scaled_prob_map[s]);

        double new_l_prob = (scaled_prob_map[l] + scaled_prob_map[s]) - 1.0;
        scaled_prob_map[l] = new_l_prob;
        if(new_l_prob < 1.0)
            small.push_front(l);
        else
            large.push_front(l);
    }

    while(!large.empty())
    {
        prob_outcomes.push_back(large.front());
        prob_v.push_back(1.0);
        large.pop_front();
    }

    while(!small.empty())
    {
        prob_outcomes.push_back(small.front());
        prob_v.push_back(1.0);
        small.pop_front();
    }
}

template<typename T>
void Alias_table<T>::clear()
{
    n_elements = 0;
    prob_v.clear();
    prob_outcomes.clear();
    small.clear();
    large.clear();
    scaled_prob_map.clear();
    alias.clear();
}

template<typename T>
T Alias_table<T>::generate()
{
    int index = int(std::floor(prob_v.size() * rand()));
    if(rand() < prob_v[index])
        return prob_outcomes[index];
    else
        return alias[prob_outcomes[index]];
}

template<typename T>
double Alias_table<T>::rand()
{
    return unif_distribution(MT_generator);
}

template<typename T>
Alias_table<T>::~Alias_table(){}

#endif // ATABLE_H
