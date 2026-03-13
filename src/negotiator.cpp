/*

  _   _                  _   _       _                ____ _               
 | \ | | ___  __ _  ___ | |_(_) __ _| |_ ___  _ __   / ___| | __ _ ___ ___ 
 |  \| |/ _ \/ _` |/ _ \| __| |/ _` | __/ _ \| '__| | |   | |/ _` / __/ __|
 | |\  |  __/ (_| | (_) | |_| | (_| | || (_) | |    | |___| | (_| \__ \__ \
 |_| \_|\___|\__, |\___/ \__|_|\__,_|\__\___/|_|     \____|_|\__,_|___/___/
             |___/                                                         

*/

#include "negotiator.hpp"

Negotiator::Negotiator(double c, double p) : _covariance(c), _p_max(p) {}

Negotiator::~Negotiator(){

  cerr << "node [" << _id << "] left the party" << endl; 
}

void Negotiator::listen(json const &input, string topic){

  string tmp_id = input.at("agent_id").get<string>();

  // if source node
  if(topic.rfind("source", 0) == 0){

    auto iter_sources = _nodes_states.find(tmp_id);
    auto now = steady_clock::now();

    if(iter_sources != _nodes_states.end() && input.contains("state")){

      iter_sources -> second._proposed_power = input.at("state").at("proposed_power").get<double>();
      iter_sources -> second._covariance = input.at("state").at("covariance").get<double>();
      iter_sources -> second._last_active = now;
    
    } else{

      Source_state new_state;
      new_state._proposed_power = input.at("state").at("proposed_power").get<double>();
      new_state._covariance = input.at("state").at("covariance").get<double>();
      new_state._last_active = now;

      _nodes_states[tmp_id] = new_state;
    }
  
  // if load node
  } else if(topic.rfind("load", 0) == 0){

    auto iter_loads = _loads_requests.find(tmp_id);
    auto now = steady_clock::now();

    if(iter_loads != _loads_requests.end() && input.contains("request")){

      iter_loads -> second._required_power = input.at("load").at("request").get<double>();
      iter_loads -> second._last_active = now;
      
    } else{

      Load_state new_state;
      new_state._required_power = input.at("load").at("request").get<double>();
      new_state._last_active = now;

      _loads_requests[tmp_id] = new_state;
    }

  } else{

    return;
  }
}

json Negotiator::speak(){

  json out;
  out.clear();

  out["state"]["proposed_power"] = _proposed_power;
  out["state"]["covariance"] = _covariance;
  out["state"]["p_max"] = _p_max;

  return out;
}

void Negotiator::clean_nodes(){

  auto now = std::chrono::steady_clock::now();

  // clean sources
  for (auto it = _nodes_states.begin(); it != _nodes_states.end(); ) {
      std::chrono::duration<double> diff = now - it->second._last_active;

      if (diff.count() > TIME_SLEEP) {

          it = _nodes_states.erase(it); // sleeping node

      } else {
          ++it;
      }
  }

  // clean loads
  for (auto it = _loads_requests.begin(); it != _loads_requests.end(); ) {
      std::chrono::duration<double> diff = now - it->second._last_active;

      if (diff.count() > TIME_SLEEP) {

          it = _loads_requests.erase(it);

      } else {
          ++it;
      }
  }
}

void Negotiator::update_proposal(){

  clean_nodes();

  double prev_proposal = _proposed_power;

  double total_demand = 0.0;
  for(auto const &[id, load] : _loads_requests){
    total_demand += load._required_power;
  }

  double tot_proposal = _proposed_power;
  double tot_weight = 1.0 / _covariance;

  for(auto const &[id, state] : _nodes_states){

    tot_proposal += state._proposed_power;
    tot_weight += 1.0 / state._covariance;
  }

  double err = total_demand - tot_proposal;
  double weight = (1.0 / _covariance);

  if(_weather_flag){
    // weight = weight * _weather_weight;
  }

  double correction = (weight / tot_weight) * err;
  _proposed_power += correction;
  _proposed_power = std::clamp(_proposed_power, 0.0, _p_max);

  if(abs(prev_proposal - _proposed_power) < _threshold){

    _local_stab_flag = true;
  }

}


void Negotiator::update_queue(double new_power){

  _buffer_power.push_back(new_power);
  _temporal_sum += new_power;

  if(_buffer_power.size() > BUFFER_SIZE){

    _temporal_sum -= _buffer_power.front();
    _buffer_power.pop_front();  // fifo like
  }

  // ergodicity check
  if(_buffer_power.empty()){

    _ergodic_weight = 1.0;
  
  } else{

    double m = _temporal_sum / _buffer_power.size();
    double ergodic_err = abs(m - _proposed_power);

    double threshold = _p_max * 0.2;
    if(ergodic_err > threshold){

      // in base all'errore ergodico, modifichiamo la R delle misurazioni in modo da fidarci sempre meno e alzare la covarianza
      // first try: errore quadratico
      _ergodic_weight = 1.0 + (pow(ergodic_err - threshold, 2) * 10.0);
    }
  }

}