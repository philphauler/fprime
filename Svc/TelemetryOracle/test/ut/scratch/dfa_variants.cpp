#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>

double compute_alpha(const double* v, size_t n, int variant, bool dbg=false){
 if(n<64) return 0.5;
 double mean=0; for(size_t i=0;i<n;i++) mean+=v[i]; mean/=n;
 std::vector<double> y(n);
 y[0]=v[0]-mean; for(size_t i=1;i<n;i++) y[i]=y[i-1]+v[i]-mean;
 static constexpr size_t boxes[]={16,24,36,54,81,121};
 double ls[6]={0}, lF[6]={0};
 int nb=0;
 for(int b=0;b<6;b++){
  size_t s=boxes[b]; if(s>n/4) break; size_t ns=n/s; if(ns==0) continue;
  double f2sum=0;
  for(size_t seg=0;seg<ns;seg++){
   size_t st=seg*s, en=st+s;
   if(variant==0){ // DFA1 local linear detrend (current)
     double sx=0,sy=0,sxy=0,sx2=0;
     for(size_t i=st;i<en;i++){ double xi=(double)(i-st); sx+=xi; sy+=y[i]; sxy+=xi*y[i]; sx2+=xi*xi; }
     double den=s*sx2 - sx*sx; double m=den!=0?(s*sxy - sx*sy)/den:0; double c=(sy - m*sx)/(double)s;
     double f2=0; for(size_t i=st;i<en;i++){ double xi=(double)(i-st); double fit=m*xi+c; double d=y[i]-fit; f2+=d*d; } f2sum+=f2/(double)s;
   } else if(variant==1){ // no detrend: just variance of y
     double f2=0; for(size_t i=st;i<en;i++) f2+=y[i]*y[i]; f2sum+=f2/(double)s;
   } else if(variant==2){ // DFA1 but global index for fit
     double sx=0,sy=0,sxy=0,sx2=0;
     for(size_t i=st;i<en;i++){ double xi=(double)i; sx+=xi; sy+=y[i]; sxy+=xi*y[i]; sx2+=xi*xi; }
     double den=s*sx2 - sx*sx; double m=den!=0?(s*sxy - sx*sy)/den:0; double c=(sy - m*sx)/(double)s;
     double f2=0; for(size_t i=st;i<en;i++){ double xi=(double)i; double fit=m*xi+c; double d=y[i]-fit; f2+=d*d; } f2sum+=f2/(double)s;
   } else if(variant==3){ // DFA: use mean of y in box as detrend (remove constant)
     double sy=0; for(size_t i=st;i<en;i++) sy+=y[i]; double my=sy/(double)s;
     double f2=0; for(size_t i=st;i<en;i++){ double d=y[i]-my; f2+=d*d; } f2sum+=f2/(double)s;
   }
  }
  double F=sqrt(f2sum/(double)ns); if(F>1e-15){ ls[nb]=log((double)s); lF[nb]=log(F); nb++; if(dbg) printf(" s=%zu F=%.4f\n", s, F); }
 }
 if(nb<3) return 0.5;
 double sx=0,sy=0,sxy=0,sx2=0; for(int i=0;i<nb;i++){ sx+=ls[i]; sy+=lF[i]; sxy+=ls[i]*lF[i]; sx2+=ls[i]*ls[i]; }
 double k=nb; double slope=(k*sxy - sx*sy)/(k*sx2 - sx*sx);
 return slope;
}

int main(){
 srand(42);
 const int N=1024;
 double white[N]; for(int i=0;i<N;i++) white[i]=(double)rand()/RAND_MAX*2.0-1.0;
 srand(43); double brown[N]; double cum=0; for(int i=0;i<N;i++){ cum+=(double)rand()/RAND_MAX*2.0-1.0; brown[i]=cum; }
 srand(44); double pink[N]; // 1/f via filtering approx: cumulative of white with damping
 for(int i=0;i<N;i++) pink[i]=0;
 // Generate pink via Voss-McCartney not needed; use integrated white then differentiate? skip

 printf("N=1024  variant 0=linear-detrend 1=no-detrend 2=global-fit 3=mean-detrend\n");
 for(int v=0;v<4;v++){
   double aW=compute_alpha(white,N,v,false);
   double aB=compute_alpha(brown,N,v,false);
   printf("v%d: white alpha=%.3f  brown alpha=%.3f\n", v, aW, aB);
 }
 // Also test with Gaussian white (Box-Muller)
 srand(42);
 double gwhite[N]; for(int i=0;i<N;i++){ double u1=(double)rand()/RAND_MAX+1e-9, u2=(double)rand()/RAND_MAX; double z=sqrt(-2*log(u1))*cos(6.28318530718*u2); gwhite[i]=z; }
 printf("\nGaussian white N=1024:\n");
 for(int v=0;v<4;v++) printf(" v%d alpha=%.3f\n", v, compute_alpha(gwhite,N,v,false));
 // Test with known DFA reference: if we generate pure sine wave, alpha should be ~? not needed

 // Also test scaling with larger s set {8,16,32,64,128,256}
 printf("\nCustom box set {16,32,64,128,256} linear detrend:\n");
 {
   // manual quick
   auto comp2=[&](const double* v,size_t n){
     double mean=0;for(size_t i=0;i<n;i++)mean+=v[i];mean/=n;
     std::vector<double> y(n); y[0]=v[0]-mean; for(size_t i=1;i<n;i++) y[i]=y[i-1]+v[i]-mean;
     size_t boxes2[]={16,32,64,128,256};
     double ls[5]={0},lF[5]={0};int nb=0;
     for(int b=0;b<5;b++){size_t s=boxes2[b]; if(s>n/4) break; size_t ns=n/s; double f2sum=0;
       for(size_t seg=0;seg<ns;seg++){size_t st=seg*s,en=st+s; double sx=0,sy=0,sxy=0,sx2=0;
         for(size_t i=st;i<en;i++){double xi=i-st;sx+=xi;sy+=y[i];sxy+=xi*y[i];sx2+=xi*xi;}
         double den=s*sx2 - sx*sx; double m=den!=0?(s*sxy - sx*sy)/den:0; double c=(sy - m*sx)/s; double f2=0; for(size_t i=st;i<en;i++){double xi=i-st; double fit=m*xi+c; double d=y[i]-fit; f2+=d*d;} f2sum+=f2/s;
       }
       double F=sqrt(f2sum/ns); ls[nb]=log((double)s); lF[nb]=log(F); nb++;
     }
     double sx=0,sy=0,sxy=0,sx2=0;for(int i=0;i<nb;i++){sx+=ls[i];sy+=lF[i];sxy+=ls[i]*lF[i];sx2+=ls[i]*ls[i];}
     double k=nb; return (k*sxy - sx*sy)/(k*sx2 - sx*sx);
   };
   printf(" white pow2 boxes alpha=%.3f\n", comp2(white,N));
   printf(" brown pow2 boxes alpha=%.3f\n", comp2(brown,N));
 }
}
