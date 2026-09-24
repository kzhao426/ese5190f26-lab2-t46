# Lab 2 Morse

**Team Name/Number: Team 46**

**GitHub Repository URL: [github.com/kzhao426/ese5190f26-lab2-t46.git](https://github.com/kzhao426/ese5190f26-lab2-t46.git)**

| Team Member Name | Email Address                  | GitHub Handle |
| ---------------- | ------------------------------ | ------------- |
| Liyan Luo        | ll0@engineering.upenn.edu      | liyanluo-penn |
| Kevin Zhao       | kzhao426@engineering.upenn.edu | kzhao426      |

# Questions

## Part A

### (S1)

![Loop Blink Code](images/lab2s1.png)

### (S2)

![Button LED Code](images/lab2s2.png)

Correction: the condition should be `if (PINB & (1 << BUTTON_PIN))` to read PB0 without writing to PINB.

### (S3)

![LED Cycle Code](images/lab2s3.png)

### (S4)

![Circuit Schematic](images/lab2s4.png)

## Part B

### (C1)

[lab2c1.c](lab2c1.c)

## Part C

### (R1)

| Time (ms) | Ticks     |
| --------- | --------- |
| 50        | 800,000   |
| 200       | 3,200,000 |
| 400       | 6,400,000 |

### (R2)

A prescaler divides the microcontroller clock before it reaches the timer, so the timer counts more slowly and measures longer intervals before overflowing. The tradeoff is lower timing resolution; with a 1/1024 prescaler at 16 MHz, each tick is 64 us and Timer1 overflows after about 4.19 s instead of 4.10 ms.

## Part D

### (I1)

Pending: circuit photo (R1 = 1 kOhm, R2 = 10 kOhm).

### (I2)

Pending: oscilloscope capture with C1 = 1 nF.

### (I3)

Pending: oscilloscope capture with C1 = 10 nF.

### (R3)

The 10 nF capacitor is better for debouncing. With the same resistance, its RC time constant is ten times larger than with 1 nF, so it smooths fast voltage changes more effectively but responds more slowly.

## Part E

### (C2)

[lab2c2.c](lab2c2.c) decodes A–Z and 0–9 and prints the characters over UART. Invalid sequences print `?`. The same code is in [src/main.c](src/main.c) for PlatformIO.

- Button: PB0/ICP1 with a 10 kOhm external pull-down resistor.
- LEDs: two different colors, PB1 for dots and PB2 for dashes, each with a current-limiting resistor to GND.
- Serial: 9600 baud, 8 data bits, no parity, 2 stop bits (8N2).
- Timing: dot = 50 ms to less than 200 ms; dash = 200–800 ms; character gap = at least 400 ms. Each LED lights for about 50 ms after release.

The dash upper limit was increased from 400 ms to 800 ms to make manual input easier.

### (T1)

Pending: TA demo with both team members. Announce and enter `BRAIN` (five different characters) twice, showing the board connected to the computer, button/LED wiring, button presses, LEDs, and terminal output.
