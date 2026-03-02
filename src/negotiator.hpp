#ifndef __NEGOTIATOR_HPP__

#include <nlohmann/json.hpp>
#include <string.h>
#include <iostream>
#include <deque>

#define BUFFER_SIZE 10

using namespace std;
using json = nlohmann::json;

struct Node_state{
  double _proposed_power = 0.0;
  double _covariance = 0.0;
};


class Negotiator{

  public:

    Negotiator(){ }
    Negotiator(double c);
    ~Negotiator();

    void set_covariance(double c) { _state._covariance = c; }
    void set_required_power(double p) { _required_power = p; }
    double get_covariance() const { return _state._covariance; }
    double get_proposed_power() const { return _state._proposed_power; }
    double get_ergodic_penalty() const { return _ergodic_weight; }
    void set_weather_flag(bool f) { _weather_flag = f; } // mah, probabilmente può farlo direttamente il nodo senza che se lo gestisca il negoziatore

    void listen(json const &input, string topic);
    json speak();
    void update_proposal();

    void update_queue(double new_power);

  private:

    Node_state _state;
    double _required_power = 0;
    double _p_max = 0.0;
    double _weather_weight = 1.0;
    double _ergodic_weight = 1.0;

    double _residual;
    int _id;

    bool _weather_flag = true;

    map<string, Node_state> _nodes_states;
    deque<double> _buffer_power;
    double _temporal_sum = 0.0;
};




#endif
