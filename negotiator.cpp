#include "negotiator.hpp"

Negotiator::Negotiator(double c){

  set_covariance(c);
}

Negotiator::~Negotiator(){

  cerr << "node [" << _id << "] left the party" << endl; 
}

void Negotiator::listen(json const &input){

  string tmp_id = input.at("_id").get<string>();
  bool presence_flag = false;

  auto iter = _nodes_states.find(tmp_id);

  if(iter != _nodes_states.end()){
    iter -> second._proposed_power = input.at("state").at("proposed_power").get<double>();
    iter -> second._proposed_power = input.at("state").at("covariance").get<double>();
    // iter -> second._ergodic_weight = input.at("state").at("ergodic_weight").get<double>();
  
  } else{

    Node_state new_state;
    new_state._proposed_power = input.at("state").at("proposed_power").get<double>();
    new_state._covariance = input.at("state").at("covariance").get<double>();
    // new_state._ergodic_weight = input.at("state").at("ergodic_weight").get<double>();

    _nodes_states[tmp_id] = new_state;
  }
}

json Negotiator::speak(){

  json out;
  out.clear();

  out["state"]["proposed_power"] = _state._proposed_power;
  out["state"]["covariance"] = _state._covariance;
  // out["state"]["ergodic_weight"] = _state._ergodic_weight;

  return out;
}

void Negotiator::update_proposal(){

  double tot_proposal = _state._proposed_power;
  double tot_weight = 1.0 / _state._covariance;

  for(auto const &[id, state] : _nodes_states){

    tot_proposal += state._proposed_power;
    tot_weight += 1.0 / state._covariance;
  }

  double err = _required_power - tot_proposal;
  double weight = (1.0 / _state._covariance) * _ergodic_weight;

  if(_weather_flag){
    weight = weight * _weather_weight;
  }

  double correction = (weight / tot_weight) * err;
  _state._proposed_power += correction;
  _state._proposed_power = std::clamp(_state._proposed_power, 0.0, _p_max);

}
