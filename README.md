# Real-Time FIR Filtering on TMS320F28335

This project performs real-time digital filtering using the TI TMS320F28335 DSP. A 2kHz PWM signal is generated on ePWM1A, sampled via ADCINA2 at 50kHz, and passed through a 5-tap low-pass FIR filter implemented using IQmathLib.

---

## 🎯 What This Project Does

- Generates a 2kHz PWM signal on `ePWM1A` (25% duty cycle)
- Samples that PWM signal at 50kHz using `ADCINA2`
- Applies a **5-tap FIR low-pass filter** in real time:
  - Coefficients: `{0.0357, 0.2411, 0.4465, 0.2411, 0.0357}`
  - Implemented via `IQssfir()` using fixed-point IQ math
- Stores:
  - Raw ADC values in `AdcBuf[]`
  - Filtered values in `AdcBufFiltered[]`
- Visualizes system status via 4 GPIO-controlled LEDs (binary counter)

---

## ⚙️ Hardware Requirements

- **TI TMS320F28335** DSP or compatible ControlCARD
- Onboard ADC (channel A2 / ADCINA2)
- PWM output on GPIO0 (`ePWM1A`)
- LEDs connected to:
  - GPIO9 (LD1)
  - GPIO11 (LD2)
  - GPIO34 (LD3)
  - GPIO49 (LD4)

---

## 📦 Project Structure

dsp28335-realtime-fir-filter/
├── main.c # Main logic (PWM, ADC, FIR, LED control)
├── filter.c/.h # IQssfir implementation (if not linked from library)
├── IQmathLib headers # TI's fixed-point math library (linked or included)
├── DSP2833x_* # TI-provided device support files
├── .project / .cproject # CCS project files
└── README.md


---

## 🛠️ How It Works

1. **PWM Signal Generation**
   - `ePWM1A` generates a 2kHz test signal (25% duty)
   - `Setup_ePWM1A()` configures PWM period and duty cycle

2. **ADC Sampling**
   - `ADCINA2` is sampled at 50kHz using `Timer0` (20 µs interval)
   - Each new sample triggers `adc_isr()`

3. **FIR Filtering**
   - Each ADC value is converted from 12-bit to IQ format using `_IQ16toIQ()`
   - Filtered using `IQssfir(xBuffer, coeffs, 5)`
   - Result stored in `AdcBufFiltered[]`

4. **LED Counter**
   - Main loop updates GPIO LEDs every 100ms (after 5000 samples)
   - Binary counter value shown using GPIO9, GPIO11, GPIO34, GPIO49

---

## 🧠 Useful For:

- Learning real-time digital filtering on DSPs
- IQmathLib usage (fixed-point math)
- Embedded PWM/ADC signal chain processing
- Visualization of filtering effects (can be observed in CCS Graph)

---

## 📈 Visualizing the Output

In **Code Composer Studio (CCS)**, use:

- **Tools → Graph → Single Time**
  - Address: `&AdcBuf[0]` or `&AdcBufFiltered[0]`
  - Display size: 50
  - Sampling rate: 1
  - Format: Q15 or floating-point, depending on build

---

## 🚧 Future Improvements

- [ ] Add UART output for real-time data monitoring
- [ ] Support dynamic filter coefficient update
- [ ] Implement variable sample rates
- [ ] Integrate with FFT or frequency analyzer

---

## 📄 License

This project is provided for educational and academic use. Contact the author for alternative licensing options.

