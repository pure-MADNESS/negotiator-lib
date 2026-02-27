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
    iter -> second._ergodic_weight = input.at("state").at("ergodic_weight").get<double>();
  
  } else{

    Node_state new_state;
    new_state._proposed_power = input.at("state").at("proposed_power").get<double>();
    new_state._covariance = input.at("state").at("covariance").get<double>();
    new_state._ergodic_weight = input.at("state").at("ergodic_weight").get<double>();

    _nodes_states[tmp_id] = new_state;
  }
}

json Negotiator::speak(){

  json out;
  out.clear();

  out["state"]["proposed_power"] = _state._proposed_power;
  out["state"]["covariance"] = _state._covariance;
  out["state"]["ergodic_weight"] = _state._ergodic_weight;

  return out;
}

void Negotiator::update_proposal(){


  
}