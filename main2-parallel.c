#include <stdio.h>
#include <math.h>
#include <arm_neon.h>     
#include "xil_io.h"
#include "xparameters.h"

// --- ΡΥΘΜΙΣΕΙΣ TIMER ---
#define GLOBAL_TIMER_BASE 0xF8F00200
#define GTIMER_COUNTER_LOWER (GLOBAL_TIMER_BASE + 0x00)
#define GTIMER_COUNTER_UPPER (GLOBAL_TIMER_BASE + 0x04)
#define GTIMER_CONTROL       (GLOBAL_TIMER_BASE + 0x08)

#if defined(XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ)
  #define CPU_FREQ XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ
#else
  #define CPU_FREQ 650000000
#endif
#define TIMER_FREQ (CPU_FREQ / 2)

#define N 8
#define NUM_SAMPLES 32
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// Πρέπει να βάλουμε τα h ανάποδα ώστε το h[7] να πέσει πάνω στο x[n-7]
// Σειρά στον πίνακα: h[7], h[6], h[5], h[4], h[3], h[2], h[1], h[0]
float h_rev[N] = {
    -0.0010f, // h[7]
     0.1219f, // h[6]
     0.1862f, // h[5]
     0.2429f, // h[4]
     0.2429f, // h[3]
     0.1862f, // h[2]
     0.1219f, // h[1]
    -0.0010f  // h[0]
};

float x[NUM_SAMPLES];
float y[NUM_SAMPLES];

// --- Συναρτήσεις Timer ---
void StartTimer() {
    Xil_Out32(GTIMER_CONTROL, 0x01);
}

unsigned long long GetTime() {
    u32 low = Xil_In32(GTIMER_COUNTER_LOWER);
    u32 high = Xil_In32(GTIMER_COUNTER_UPPER);
    return ((unsigned long long)high << 32) | low;
}

// --- Κύρια Συνάρτηση Παράλληλης εκτέλεσης ---
void run_experiment_parallel(double omega, const char* description) {
    int n;
    unsigned long long start, end;
    double time_in_seconds;

    // 1. Προετοιμασία
    for (n = 0; n < NUM_SAMPLES; n++) {
        x[n] = (float)cos(omega * n);
        y[n] = 0.0f;
    }

    // 2. Μέτρηση Χρόνου (NEON)
    
    // Φόρτωση συντελεστών σε καταχωρητές NEON (εκτός του loop για ταχύτητα)
    // q0 = {h[7], h[6], h[5], h[4]}
    float32x4_t h_vec_low  = vld1q_f32(&h_rev[0]); 
    // q1 = {h[3], h[2], h[1], h[0]}
    float32x4_t h_vec_high = vld1q_f32(&h_rev[4]); 

    start = GetTime();


    for (n = 7; n < NUM_SAMPLES; n++) {
        // Διαβάζουμε συνεχόμενα 4 float από τη μνήμη
        // Φορτώνουμε τα x[n-7], x[n-6], x[n-5], x[n-4]
        float32x4_t x_vec_low  = vld1q_f32(&x[n - 7]); 
        
        // Φορτώνουμε τα x[n-3], x[n-2], x[n-1], x[n]
        float32x4_t x_vec_high = vld1q_f32(&x[n - 3]); 

        // Παράλληλος Πολλαπλασιασμός
        float32x4_t prod_low  = vmulq_f32(h_vec_low, x_vec_low);
        float32x4_t prod_high = vmulq_f32(h_vec_high, x_vec_high);

        // Πρόσθεση των δύο διανυσμάτων αποτελεσμάτων
        float32x4_t sum_vec = vaddq_f32(prod_low, prod_high);

        // Οριζόντια άθροιση
        // Προσθέτουμε τα 4 στοιχεία του sum_vec μεταξύ τους
        float val = vgetq_lane_f32(sum_vec, 0) + vgetq_lane_f32(sum_vec, 1) +
                    vgetq_lane_f32(sum_vec, 2) + vgetq_lane_f32(sum_vec, 3);

        y[n] = val;
    }

    end = GetTime();
    time_in_seconds = (double)(end - start) / (double)TIMER_FREQ;

    // 3. Εμφάνιση (Ίδιο Format με το Σειραϊκό)
    printf("\n--------------------------------------------------\n");
    printf("Πείραμα (NEON): %s\n", description);
    printf("Συχνότητα (omega): %.4f rad/sample\n", omega);
    printf("Cycles: %llu\n", (end - start));
    printf("Χρόνος Εκτέλεσης: %.10f seconds\n", time_in_seconds);
    printf("--------------------------------------------------\n");
    printf("  n \t   x(n) (Είσοδος) \t   y(n) (Έξοδος)\n");
    
    for (n = 7; n < NUM_SAMPLES; n++) {
        printf(" %2d \t %10.4f \t %10.4f\n", n, x[n], y[n]);
    }
}

int main() {
    StartTimer();
    printf("--- FIR Filter Benchmark (NEON PARALLEL) ---\n");

    double omega1 = 2.0 * M_PI / 16.0;
    run_experiment_parallel(omega1, "Low Freq (2pi/16)");

    double omega2 = 2.0 * M_PI / 8.0;
    run_experiment_parallel(omega2, "High Freq (2pi/8)");

    return 0;
}