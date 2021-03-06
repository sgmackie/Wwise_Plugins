# Wwise Plugins

- [About](#about)
- [Wavetable](#wavetable)
- [Bitcrusher](#bitcrusher)
- [Transient Shaper](#transient)
- [References](#references)

### About: <a name="about"></a>

Various source and effect plugins created for Wwise.

### Wavetable: <a name="wavetable"></a>

Uses an FFT function to split a single cycle waveform into harmonic bands that populate lookup tables. These tables are read via linear interpolation to produce the current output samples. Also includes a basic ADSR envelope that uses linear ramping to generate the current amplitude at a given sample index.

### Bitcrusher: <a name="bitcrusher"></a>

Bit rate reduction and downsampler plugin with optional hard/soft clipping distortion for the final output

### Transient Shaper: <a name="transient"></a>

Transient shaper that uses amplitude enevelopes to rescale the attack and sustain portions of an incoming signal, with optional soft-clipping saturation.

### References: <a name="references"></a>

- DSP Theory:
  - The Audio Programming Book (Richard Boulanger and Victor Lazzarini)
  - Understanding Digital Signal Processing (Richard G. Lyons)
  - Hack Audio (Eric Tarr)
  - CCRMA Stanford Index (Julius Orion Smith)