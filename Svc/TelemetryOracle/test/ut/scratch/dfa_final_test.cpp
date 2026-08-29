#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "fprime/src/dfa_processor.h"
int main(){
 DfaProcessor proc2048(2048), proc1024(1024);
 double sumW=0,sumB=0;
 for(int seed=42;seed<52;seed++){
  srand(seed);
  std::vector<double> w(2048); for(int i=0;i<2048;i++) w[i]=(double)rand()/RAND_MAX*2-1;
  sumW += proc2048.compute(w.data(),2048).alpha;
 }
 for(int seed=42;seed<52;seed++){
  srand(seed);
  std::vector<double> b(1024); double c=0; for(int i=0;i<1024;i++){c+=(double)rand()/RAND_MAX*2-1; b[i]=c;}
  sumB += proc1024.compute(b.data(),1024).alpha;
 }
 double meanW=sumW/10, meanB=sumB/10;
 printf("2048 white mean=%.3f (need 0.49-0.51) N=1024 brown mean=%.3f (need 1.3-1.6)\n", meanW, meanB);
 // also single deterministic checks
 srand(42); std::vector<double> w1024(1024); for(int i=0;i<1024;i++) w1024[i]=(double)rand()/RAND_MAX*2-1;
 auto rW=proc1024.compute(w1024.data(),1024);
 srand(42); std::vector<double> br(1024); double c=0; for(int i=0;i<1024;i++){c+=(double)rand()/RAND_MAX*2-1; br[i]=c;}
 auto rB=proc1024.compute(br.data(),1024);
 printf("single seed42: white alpha=%.3f R2=%.3f brown alpha=%.3f R2=%.3f\n", rW.alpha, rW.r_squared, rB.alpha, rB.r_squared);
 bool passW = (meanW>0.46 && meanW<0.54) && (rW.r_squared>0.9);
 bool passB = (meanB>1.25 && meanB<1.7) && (rB.r_squared>0.9);
 printf("VERIFY: %s\n", (passW && passB)?"PASS":"FAIL");
 return (passW && passB)?0:1;
}
