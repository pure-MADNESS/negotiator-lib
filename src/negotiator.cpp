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

  cerr << "node left the party" << endl; 
}

void Negotiator::listen(json const &input, string topic){

  // if source node
  if(topic.rfind("source", 0) == 0 || topic.rfind("accumulator", 0) == 0){

    auto iter_sources = _nodes_states.find(topic);
    auto now = steady_clock::now();

    if(iter_sources != _nodes_states.end() && input.contains("state")){

      iter_sources -> second._proposed_power = input.at("state").at("proposed_power").get<double>();
      iter_sources -> second._covariance = input.at("state").at("covariance").get<double>();
      iter_sources -> second._p_max = input.at("state").at("p_max").get<double>();
      iter_sources -> second._last_active = now;
    
    } else{

      Source_state new_state;
      new_state._proposed_power = input.at("state").at("proposed_power").get<double>();
      new_state._covariance = input.at("state").at("covariance").get<double>();
      new_state._p_max = input.at("state").at("p_max").get<double>();
      new_state._last_active = now;

      _nodes_states[topic] = new_state;
    }
  
  // if load node or surplus
  } else if(topic.rfind("load", 0) == 0 || topic.rfind("accumulator", 0) == 0){

    auto iter_loads = _loads_requests.find(topic);
    auto now = steady_clock::now();

    if(iter_loads != _loads_requests.end() && input.contains("request")){

      iter_loads -> second._required_power = input.at("request").get<double>();
      iter_loads -> second._last_active = now;
      
    } else{

      Load_state new_state;
      new_state._required_power = input.at("request").get<double>();
      new_state._last_active = now;

      _loads_requests[topic] = new_state;
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

double Negotiator::get_proposed_power(){
  
  if(_proposed_power < 0.01){
    _proposed_power = 0.0;
  }

  return _proposed_power;
}

double Negotiator::get_other_covariances(){

  double tmp = 0.0;

  for(auto const& [id, s] : _nodes_states){
    tmp += s._covariance;
  }

  return tmp;
}

double Negotiator::get_other_powers(){

  double tmp = 0.0;

  for(auto const& [id, s] : _nodes_states){
    tmp += s._p_max;
  }

  return tmp;
}

double Negotiator::get_tot_requests(){
  double tmp = 0.0;

  for(auto const& [id, load] : _loads_requests){
    tmp += load._required_power;
  }

  return tmp;
}

double Negotiator::how_many_accumulators(){

  int count = 0;

  for(auto it = _loads_requests.begin(); it != _loads_requests.end(); ) {

    if(it -> first.rfind("accumulator", 0) == 0 && it -> second._required_power < 0.1){
      count++;
    }
  }

  return count;
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

  double w = (_p_max / (_covariance + 1e-6));

  double tot_weight = w;
  for(auto const &[id, state] : _nodes_states){
    tot_weight += (state._p_max / (state._covariance + 1e-6));
  }

  double target = 0.0;

  if(tot_weight > 1e-6){
    target = (w / tot_weight) * total_demand;
  } else{
    target = 0.0;
  }

  if(_weather_flag){
    // weight = weight * _weather_weight;
  }

  double correction = 0.25 * (target - _proposed_power);
  _proposed_power += correction;
  _proposed_power = std::clamp(_proposed_power, 0.0, _p_max);

  if(abs(prev_proposal - _proposed_power) < _threshold){

    if(!_local_stab_flag){
      update_queue(_proposed_power);
    }

    _local_stab_flag = true;
  } else{

    _local_stab_flag = false;
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
  double ergodic_err = 0.0;
  double m = 0.0;
  double threshold = _p_max * 0.2;

  if(_buffer_power.empty()){

    _ergodic_weight = 1.0;
  
  } else{

    m = _temporal_sum / _buffer_power.size();
    ergodic_err = abs(m - _proposed_power);

    if(ergodic_err > threshold){

      // in base all'errore ergodico, modifichiamo la R delle misurazioni in modo da fidarci sempre meno e alzare la covarianza
      // first try: errore quadratico
      _ergodic_weight = 1.0 + (pow(ergodic_err - threshold, 2) * 10.0);
    } else{

      _ergodic_weight = 1.0;
    }
  }

  m = _weather_mean;
  ergodic_err = _proposed_power - m;
  
  // if i propose more power than what it's going to be
  if(ergodic_err > threshold){

    _weather_weight = 1.0 + (pow(ergodic_err - threshold, 2) * 10.0);
  
  } else if(ergodic_err < -threshold){ // i propose less power tha what it's going to be

    _weather_weight = max(0.8, 1.0 - (pow(ergodic_err - threshold, 2)));
  
  } else{

    _weather_weight = 1.0;
  }


}
