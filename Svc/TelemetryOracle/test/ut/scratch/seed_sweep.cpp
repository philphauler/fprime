#include <cstdio>
#include <cmath>
#include <vector>
double alpha_lin(const double* v,size_t n){
 double mean=0;for(size_t i=0;i<n;i++)mean+=v[i];mean/=n;
 std::vector<double> y(n); y[0]=v[0]-mean; for(size_t i=1;i<n;i++) y[i]=y[i-1]+v[i]-mean;
 static constexpr size_t boxes[]={16,24,36,54,81,121};
 double ls[6]={0},lF[6]={0};int nb=0;
 for(int b=0;b<6;b++){size_t s=boxes[b]; if(s>n/4) break; size_t ns=n/s; double f2sum=0;
  for(size_t seg=0;seg<ns;seg++){size_t st=seg*s,en=st+s; double sx=0,sy=0,sxy=0,sx2=0;
   for(size_t i=st;i<en;i++){double xi=i-st; sx+=xi; sy+=y[i]; sxy+=xi*y[i]; sx2+=xi*xi;}
   double den=s*sx2 - sx*sx; double m=den!=0?(s*sxy - sx*sy)/den:0; double c=(sy - m*sx)/s; double f2=0; for(size_t i=st;i<en;i++){double xi=i-st; double fit=m*xi+c; double d=y[i]-fit; f2+=d*d;} f2sum+=f2/s;
  } double F=sqrt(f2sum/ns); ls[nb]=log((double)s); lF[nb]=log(F); nb++;
 } double sx=0,sy=0,sxy=0,sx2=0; for(int i=0;i<nb;i++){sx+=ls[i];sy+=lF[i];sxy+=ls[i]*lF[i];sx2+=ls[i]*ls[i];}
 double k=nb; return (k*sxy - sx*sy)/(k*sx2 - sx*sx);
}
int main(){
 printf("Seed sweep uniform white, boxes 16,24,36,54,81,121\n");
 printf("N=256: "); for(int seed=42;seed<52;seed++){ srand(seed); double w[256]; for(int i=0;i<256;i++) w[i]=(double)rand()/RAND_MAX*2-1; printf("%.3f ", alpha_lin(w,256)); } printf("\n");
 printf("N=512: "); for(int seed=42;seed<52;seed++){ srand(seed); double w[512]; for(int i=0;i<512;i++) w[i]=(double)rand()/RAND_MAX*2-1; printf("%.3f ", alpha_lin(w,512)); } printf("\n");
 printf("N=1024:"); for(int seed=42;seed<52;seed++){ srand(seed); double w[1024]; for(int i=0;i<1024;i++) w[i]=(double)rand()/RAND_MAX*2-1; printf("%.3f ", alpha_lin(w,1024)); } printf("\n");
 printf("\nGaussian white:\n");
 printf("N=256: "); for(int seed=42;seed<52;seed++){ srand(seed); double w[256]; for(int i=0;i<256;i++){double u1=(double)rand()/RAND_MAX+1e-9,u2=(double)rand()/RAND_MAX; w[i]=sqrt(-2*log(u1))*cos(6.28318530718*u2);} printf("%.3f ", alpha_lin(w,256)); } printf("\n");
 printf("N=1024:"); for(int seed=42;seed<52;seed++){ srand(seed); double w[1024]; for(int i=0;i<1024;i++){double u1=(double)rand()/RAND_MAX+1e-9,u2=(double)rand()/RAND_MAX; w[i]=sqrt(-2*log(u1))*cos(6.28318530718*u2);} printf("%.3f ", alpha_lin(w,1024)); } printf("\n");
}
