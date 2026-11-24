# ENGIN346-Final-Project

Team 2 (P2): radar trigger and aperture-position timestamping unit. 

Team members:
- Ahmed Ahmed
- Shaliah Fricas
- Sidi Chakkour
- Murad Eyvazov

Open `project.uvprojx` in Keil uVision 5. All tunable constants are in `config.h`.

## Week 1

- TIM1 CH1 hardware trigger on PA8 (D7), 10 Hz to 10 kHz
- TIM2 timestamps at 80 MHz: trigger on PA0 (A0), strobe on PB10 (D6)
- Commands over the ST-Link COM port at 460800 baud

Wiring: jumper PA8 to PA0. Analog Discovery 2 strobe output (0 to 3.3 V) to PB10. All grounds tied.

| Command | What it does |
| --- | --- |
| `TRIG <hz>` | Starts the trigger on PA8 at 10 to 10000 Hz, e.g. `TRIG 1000` |
| `TRIGOFF` | Stops the trigger |
| `CAPTURE <n>` | Records the next n triggers and the strobes between them, then prints their times |
| `STAT?` | Shows the trigger rate, the PSC and ARR in use, and whether a capture is running |

Every command replies `OK` or `ERR <code> <reason>`.

Times in the dump are raw TIM2 ticks of 12.5 ns. Subtract them as unsigned 32-bit so the wrap at 53.7 s is harmless.
