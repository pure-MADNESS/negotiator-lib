#ifndef __NEGOTIATOR_HPP__

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

struct Node_state{
  double _proposed_power = 0.0;
  double _covariance = 0.0;
  double _ergodic_weight = 0.0;
};


class Negotiator{

  public:

    Negotiator(){ }
    Negotiator(double c);
    ~Negotiator();

    void set_covariance(double c) { _state._covariance = c; }
    double get_covariance() const { return _state._covariance; }
    double get_proposed_power() const { return _state._proposed_power; }

    void listen();
    void speak();
    void update_proposal();

  private:

    Node_state _state;
    double _required_power = 0;
    double _residual;
    int _id;

    map<int, Node_state> _nodes_states;

};




#endif
