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
    DfaProcessor(size_t window_size = 256) : window_size_(window_size) {}

    DfaResult compute(const double* values, size_t n) {
        if (n < 64) return {0.5, 0.0, 0};

        double mean = 0;
        for (size_t i = 0; i < n; ++i) mean += values[i];
        mean /= static_cast<double>(n);

        // Cumulative profile (integration)
        std::vector<double> y(n);
        y[0] = values[0] - mean;
        for (size_t i = 1; i < n; ++i) y[i] = y[i-1] + (values[i] - mean);

        // Logarithmically spaced box sizes
        static constexpr size_t boxes[] = {16, 24, 36, 54, 81, 121};
        double log_s[6];
        double log_F[6];
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

                // Local linear fit to the box: y = m*x + c
                double sx = 0, sy = 0, sxy = 0, sx2 = 0;
                for (size_t i = start; i < end; ++i) {
                    double xi = static_cast<double>(i - start);
                    sx += xi;
                    sy += y[i];
                    sxy += xi * y[i];
                    sx2 += xi * xi;
                }
                double denom = (s * sx2 - sx * sx);
                double m = (denom != 0) ? (s * sxy - sx * sy) / denom : 0;
                double c = (sy - m * sx) / static_cast<double>(s);

                // Detrended fluctuation for this box
                double f2 = 0;
                for (size_t i = start; i < end; ++i) {
                    double xi = static_cast<double>(i - start);
                    double fit = m * xi + c;
                    double d = y[i] - fit;
                    f2 += d * d;
                }
                f2_sum += f2 / static_cast<double>(s);
            }

            double F = sqrt(f2_sum / static_cast<double>(num_segs));
            if (F > 1e-15) {
                log_s[num_valid_boxes] = log(static_cast<double>(boxes[b]));
                log_F[num_valid_boxes] = log(F);
                num_valid_boxes++;
            }
        }

        if (num_valid_boxes < 3) return {0.5, 0.0, 0};

        // Linear regression: log(F) = slope * log(s) + intercept
        double sx = 0, sy = 0, sxy = 0, sx2 = 0;
        for (int i = 0; i < num_valid_boxes; ++i) {
            sx += log_s[i];
            sy += log_F[i];
            sxy += log_s[i] * log_F[i];
            sx2 += log_s[i] * log_s[i];
        }

        double k = static_cast<double>(num_valid_boxes);
        double slope = (k * sxy - sx * sy) / (k * sx2 - sx * sx);
        double ic = (sy - slope * sx) / k;
        double ym = sy / k;

        double sst = 0, ssr = 0;
        for (int i = 0; i < num_valid_boxes; ++i) {
            double di = log_F[i] - ym;
            sst += di * di;
            double di_resid = log_F[i] - (slope * log_s[i] + ic);
            ssr += di_resid * di_resid;
        }

        DfaResult result;
        result.alpha = slope;
        result.r_squared = (sst > 1e-15) ? (1.0 - ssr / sst) : 0.0;
        result.num_boxes = num_valid_boxes;
        return result;
    }

    size_t window_size() const { return window_size_; }

private:
    size_t window_size_;
};

int main() {
    srand(42); // deterministic

    // Test 1: White noise (alpha ~ 0.5)
    const int N1 = 256;
    double white_noise[N1];
    for (int i = 0; i < N1; i++) {
        white_noise[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
    }

    // Test 2: Long-range correlated via p-model (alpha ~ 0.7-0.8)
    const int N2 = 256;
    double lrc[N2];
    double cum = 0;
    for (int i = 0; i < N2; i++) {
        cum += ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        lrc[i] = cum;  // integrated random walk -> correlated
    }

    DfaProcessor proc;
    DfaResult r1 = proc.compute(white_noise, N1);
    DfaResult r2 = proc.compute(lrc, N2);

    printf("=== DFA Algorithm Verification (fixed) ===\n");
    printf("Test 1 (White Noise):\n");
    printf("  alpha = %.4f (expected ~0.5)\n", r1.alpha);
    printf("  R^2   = %.4f (expected >0.9)\n", r1.r_squared);
    printf("  boxes = %d\n", r1.num_boxes);
    printf("\n");
    printf("Test 2 (Integrated RW):\n");
    printf("  alpha = %.4f (expected ~0.7-0.8)\n", r2.alpha);
    printf("  R^2   = %.4f (expected >0.9)\n", r2.r_squared);
    printf("  boxes = %d\n", r2.num_boxes);
    printf("\n");

    bool pass1 = (r1.alpha >= 0.4 && r1.alpha <= 0.6) && (r1.r_squared > 0.8);
    bool pass2 = (r2.alpha >= 0.6 && r2.alpha <= 0.9) && (r2.r_squared > 0.8);

    printf("Verification: %s\n", (pass1 && pass2) ? "PASS" : "FAIL");
    return (pass1 && pass2) ? 0 : 1;
}
