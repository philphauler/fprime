#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>

struct DfaResult {
    double alpha;
    double r_squared;
    int num_boxes;
};

class DfaProcessor {
public:
    DfaResult compute(const double* values, size_t n, bool debug=false) {
        if (n < 64) return {0.5, 0.0, 0};
        double mean = 0;
        for (size_t i = 0; i < n; ++i) mean += values[i];
        mean /= (double)n;
        std::vector<double> y(n);
        y[0] = values[0] - mean;
        for (size_t i = 1; i < n; ++i) y[i] = y[i-1] + (values[i] - mean);
        static constexpr size_t boxes[] = {16, 24, 36, 54, 81, 121};
        double log_s[6]={0}, log_F[6]={0};
        int num_valid_boxes = 0;
        for (int b = 0; b < 6; ++b) {
            size_t s = boxes[b];
            if (s > n/4) break;
            size_t num_segs = n / s;
            if (num_segs == 0) continue;
            double f2_sum = 0;
            for (size_t seg = 0; seg < num_segs; ++seg) {
                size_t start = seg * s;
                size_t end = start + s;
                double sx=0,sy=0,sxy=0,sx2=0;
                for (size_t i = start; i < end; ++i) {
                    double xi = (double)(i - start);
                    sx += xi; sy += y[i]; sxy += xi*y[i]; sx2 += xi*xi;
                }
                double denom = (s * sx2 - sx*sx);
                double m = denom!=0 ? (s * sxy - sx*sy)/denom : 0;
                double c = (sy - m*sx)/(double)s;
                double f2=0;
                for (size_t i = start; i < end; ++i) {
                    double xi=(double)(i-start);
                    double fit=m*xi+c;
                    double d=y[i]-fit;
                    f2+=d*d;
                }
                f2_sum += f2/(double)s;
            }
            double F = sqrt(f2_sum/(double)num_segs);
            if (F > 1e-15) {
                log_s[num_valid_boxes]=log((double)boxes[b]);
                log_F[num_valid_boxes]=log(F);
                if(debug) printf("  box s=%zu log_s=%.4f F=%.6f log_F=%.4f segs=%zu\n", s, log_s[num_valid_boxes], F, log_F[num_valid_boxes], num_segs);
                num_valid_boxes++;
            }
        }
        if (num_valid_boxes < 3) return {0.5,0.0,0};
        double sx=0,sy=0,sxy=0,sx2=0;
        for(int i=0;i<num_valid_boxes;++i){sx+=log_s[i];sy+=log_F[i];sxy+=log_s[i]*log_F[i];sx2+=log_s[i]*log_s[i];}
        double k=(double)num_valid_boxes;
        double slope=(k*sxy - sx*sy)/(k*sx2 - sx*sx);
        double ic=(sy - slope*sx)/k;
        double ym=sy/k;
        double sst=0,ssr=0;
        for(int i=0;i<num_valid_boxes;++i){ double di=log_F[i]-ym; sst+=di*di; double dr=log_F[i]-(slope*log_s[i]+ic); ssr+=dr*dr; }
        DfaResult r; r.alpha=slope; r.r_squared=(sst>1e-15)?(1.0-ssr/sst):0.0; r.num_boxes=num_valid_boxes;
        if(debug) printf("  regression slope=%.4f ic=%.4f R2=%.4f\n", slope, ic, r.r_squared);
        return r;
    }
};

int main(){
 srand(42);
 const int N=256;
 double white[N];
 for(int i=0;i<N;i++) white[i]= (double)rand()/RAND_MAX*2.0-1.0;
 double inc[N];
 {
   // separate rng for second test - reseed differently
   srand(43);
   double cum=0;
   for(int i=0;i<N;i++){ double w=(double)rand()/RAND_MAX*2.0-1.0; cum+=w; inc[i]=cum; }
 }

 DfaProcessor p;
 printf("White noise debug:\n");
 DfaResult r1=p.compute(white,N,true);
 printf("-> alpha=%.4f R2=%.4f\n\n", r1.alpha, r1.r_squared);
 printf("Integrated RW (Brownian) debug:\n");
 DfaResult r2=p.compute(inc,N,true);
 printf("-> alpha=%.4f R2=%.4f\n\n", r2.alpha, r2.r_squared);

 // Test 3: use larger window 1024 with more boxes
 const int N3=1024;
 double white2[N3];
 srand(44);
 for(int i=0;i<N3;i++) white2[i]= (double)rand()/RAND_MAX*2.0-1.0;
 printf("White noise N=1024 debug (more scales):\n");
 // temporarily compute with larger boxes to see scaling
 {
   // manual with larger boxes set
   // just reuse same processor but N/4=256 so all 6 boxes valid
   DfaResult r3=p.compute(white2,N3,true);
   printf("-> alpha=%.4f R2=%.4f boxes=%d\n\n", r3.alpha, r3.r_squared, r3.num_boxes);
 }
}
