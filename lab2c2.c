/* Part E: decode Morse code and print the result over UART. */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/atomic.h>

#include "uart.h"

#define BUTTON_PIN PB0
#define DOT_LED_PIN PB1
#define DASH_LED_PIN PB2
#define LED_MASK ((uint8_t)(_BV(DOT_LED_PIN) | _BV(DASH_LED_PIN)))

#define TIMER_PRESCALER 1024UL
#define TICKS_FOR_MS(ms) \
    ((uint16_t)(((F_CPU / TIMER_PRESCALER) * (ms)) / 1000UL))

#define DEBOUNCE_TICKS TICKS_FOR_MS(10)
#define DOT_MIN_TICKS  TICKS_FOR_MS(50)
#define DASH_MIN_TICKS TICKS_FOR_MS(200)
#define DASH_MAX_TICKS TICKS_FOR_MS(400)
#define LED_TIME_TICKS TICKS_FOR_MS(50)
#define LETTER_GAP_TICKS TICKS_FOR_MS(400)

/* Binary Morse tree: dot goes left, dash goes right. */
static const char morse_tree[64] PROGMEM = {
    [2] = 'E', [3] = 'T',
    [4] = 'I', [5] = 'A', [6] = 'N', [7] = 'M',
    [8] = 'S', [9] = 'U', [10] = 'R', [11] = 'W',
    [12] = 'D', [13] = 'K', [14] = 'G', [15] = 'O',
    [16] = 'H', [17] = 'V', [18] = 'F', [20] = 'L',
    [22] = 'P', [23] = 'J', [24] = 'B', [25] = 'X',
    [26] = 'C', [27] = 'Y', [28] = 'Z', [29] = 'Q',
    [32] = '5', [33] = '4', [35] = '3', [39] = '2',
    [47] = '1', [48] = '6', [56] = '7', [60] = '8',
    [62] = '9', [63] = '0'
};

static volatile uint8_t button_down;
static volatile uint8_t morse_code;
static volatile uint8_t morse_length;
static volatile uint8_t pending_code;
static volatile uint8_t pending_length;
static volatile uint8_t character_ready;

static void add_symbol(uint8_t is_dash)
{
    if (morse_length < 5) {
        morse_code = (uint8_t)(morse_code << 1);
        if (is_dash) {
            morse_code |= 1U;
        }
        morse_length++;
    } else {
        morse_length = 6; /* Mark an overlong sequence as invalid. */
    }
}

static char decode_morse(uint8_t code, uint8_t length)
{
    if ((length == 0) || (length > 5) || (code >= 64)) {
        return '?';
    }

    char result = (char)pgm_read_byte(&morse_tree[code]);
    return result == 0 ? '?' : result;
}

ISR(TIMER1_CAPT_vect)
{
    uint16_t elapsed = ICR1;

    if (button_down) {
        /* Ignore short falling edges caused by button bounce. */
        if (elapsed < DOT_MIN_TICKS) {
            TIFR1 = _BV(ICF1);
            return;
        }

        if (elapsed < DASH_MIN_TICKS) {
            add_symbol(0);
            PORTB = (uint8_t)((PORTB & (uint8_t)~LED_MASK)
                              | _BV(DOT_LED_PIN));
        } else if (elapsed <= DASH_MAX_TICKS) {
            add_symbol(1);
            PORTB = (uint8_t)((PORTB & (uint8_t)~LED_MASK)
                              | _BV(DASH_LED_PIN));
        }

        button_down = 0;
        TCNT1 = 0;
        OCR1B = LED_TIME_TICKS;
        OCR1A = LETTER_GAP_TICKS;
        TIFR1 = _BV(OCF1A) | _BV(OCF1B);
        TIMSK1 |= _BV(OCIE1A) | _BV(OCIE1B);
        TCCR1B |= _BV(ICES1); /* Capture a rising edge next. */
    } else {
        /* Ignore short rising edges caused by button bounce. */
        if (elapsed < DEBOUNCE_TICKS) {
            TIFR1 = _BV(ICF1);
            return;
        }

        TIMSK1 &= (uint8_t)~(_BV(OCIE1A) | _BV(OCIE1B));
        PORTB &= (uint8_t)~LED_MASK;
        button_down = 1;
        TCNT1 = 0;
        TCCR1B &= (uint8_t)~_BV(ICES1); /* Capture a falling edge next. */
    }

    TIFR1 = _BV(ICF1);
}

ISR(TIMER1_COMPB_vect)
{
    PORTB &= (uint8_t)~LED_MASK;
    TIMSK1 &= (uint8_t)~_BV(OCIE1B);
    TIFR1 = _BV(OCF1B);
}

ISR(TIMER1_COMPA_vect)
{
    if (!button_down && (morse_length > 0)) {
        pending_code = morse_code;
        pending_length = morse_length;
        character_ready = 1;

        morse_code = 1;
        morse_length = 0;
    }

    PORTB &= (uint8_t)~LED_MASK;
    TIMSK1 &= (uint8_t)~(_BV(OCIE1A) | _BV(OCIE1B));
    TIFR1 = _BV(OCF1A);
}

int main(void)
{
    cli();

    /* PB0 is the button input. PB1 and PB2 drive the two LEDs. */
    DDRB &= (uint8_t)~_BV(DDB0);
    PORTB &= (uint8_t)~_BV(PORTB0);
    PORTB &= (uint8_t)~LED_MASK;
    DDRB |= LED_MASK;

    uart_init();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = LETTER_GAP_TICKS;
    OCR1B = LED_TIME_TICKS;

    button_down = (PINB & _BV(BUTTON_PIN)) != 0;
    morse_code = 1;
    morse_length = 0;
    character_ready = 0;

    /* Run Timer1 at 16 MHz / 1024. */
    TCCR1B = _BV(CS12) | _BV(CS10);
    if (!button_down) {
        TCCR1B |= _BV(ICES1); /* Start by waiting for a button press. */
    }

    TIFR1 = _BV(ICF1) | _BV(OCF1A) | _BV(OCF1B);
    TIMSK1 = _BV(ICIE1);
    sei();

    printf("Part E ready\r\n");

    while (1) {
        uint8_t code = 0;
        uint8_t length = 0;
        uint8_t ready = 0;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            if (character_ready) {
                code = pending_code;
                length = pending_length;
                character_ready = 0;
                ready = 1;
            }
        }

        if (ready) {
            printf("%c", decode_morse(code, length));
        }
    }
}
