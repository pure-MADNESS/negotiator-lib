#ifndef __NEGOTIATOR_HPP__

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;


class Negotiator{

  public:

    Negotiator();
    ~Negotiator();

    void set_covariance(double c) { _covariance = c; }
    double get_covariance();
    double get_proposed_power();

    void listen();
    void speak();
    void update_proposal();

  private:

    double _required_power = 0;
    double _proposed_power = 0;
    double _covariance = 0;
    
    map<int, double> _nodes_proposals;

    double _residual;

};




#endif
