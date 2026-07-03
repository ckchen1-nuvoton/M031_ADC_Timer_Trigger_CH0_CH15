# M031 ADC Hardware Trigger & Timing Benchmark

This project demonstrates how to implement a high-efficiency ADC sampling system using **Timer0 periodic hardware triggering** on the Nuvoton M031 microcontroller. It includes a benchmarking mechanism using GPIO toggling (`PA0`/`PA1`) to precisely measure peripheral latency via an oscilloscope.

## 🚀 Key Features
* **Zero-CPU Triggering:** Timer0 timeout events directly trigger the ADC module via hardware linkage.
* **16-Channel Scan:** Sequentially samples ADC Channels 0 to 15 in Single Mode.
* **Dynamic Mode Switching:** Automatically switches to high-speed **Burst Mode on CH2** (40 continuous samples via software polling) every 20,000 single cycles.

---

## 📊 Hardware & Oscilloscope Configuration

Connect your oscilloscope probes to the following pins to benchmark the system execution time:

| Pin | Function | Signal Description |
| :--- | :--- | :--- |
| **PA0** | **ADC Conversion Time** | **HIGH:** Timer0 timeout (ADC Starts)<br>**LOW:** ADC ISR entered (Conversion Ends)<br>*The pulse width represents the true ADC hardware latency.* |
| **PA1** | **Burst Mode Duration** | **HIGH:** Entering `ADC_Burst_CH2()` routine<br>**LOW:** Leaving Burst Mode (40 samples completed) |
| **PB.0-15**| **Analog Inputs** | ADC0 Channel 0 to 15 input paths (Digital paths disabled). |
| **PB.13/12**| **Debug UART0** | TX / RX for serial logging (`115200, 8-N-1`). |

<img width="80%" alt="image" src="https://github.com/user-attachments/assets/3f77f3a6-1130-46f0-b1d7-f26556b58fdc" />

---

## 🛠️ Timing & Execution Logic

The calculation of the ADC conversion time is isolated from CPU thread jitter by splitting the GPIO toggling inside low-overhead Interrupt Service Routines (ISRs):

1. **`TMR0_IRQHandler`:** Triggered at **20 KHz** (every 50µs). It auto-triggers the ADC hardware and pulls **`PA0` HIGH**.
2. **`ADC_IRQHandler`:** Triggered as soon as the SAR ADC finishes its conversion. It pulls **`PA0` LOW**.



## 💻 Environment
* **MCU:** Nuvoton NuMicro M031 Series
* **IDE:** Keil MDK uVision
* **Clock:** 48MHz Internal High-Speed RC Oscillator (HIRC) as HCLK
