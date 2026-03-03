#ifndef __NEGOTIATOR_HPP__

#include <nlohmann/json.hpp>
#include <string.h>
#include <iostream>
#include <deque>
#include <chrono>

#define BUFFER_SIZE 10
#define TIME_SLEEP 60

using namespace std;
using json = nlohmann::json;
using namespace chrono;

struct Source_state{ // mainly for neighboors
  double _proposed_power = 0.0;
  double _covariance;

  // activation time
  steady_clock::time_point _last_active;    
};


struct Load_state{
  double _required_power = 0.0;
  steady_clock::time_point _last_active; 
}

class Negotiator{

  public:

    Negotiator() = delete;
    Negotiator(const double &c, const double &p);
    ~Negotiator();

    bool get_stab_flag() const { return _local_stab_flag; }
    double get_proposed_power() const { return _proposed_power; }
    double get_ergodic_penalty() const { return _ergodic_weight; }
    void set_weather_flag(bool f) { _weather_flag = f; } // mah, probabilmente può farlo direttamente il nodo senza che se lo gestisca il negoziatore
    void set_required_power(double p) { _required_power = p; }

    void listen(json const &input, string topic);
    json speak();
    void update_proposal();

    void update_queue(double new_power);
    void clean_nodes();

  private:

    const double _threshold = 0.01;
    bool _local_stab_flag = false;

    double _proposed_power = 0.0;
    double _required_power = 0;
    const  double &_p_max;
    const double &_covariance;

    double _weather_weight = 1.0;
    double _ergodic_weight = 1.0;

    double _residual;
    int _id;

    bool _weather_flag = true;

    map<string, Source_state> _nodes_states;
    map<string, Load_state> _loads_requests;
    deque<double> _buffer_power;
    double _temporal_sum = 0.0;
};




#endif
