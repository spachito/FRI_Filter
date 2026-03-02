🎛️ FIR Filter Benchmark & NEON Optimization:

A low-level embedded C project demonstrating the implementation, profiling, and optimization of a Finite Impulse Response (FIR) filter on an ARM Cortex-A9 processor. The project evaluates execution speed and efficiency by comparing standard serial execution against parallel processing using ARM NEON SIMD intrinsics.

✨ Technologies:

1.C

2.ARM NEON SIMD

3.Xilinx Vitis IDE

4.Zybo Development Board

5.Zynq Global Timer

🚀 Features:

1.Custom Profiling: Uses the memory-mapped 64-bit Global Timer of the Zynq SoC for highly accurate clock cycle counting

2.SIMD Vectorization: Leverages ARM NEON intrinsics (vld1q_f32, vmulq_f32, vaddq_f32) for parallel floating-point multiplication and vertical addition

3.Signal Verification: Validates filter functionality by confirming passband integrity for low frequencies (omega=2pi/16) and signal attenuation for high frequencies (omega=2pi/8)

4.Scalability Testing: Evaluates performance across different filter orders, comparing N=8 with a heavier N=128 configuration

5.Compiler Optimization Analysis: Demonstrates the impact of GCC Level 3 (-O3) optimizations on loop unrolling and pipeline efficiency.

🎈 The Process:

I've been on a mission to explore hardware-level acceleration for Digital Signal Processing (DSP). Most textbook implementations stop at serial execution, so I wanted to create a benchmark that truly leverages the underlying silicon. Started simple with a standard serial FIR filter implementation in C (main1.c). To make the processing feel more "alive" and modern, I incorporated ARM NEON SIMD instructions (main2.c) to calculate multiple samples simultaneously.

Sure, the parallel approach wasn't perfect at first—for a small filter ($N=8$), the overhead of loading vectors and extracting horizontal sums actually made it slower than serial execution ($Speedup < 1$). However, after cranking up the compiler optimizations to -O3 and increasing the filter complexity to $N=128$ (main3.c), the true power of parallelization unlocked, achieving a highly satisfying 1.94x speedup over the serial version!

🚦 Running the Project:

1.Clone the repository to your local machine.

2.Open Xilinx Vitis IDE and create a new application project targeting your Zybo/Zynq hardware.

3.Import the source files (main1.c, main2.c, main3.c) into your src directory

4.Build the project (Ensure you set the compiler optimization to -O3 for optimal results)

5.Connect your board and select Run As -> Launch on Hardware.

6.Open your serial terminal (Baud rate: 115200) to view the benchmark results.
