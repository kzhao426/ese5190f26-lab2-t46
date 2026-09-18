# Part E: LEDs and ASCII

## Wiring

- Button: PB0/ICP1 with a 10 kOhm external pull-down resistor
- Dot LED: PB1 through a current-limiting resistor to GND
- Dash LED: PB2 through a current-limiting resistor to GND
- Serial monitor: 9600 baud, 8 data bits, no parity, 2 stop bits

## Timing

- Dot: button held for 50 ms to less than 200 ms
- Dash: button held for 200 ms to 400 ms
- Character gap: button released for at least 400 ms
- Dot or dash LED: on for about 50 ms after release

The decoded A-Z or 0-9 characters are printed continuously in the serial
terminal. An invalid or overlong sequence prints `?`.

## T1 demo checklist

1. Show the board connected to the computer.
2. Show the PB0 button and PB1/PB2 LED wiring.
3. State the word before entering it.
4. Enter a word containing at least five different characters.
5. Enter the same word a second time.
6. Show the LEDs and the two matching words in the serial terminal.

`BRAIN` is one possible five-character test word:

```text
B  -...
R  .-.
A  .-
I  ..
N  -.
```
