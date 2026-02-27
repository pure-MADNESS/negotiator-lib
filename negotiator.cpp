#include "negotiator.hpp"

Negotiator::Negotiator(double c){

  _state._covariance = c;
}

Negotiator::~Negotiator(){

  cerr << "node [" << _id << "] left the party" << endl; 
}

void Negotiator::listen(){




}

void Negotiator::speak(){



}

void Negotiator::update_proposal(){


  
}