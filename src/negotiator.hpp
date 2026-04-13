/*

  _   _                  _   _       _                ____ _               
 | \ | | ___  __ _  ___ | |_(_) __ _| |_ ___  _ __   / ___| | __ _ ___ ___ 
 |  \| |/ _ \/ _` |/ _ \| __| |/ _` | __/ _ \| '__| | |   | |/ _` / __/ __|
 | |\  |  __/ (_| | (_) | |_| | (_| | || (_) | |    | |___| | (_| \__ \__ \
 |_| \_|\___|\__, |\___/ \__|_|\__,_|\__\___/|_|     \____|_|\__,_|___/___/
             |___/                                                         

*/

#ifndef __NEGOTIATOR_HPP__

#include <nlohmann/json.hpp>
#include <string.h>
#include <iostream>
#include <deque>
#include <chrono>

#define BUFFER_SIZE 10
#define TIME_SLEEP 10

using namespace std;
using json = nlohmann::json;
using namespace chrono;

struct Source_state{ // mainly for neighboors
  double _proposed_power = 0.0;
  double _covariance;
  double _p_max;

  // activation time
  steady_clock::time_point _last_active;    
};


struct Load_state{
  double _required_power = 0.0;
  steady_clock::time_point _last_active; 
};

class Negotiator{

  public:

    Negotiator() { };
    Negotiator(double c, double p);
    ~Negotiator();

    void set_cov(double c) { _covariance = c; }
    void set_pmax(double p) { _p_max = p;} // 98% sure to not erogate more than what generated
    double get_pmax() const { return _p_max; }

    bool get_stab_flag() const { return _local_stab_flag; }
    double get_proposed_power() const { return _proposed_power; }
    double get_ergodic_penalty() const { return _ergodic_weight; }
    double get_weather_penalty() const { return _weather_weight; }
    double get_other_covariances();
    double get_other_powers();
    double get_tot_requests();
    double how_many_accumulators();
    
    void set_weather_flag(bool f) { _weather_flag = f; } // mah, probabilmente può farlo direttamente il nodo senza che se lo gestisca il negoziatore
    void set_required_power(double p) { _required_power = p; }
    void set_weather_mean(double weather_mean) { _weather_mean = weather_mean; }

    void listen(json const &input, string topic);
    json speak();

    /**
     * 
     * @brief CONSENSUS BALANCING:
     * 
     * Each node computes its own proposed power as the total demand P_demand scaled by a factor defined by the ratio between its own weight (1/_covariance) and the sum of the weights of all the other source nodes.
     * 
     * In this way, the total proposed power P_tot, given by the summation of all proposals, is given by the total demand P_demand scaled by the ratio between the summation of all weights and the summation of the same weights: the total demand scaled by 1.
     * 
     */
    void update_proposal();

    void update_queue(double new_power);
    void clean_nodes();

  private:

    const double _threshold = 0.01;
    bool _local_stab_flag = false;

    double _proposed_power = 0.0;
    double _required_power = 0;
    double _p_max = 0.0;
    double _covariance = 0.1;

    double _weather_mean = 0.0;

    double _weather_weight = 1.0;
    double _ergodic_weight = 1.0;

    double _residual;

    bool _weather_flag = true;

    map<string, Source_state> _nodes_states;
    map<string, Load_state> _loads_requests;
    deque<double> _buffer_power;
    double _temporal_sum = 0.0;
};




#endif
