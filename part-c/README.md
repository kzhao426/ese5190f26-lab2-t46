# Part C: Dash or Dot

## R1

At 16 MHz, the number of CPU clock ticks is:

- 50 ms: `16,000,000 * 0.050 = 800,000` ticks
- 200 ms: `16,000,000 * 0.200 = 3,200,000` ticks
- 400 ms: `16,000,000 * 0.400 = 6,400,000` ticks

This implementation uses a Timer1 prescaler of 1024. The timer frequency is
15,625 Hz, so one Timer1 tick is 64 us. The corresponding Timer1 counts are
about 781, 3125, and 6250.

## R2

A prescaler divides the microcontroller clock before it reaches the timer. This
makes the timer count more slowly, so a 16-bit timer can measure longer time
intervals before overflowing. The tradeoff is lower timing resolution because
each timer tick represents more time. With the 1/1024 prescaler used here,
Timer1 overflows after about 4.19 seconds instead of about 4.10 ms.

## Expected serial output

The serial settings are 9600 baud, 8 data bits, no parity, and 2 stop bits.

- Hold the button for 50-200 ms: `DOT`
- Hold the button for 200-400 ms: `DASH`
- Leave the button released for at least 400 ms: `SPACE`

Transitions shorter than 50 ms while the button is down are ignored to reduce
the effect of switch bounce.
