#include <stdio.h>
#include <math.h>
#include "xil_io.h"       
#include "xparameters.h"  

// --- ΡΥΘΜΙΣΕΙΣ ΓΙΑ ΤΟΝ GLOBAL TIMER ---

#define GLOBAL_TIMER_BASE 0xF8F00200
#define GTIMER_COUNTER_LOWER (GLOBAL_TIMER_BASE + 0x00)
#define GTIMER_COUNTER_UPPER (GLOBAL_TIMER_BASE + 0x04)
#define GTIMER_CONTROL       (GLOBAL_TIMER_BASE + 0x08)

// Προσπάθεια εύρεσης συχνότητας CPU για σωστή μέτρηση χρόνου
#if defined(XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ)
  #define CPU_FREQ XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ
#elif defined(XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ)
  #define CPU_FREQ XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ
#else
  #define CPU_FREQ 650000000 
#endif

// Ο Global Timer τρέχει πάντα στο 1/2 της συχνότητας της CPU
#define TIMER_FREQ (CPU_FREQ / 2)

// Σταθερές Εργασίας
#define N 8
#define NUM_SAMPLES 32
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float h[N] = {
    -0.0010f, 0.1219f, 0.1862f, 0.2429f,
     0.2429f, 0.1862f, 0.1219f, -0.0010f
};

float x[NUM_SAMPLES];
float y[NUM_SAMPLES];

// Συνάρτηση Έναρξης Timer
void StartTimer() {
    
    Xil_Out32(GTIMER_CONTROL, 0x01);
}

// Συνάρτηση Ανάγνωσης Χρόνου (64-bit)
unsigned long long GetTime() {
    // Διαβάζουμε τους καταχωρητές του μετρητή
    u32 low = Xil_In32(GTIMER_COUNTER_LOWER);
    u32 high = Xil_In32(GTIMER_COUNTER_UPPER);
    // Συνδυασμός σε έναν 64-bit αριθμό
    return ((unsigned long long)high << 32) | low;
}

void run_experiment(double omega, const char* description) {
    int n, k;
    unsigned long long start, end;
    double time_in_seconds;

    // --- ΒΗΜΑ 1: Προετοιμασία ---
    for (n = 0; n < NUM_SAMPLES; n++) {
        x[n] = (float)cos(omega * n);
    }
    for (n = 0; n < NUM_SAMPLES; n++) {
        y[n] = 0.0f;
    }

    // --- ΒΗΜΑ 2: Μέτρηση Χρόνου ---
    start = GetTime(); // Λήψη αρχικού χρόνου

    // Ο Υπολογισμός Συνέλιξης (Serial)
    for (n = 7; n < NUM_SAMPLES; n++) {
        float sum = 0.0f;
        for (k = 0; k < N; k++) {
            sum += h[k] * x[n - k];
        }
        y[n] = sum;
    }

    end = GetTime(); // Λήψη τελικού χρόνου
    
    // Μετατροπή κύκλων σε δευτερόλεπτα
    time_in_seconds = (double)(end - start) / (double)TIMER_FREQ;

    // --- ΒΗΜΑ 3: Εμφάνιση ---
    printf("\n--------------------------------------------------\n");
    printf("Πείραμα: %s\n", description);
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
    StartTimer(); // Εκκίνηση του Hardware Timer

    printf("--- Ξεκινάει το FIR Filter Benchmark (Direct Hardware Access) ---\n");

    double omega1 = 2.0 * M_PI / 16.0;
    run_experiment(omega1, "Low Freq (2pi/16)");

    double omega2 = 2.0 * M_PI / 8.0;
    run_experiment(omega2, "High Freq (2pi/8)");

    return 0;
}