# Extio.dll for Enhanced Integration for RGO ONE and N1MM+ Logger

## Background

RGO ONE is a great CW transceiver. However, unlike modern SDR rigs, it doesn't have a built-in spectrum display, which makes it less convenient for search-and-pounce QSOs during contests. It does have an IF output, allowing the use of an SDR dongle like Airspy or SDRPlay to display the spectrum. The popular contest logger, N1MM+ Logger, supports Airspy and SDRPlay. However, using this setup as-is presents two issues:

1. **Lack of IF-based Spectrum Display Support**: N1MM+ Logger assumes that an external SDR gets the RF signal via a T/R switch, meaning the SDR LO frequency needs to be in the same band as the VFO of the radio is in. With IF, however, the SDR LO frequency should remain fixed. In my RGO ONE setup, it is 8.9989 MHz.

2. **Sideband Swap in IF Output**: The upper and lower sidebands in the IF output are swapped. This means, for example, when the radio's VFO is set to 14.050 MHz, FT8 signals at 14.074 MHz appear 24 kHz *lower* than the center in the spectrum display, instead of higher.

This project addresses these issues to ensure smooth integration with N1MM+ Logger, allowing RGO ONE to function effectively as a contest rig.

## How It Works

This project provides a shim layer `extio.dll` on top of the real `extio.dll`. The shim layer performs three main functions:

1. **Pass-Through Functionality**: For most function calls, it simply passes them through to the real `extio.dll`. This includes initialization, start/close operations, and querying the SDR hardware.

2. **Intercepting LO Frequency API Calls**: It intercepts the `SetHWLO()` and `GetHWLO()` APIs for setting and getting the hardware LO frequency of the SDR dongle. In this use case, the SDR hardware LO should always be set to the IF frequency. Thus:

    - When receiving a `SetHWLO(desired_freq)` call, it saves the `desired_freq` but sets the SDR to the IF frequency (8.9989 MHz in my setup) via the real `extio.dll`.

    - When receiving a `GetHWLO()` call, it returns the previously saved `desired_freq`, making N1MM+ Logger believe the SDR is set to the desired frequency.

3. **Swapping I and Q Samples**: It swaps the I and Q samples returned to N1MM+ Logger to ensure the upper and lower sidebands are displayed correctly in the spectrum display.

## How to Use

To use this shim layer:

Place both the shim layer `extio.dll` and the real `extio.dll` inside the N1MM+ Logger installation directory. The pre-built shim layer DLL is named `extio_rgo1_n1mm.dll`.

Rename the real `extio.dll` to `_extio_real.dll` (don't miss the leading underscore `_`).

Follow the N1MM+ Logger configuration instructions: [N1MM SDR Server – For radios with Extio dll support – THIS IS THE PREFERRED METHOD FOR SDRPlay and Airspy HF+/Discovery SDRs](https://n1mmwp.hamdocs.com/manual-windows/spectrum-display-window/#n1mm-sdr-server-for-radios-with-extio-dll-support-this-is-the-preferred-method-for-sdrplay-and-airspy-hf-discovery-sdrs).

Ensure the spectrum display mode is set to **Center**.

## How to Build

If you're not familiar with Visual Studio, you can use `mingw`. Since N1MM+ Logger is a 32-bit program, you will need `mingw32`. Download it using [msys2](https://www.msys2.org/). On a 64-bit Windows system, the required toolchain is `mingw-w64-i686-toolchain`.

### Build Command

Add the `mingw32` bin directory to your `PATH` environment variable. Once done, simply run:

```
g++ -Wall -shared -o extio_rgo1_n1mm.dll extio_rgo1_n1mm.cpp "-Wl,--kill-at"
```