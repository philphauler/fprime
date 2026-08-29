#include <cstdio>
#include <cmath>
#include <vector>
double alpha_lin(const double* v,size_t n){
 double mean=0;for(size_t i=0;i<n;i++)mean+=v[i];mean/=n;
 std::vector<double> y(n); y[0]=v[0]-mean; for(size_t i=1;i<n;i++) y[i]=y[i-1]+v[i]-mean;
 static constexpr size_t boxes[]={16,32,64,128,256,512};
 double ls[6]={0},lF[6]={0};int nb=0;
 for(int b=0;b<6;b++){size_t s=boxes[b]; if(s>n/4) break; size_t ns=n/s; double f2sum=0;
  for(size_t seg=0;seg<ns;seg++){size_t st=seg*s,en=st+s; double sx=0,sy=0,sxy=0,sx2=0;
   for(size_t i=st;i<en;i++){double xi=i-st; sx+=xi; sy+=y[i]; sxy+=xi*y[i]; sx2+=xi*xi;}
   double den=s*sx2 - sx*sx; double m=den!=0?(s*sxy - sx*sy)/den:0; double c=(sy - m*sx)/s; double f2=0; for(size_t i=st;i<en;i++){double xi=i-st; double fit=m*xi+c; double d=y[i]-fit; f2+=d*d;} f2sum+=f2/s;
  } double F=sqrt(f2sum/ns); if(F<1e-15) continue; ls[nb]=log((double)s); lF[nb]=log(F); nb++;
 } if(nb<3) return 0.5; double sx=0,sy=0,sxy=0,sx2=0; for(int i=0;i<nb;i++){sx+=ls[i];sy+=lF[i];sxy+=ls[i]*lF[i];sx2+=ls[i]*ls[i];}
 double k=nb; return (k*sxy - sx*sy)/(k*sx2 - sx*sx);
}
int main(){
 for(int N: {1024,2048,4096}){
   printf("N=%d uniform white 10 seeds: ",N);
   double sum=0,sum2=0;
   for(int seed=42;seed<52;seed++){ srand(seed); std::vector<double> w(N); for(int i=0;i<N;i++) w[i]=(double)rand()/RAND_MAX*2-1; double a=alpha_lin(w.data(),N); printf("%.3f ",a); sum+=a; sum2+=a*a; }
   double mean=sum/10; double var=sum2/10 - mean*mean;
   printf(" | mean=%.3f std=%.3f\n", mean, sqrt(var));
 }
 printf("\nN=1024 Brownian mean:\n");
 {
   double sum=0; for(int seed=42;seed<52;seed++){ srand(seed); std::vector<double> br(1024); double c=0; for(int i=0;i<1024;i++){c+=(double)rand()/RAND_MAX*2-1; br[i]=c;} sum+=alpha_lin(br.data(),1024); } printf(" brown mean=%.3f expected ~1.5\n", sum/10);
 }
}
