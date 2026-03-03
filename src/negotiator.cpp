#include "negotiator.hpp"

Negotiator::Negotiator(const double &c, const double &p) : _covariance(c), _p_max(p) {}

Negotiator::~Negotiator(){

  cerr << "node [" << _id << "] left the party" << endl; 
}

void Negotiator::listen(json const &input, string topic){

  string tmp_id = input.at("_id").get<string>();

  if(topic.rfind("source", 0) == 0){

    auto iter_sources = _nodes_states.find(tmp_id);

    if(iter_sources != _nodes_states.end() && input.contains("state")){

      iter_sources -> second._proposed_power = input.at("state").at("proposed_power").get<double>();
      iter_sources -> second._covariance = input.at("state").at("covariance").get<double>();
    
    } else{

      Node_state new_state;
      new_state._proposed_power = input.at("state").at("proposed_power").get<double>();
      new_state._covariance = input.at("state").at("covariance").get<double>();

      _nodes_states[tmp_id] = new_state;
    }
  
  } else if(topic.rfind("load", 0) == 0){

    auto iter_loads = _loads_requests.find(tmp_id);

    if(iter_loads != _loads_requests.end() && input.contains("request")){

      iter_loads -> second = input.at("load").at("request").get<double>();
      
    } else{

      _loads_requests[tmp_id] = input.at("load").at("request").get<double>();
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

  return out;
}

void Negotiator::update_proposal(){

  double prev_proposal = _proposed_power;

  double tot_proposal = _proposed_power;
  double tot_weight = 1.0 / _covariance;

  for(auto const &[id, state] : _nodes_states){

    tot_proposal += state._proposed_power;
    tot_weight += 1.0 / state._covariance;
  }

  double err = _required_power - tot_proposal;
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