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

<img width="440" height="461" alt="image" src="https://github.com/user-attachments/assets/49cbccb6-6bef-4653-be49-cfd4e33bf0a9" />


### (S2)

<img width="494" height="500" alt="image" src="https://github.com/user-attachments/assets/0212a6c6-1213-4346-bc54-d24466f30f54" />


Correction: the condition should be `if (PINB & (1 << BUTTON_PIN))` to read PB0 without writing to PINB.

### (S3)

<img width="401" height="623" alt="image" src="https://github.com/user-attachments/assets/7be75652-4f2b-43c5-bcb3-d9d4e0eb8978" />

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

A prescaler divides the microcontroller clock before it's used for timers, so the timers count more slowly and takes longer before overflowing. With the different prescaler values that can be selected for the microcontroller, we can better tune the time until overflow for the situation.

## Part D

### (I1)
<img width="361" height="491" alt="image" src="https://github.com/user-attachments/assets/914a1cb9-7f8e-4ba3-ae02-0d4f26f4f628" />


### (I2)

<img width="651" height="489" alt="image" src="https://github.com/user-attachments/assets/b6aa158e-057d-4821-a421-415aebdef0cd" />


### (I3)

<img width="656" height="472" alt="image" src="https://github.com/user-attachments/assets/ffe1c36b-8e96-4cab-9c81-9b4014e318a9" />


### (R3)

The 10 nF capacitor is better for debouncing. With the same resistance, it can store and release more power, so it smooths fast voltage changes more effectively but responds more slowly.

## Part E

### (C2)

[lab2c2.c](lab2c2.c)

The dash upper limit was increased from 400 ms to 800 ms to make manual input easier.

## Part F

### (C3)

[lab2c3.c](lab2c3.c)

Message: `penn ESE 5190 fall 2026` (13 unique characters).


