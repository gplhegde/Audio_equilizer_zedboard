# Audio_equilizer_zedboard
This is a small application which takes audio input from audio jack, computes its spectrum, applies some noise cancellation techniques and displays the spectrum on OLED display of ZedBoard. Spectrum computation(FFT) is done in Hardware and Software\\

# Directory Structure
sw_only: pure software based implementation of the project. The source files should be built with standard Xilinx Zynq platform BSP and the FPGA should be programmed with bitstream under sw_only/hw_platform/system.bit \\

hw_sw_codesign: In this version, FFT computation is done on FPGA. The HW has to be generated using given HDL source files using Xilinx tools.
