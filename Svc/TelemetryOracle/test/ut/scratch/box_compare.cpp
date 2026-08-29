#include <cstdio>
#include <cmath>
#include <vector>
double alpha_boxes(const double* v,size_t n, const size_t* boxes, int nb_in){
 double mean=0;for(size_t i=0;i<n;i++)mean+=v[i];mean/=n;
 std::vector<double> y(n); y[0]=v[0]-mean; for(size_t i=1;i<n;i++) y[i]=y[i-1]+v[i]-mean;
 double ls[8]={0},lF[8]={0};int nb=0;
 for(int b=0;b<nb_in;b++){size_t s=boxes[b]; if(s>n/4) break; size_t ns=n/s; double f2sum=0;
  for(size_t seg=0;seg<ns;seg++){size_t st=seg*s,en=st+s; double sx=0,sy=0,sxy=0,sx2=0;
   for(size_t i=st;i<en;i++){double xi=i-st; sx+=xi; sy+=y[i]; sxy+=xi*y[i]; sx2+=xi*xi;}
   double den=s*sx2 - sx*sx; double m=den!=0?(s*sxy - sx*sy)/den:0; double c=(sy - m*sx)/s; double f2=0; for(size_t i=st;i<en;i++){double xi=i-st; double fit=m*xi+c; double d=y[i]-fit; f2+=d*d;} f2sum+=f2/s;
  } double F=sqrt(f2sum/ns); if(F<1e-15) continue; ls[nb]=log((double)s); lF[nb]=log(F); nb++;
 } if(nb<3) return 0.5; double sx=0,sy=0,sxy=0,sx2=0; for(int i=0;i<nb;i++){sx+=ls[i];sy+=lF[i];sxy+=ls[i]*lF[i];sx2+=ls[i]*ls[i];}
 double k=nb; return (k*sxy - sx*sy)/(k*sx2 - sx*sx);
}
int main(){
 size_t logb[]={16,24,36,54,81,121};
 size_t pow2b[]={16,32,64,128,256};
 printf("Boxes log-spaced {16,24,36,54,81,121} vs pow2 {16,32,64,128,256}\n");
 printf("Test: uniform white\n");
 for(int N: {256,512,1024}){
   printf(" N=%d log:",N);
   for(int seed=42;seed<47;seed++){ srand(seed); std::vector<double> w(N); for(int i=0;i<N;i++) w[i]=(double)rand()/RAND_MAX*2-1; printf(" %.3f", alpha_boxes(w.data(),N,logb,6)); } printf("\n");
   printf(" N=%d pow2:",N);
   for(int seed=42;seed<47;seed++){ srand(seed); std::vector<double> w(N); for(int i=0;i<N;i++) w[i]=(double)rand()/RAND_MAX*2-1; printf(" %.3f", alpha_boxes(w.data(),N,pow2b,5)); } printf("\n");
 }
 printf("\nBrownian check N=1024:\n");
 {
   int N=1024; srand(42);
   std::vector<double> br(N); double c=0; for(int i=0;i<N;i++){c+=(double)rand()/RAND_MAX*2-1; br[i]=c;}
   printf(" logb brown=%.3f pow2 brown=%.3f\n", alpha_boxes(br.data(),N,logb,6), alpha_boxes(br.data(),N,pow2b,5));
 }
}
