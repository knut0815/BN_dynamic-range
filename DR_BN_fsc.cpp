// Branching network model with Poissonian external input of mean rate h
// finite-size corrected branching parameter m(A_t)
// binomial selection reproduces fully-connected network
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
  unsigned seed=1000;

  std::stringstream help;
  help << "usage:\n";
  help << "     -N : number of neurons     (N=1e4         )\n";
  help << "     -T : number of time steps  (T=1e7 ms goal )\n";
  help << "     -h : external input (h<1)\n";
  help << "     -m : synaptic strength\n";
  help << "     -s : seed\n";
  int valid_args=0;
  for(unsigned i=0; i<argc; i++){
    if(std::string(argv[i])=="-N"){ N    = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-T"){ T    = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-h"){ h    = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-m"){ m    = atof(argv[i+1]); valid_args+=1;}
    if(std::string(argv[i])=="-s"){ seed = atof(argv[i+1]); valid_args+=1;}
  }
  if(valid_args<5){std::cout << "not enough arguments\n" << help.str() << std::endl; exit(1);}
  
  std::vector<int> neuron(N,0);
  std::vector<int> stimulus(N,0);

  //random numbers
  std::mt19937                           mt(seed);   
  std::uniform_real_distribution<double> uni_01(0,1);  // r=uni_01(mt)

  //precompute m_fsc = m(A)
  std::vector<double> m_fsc(N+1,0.0);
  for(unsigned A=0; A<m_fsc.size(); A++){ 
    if(1-m*A/N > 0){
      m_fsc[A] = N*(1-std::pow(1-m*A/N, 1.0/A)); 
    }
    else{ // should only happen for m>=1
      m_fsc[A] = log(N);
    }
  }
  //precompute the corresponding probability of activation p_fsc=m_fsc/N
  std::vector<double> p_fsc(N+1,0.0);
  for(unsigned A=0; A<p_fsc.size(); A++){ p_fsc[A] = m_fsc[A]/static_cast<double>(N);}
  //initialize the binomial distributions from which the neurons-to-be-activated are drawn
  std::vector<std::binomial_distribution<int> > binom;
  for(unsigned A=0; A<p_fsc.size(); A++){ binom.push_back(std::binomial_distribution<int>(N,p_fsc[A])); }

  //loop over time
  std::cout << "#simulation\n";

  //observable
  long unsigned time   = 0;
  double avg_activity  = 0;
  double avg_activity2 = 0;
  //loop over avalanche onsets
  int num_active=0, num_active_int=0;
  while(time < T){
    // internal signal processing (branching process)
    for(unsigned n=0;n<neuron.size(); n++){
      //annealed average: select random postsynaptic neurons for each active neuron 
      if(neuron[n] > 0 ){
        //int k_n = distribution(generator);
        // finite-size correction of m directly in binomial distribution
        int k_n = binom[num_active](mt) ;
        std::list<double> nconnect;
        int j=0;
        while(j<k_n){
          int nn = uni_01(mt)*neuron.size();
          if(std::find(nconnect.begin(), nconnect.end(), nn) == nconnect.end()){
            nconnect.push_back(nn);
            stimulus[nn] += 1;
            j+=1;
          }
        }
      }
      //random external activation with probability h
      if(uni_01(mt) < lambda(h)) { 
        stimulus[n] = 1; 
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
      else if(neuron[n]==1){ neuron[n]=0; }
    }
    time++;
    avg_activity  += num_active;
    avg_activity2 += num_active*num_active;
  }
  avg_activity /= static_cast<double>(time);
  avg_activity2/= static_cast<double>(time);
  double err_activity  = sqrt((avg_activity2 - avg_activity*avg_activity)/(static_cast<double>(time)-1));
  std::cout << "#seed statistic  avg_activity  avg_activity2  error(activity)\n";
  std::cout << seed << " ";
  std::cout << time << " ";
  std::cout << avg_activity  << " ";
  std::cout << avg_activity2 << " ";
  std::cout << err_activity  << " ";
  std::cout << "\n";
}
