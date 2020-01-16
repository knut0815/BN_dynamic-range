// Probabilisitic integrate-and-fire model with Poissonian external input of mean rate h
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <list>
#include <random>
#include <algorithm>

//assumes external POPULATION WITH rate h -> 1-probability for zero input
inline double lambda(double h){
  double dt=1;
  return 1.0-exp(-h*dt);
}

int main(int argc, char* argv[]){
  std::string path;  // output path
  unsigned N = 1e0;  // system size 
  double   T = 1e0;  // time steps 
  double   h = 1.00; // external drive
  double   m = 0;    // synaptic strength
  double   p_edge = 0.01; // probability of edge in ER network   
  unsigned seed=1000;

  std::stringstream help;
  help << "usage:\n";
  help << "     -N : number of neurons     (N=1e4         )\n";
  help << "     -T : number of time steps  (T=1e7 ms goal )\n";
  help << "     -h : external input (h<1)\n";
  help << "     -m : synaptic strength\n";
  help << "     -p : edge probability of random graph\n";
  help << "     -s : seed\n";
  int valid_args=0;
  for(unsigned i=0; i<argc; i++){
    if(std::string(argv[i])=="-N"){ N    = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-T"){ T    = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-h"){ h    = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-m"){ m    = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-s"){ seed = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-p"){ p_edge = atof(argv[i+1]); valid_args+=1;}
  }
  if(valid_args<6){std::cout << "not enough arguments\n" << help.str() << std::endl; exit(1);}
  
  std::vector<int> neuron(N,0);
  std::vector<int> stimulus(N,0);

  //random numbers
  std::mt19937                           mt(seed);   
  std::uniform_real_distribution<double> uni_01(0,1);  // r=uni_01(mt)

  //generate_ER
  std::vector<std::vector<unsigned>> outgoing(N, std::vector<unsigned>());
  std::vector<std::vector<unsigned>> incoming(N, std::vector<unsigned>());
  for(unsigned n=0;n<neuron.size(); n++){
    for(unsigned nn=0;nn<neuron.size(); nn++){
      if(n!=nn){
        if(uni_01(mt)<p_edge){
          outgoing[n ].push_back(nn);
          incoming[nn].push_back(n );
        }
      }
    }
  }

  //linear activation function:
  std::vector<double> w(N,0);
  for(unsigned n=0; n<w.size(); n++) w[n] = m/incoming[n].size();

  //loop over time
  long unsigned T_therm = int(1.0/lambda(h));
  std::cout << "#T_therm="<<T_therm <<"\n";
  std::cout << "#simulation\n";

  //observable
  long unsigned time   = 0;
  double avg_activity  = 0;
  double avg_activity2 = 0;
  //loop over avalanche onsets
  int num_active=0, num_active_old=0;
  while(time < T+T_therm){
    num_active_old = num_active;
    // internal signal processing (branching process)
    for(unsigned n=0;n<neuron.size(); n++){
      //random external activation with probability h
      if(uni_01(mt) < lambda(h)) { 
        stimulus[n] = 1; 
      } 
      // internal probabilistic integrate and fire mechanism
      else{
        int num_active_neigbors = 0;
        for(unsigned i=0; i< incoming[n].size(); i++) num_active_neigbors += neuron[incoming[n][i]];
        if(uni_01(mt) < w[n]*num_active_neigbors) { 
          stimulus[n] = 1; 
        }
      }
    }
    // integration over internal activation and external activation
    // num_active remains the number of initially active neurons that lead to this activation
    num_active = 0;
    for(unsigned n=0; n<neuron.size(); n++){
      if(stimulus[n]>0){
         neuron[n]=1; 
         num_active +=1;
         stimulus[n]=0;
      }
      else if(neuron[n]==1){ 
        neuron[n]=0; 
      }
    }
    time++;
    if(time > T_therm){
      avg_activity  += num_active;
      avg_activity2 += num_active*num_active;
    }
  }
  avg_activity /= static_cast<double>(T);
  avg_activity2/= static_cast<double>(T);
  double err_activity  = sqrt((avg_activity2 - avg_activity*avg_activity)/(static_cast<double>(T)-1));
  std::cout << "#seed statistic  avg_activity  avg_activity2  error(activity)\n";
  std::cout << seed << " ";
  std::cout << T << " ";
  std::cout << avg_activity  << " ";
  std::cout << avg_activity2 << " ";
  std::cout << err_activity  << " ";
  std::cout << "\n";
}
