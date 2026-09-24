/* Part E: decode Morse input on PB0 and print each letter after a 400 ms gap.
 * PB0 uses an external pull-down; PB1/PB2 are the dot/dash LEDs.
 */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "uart.h"

#define BUTTON_PIN PB0
#define DOT_LED_PIN PB1
#define DASH_LED_PIN PB2
#define LED_MASK ((uint8_t)(_BV(DOT_LED_PIN) | _BV(DASH_LED_PIN)))

#define TIMER_PRESCALER 1024UL
#define TICKS_FOR_MS(ms) \
    ((uint16_t)(((F_CPU / TIMER_PRESCALER) * (ms)) / 1000UL))

#define DOT_MIN_TICKS  TICKS_FOR_MS(50)
#define DASH_MIN_TICKS TICKS_FOR_MS(200)
#define DASH_MAX_TICKS TICKS_FOR_MS(800)
#define LED_TIME_TICKS TICKS_FOR_MS(50)
#define LETTER_GAP_TICKS TICKS_FOR_MS(400)

/* Start at index 1: dot = index * 2, dash = index * 2 + 1. */
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

/* Button state, current symbols, and the next character to print. */
static volatile uint8_t button_down;
static volatile uint8_t overflowed;
static volatile uint8_t morse_code;
static volatile uint8_t morse_length;
static volatile uint8_t pending_code;
static volatile uint8_t pending_length;
static volatile uint8_t character_ready;

/* Add a dot or dash to the current character. */
static void add_symbol(uint8_t is_dash)
{
    if (morse_length < 5) {
        morse_code = (uint8_t)(morse_code << 1);
        if (is_dash) {
            morse_code |= 1U;
        }
        morse_length++;
    } else {
        morse_length = 6; /* More than five symbols is invalid. */
    }
}

/* Look up the character, or return ? for invalid input. */
static char decode_morse(uint8_t code, uint8_t length)
{
    if ((length == 0) || (length > 5) || (code >= 64)) {
        return '?';
    }

    char result = (char)pgm_read_byte(&morse_tree[code]);
    return result == 0 ? '?' : result;
}

static void finish_character(void)
{
    if (morse_length > 0) {
        pending_code = morse_code;
        pending_length = morse_length;
        character_ready = 1;
        morse_code = 1;
        morse_length = 0;
    }
}

/* Reject presses that outlast the timer. */
ISR(TIMER1_OVF_vect)
{
    overflowed = 1;
}

/* Wait 10 ms to debounce the button. */
ISR(TIMER1_CAPT_vect)
{
    _delay_ms(10);
    uint8_t pressed = (PINB & _BV(BUTTON_PIN)) != 0;
    if (pressed == button_down) {
        TIFR1 = _BV(ICF1);
        return;
    }

    uint16_t elapsed = TCNT1;
    uint8_t too_long = overflowed || (TIFR1 & _BV(TOV1));

    /* Keep the remaining LED time when resetting Timer1. */
    if (TIMSK1 & _BV(OCIE1B)) {
        if (too_long || elapsed >= OCR1B) {
            PORTB &= (uint8_t)~LED_MASK;
            TIMSK1 &= (uint8_t)~_BV(OCIE1B);
        } else {
            OCR1B -= elapsed;
        }
    }

    if (button_down) {
        /* On release, accept dots from 50 to <200 ms and dashes from 200 to 800 ms. */
        if (!too_long && elapsed >= DOT_MIN_TICKS && elapsed <= DASH_MAX_TICKS) {
            uint8_t dash = elapsed >= DASH_MIN_TICKS;
            add_symbol(dash);
            PORTB = (uint8_t)((PORTB & (uint8_t)~LED_MASK) |
                              _BV(dash ? DASH_LED_PIN : DOT_LED_PIN));
            OCR1B = LED_TIME_TICKS;
            TIMSK1 |= _BV(OCIE1B);
        }
        button_down = 0;
        TCCR1B |= _BV(ICES1);
        TIMSK1 |= _BV(OCIE1A);
    } else {
        /* Handle an expired gap before starting a new press. */
        if (too_long || elapsed >= LETTER_GAP_TICKS) {
            finish_character();
        }
        /* Stop gap timing and start measuring the press. */
        button_down = 1;
        TCCR1B &= (uint8_t)~_BV(ICES1);
        TIMSK1 &= (uint8_t)~_BV(OCIE1A);
    }

    /* Restart timing after each edge. */
    TCNT1 = 0;
    overflowed = 0;
    TIFR1 = _BV(ICF1) | _BV(OCF1A) | _BV(OCF1B) | _BV(TOV1);
}

/* Turn off the LEDs after 50 ms. */
ISR(TIMER1_COMPB_vect)
{
    PORTB &= (uint8_t)~LED_MASK;
    TIMSK1 &= (uint8_t)~_BV(OCIE1B);
    TIFR1 = _BV(OCF1B);
}

/* Finish the character after a 400 ms gap. */
ISR(TIMER1_COMPA_vect)
{
    if (!button_down) {
        finish_character();
    }

    PORTB &= (uint8_t)~LED_MASK;
    TIMSK1 &= (uint8_t)~(_BV(OCIE1A) | _BV(OCIE1B));
    TIFR1 = _BV(OCF1A);
}

static void part_e(void)
{
    cli();

    /* Set up the button, LEDs, and UART. */
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

    /* Use a 1/1024 prescaler and capture each button edge. */
    TCCR1B = _BV(CS12) | _BV(CS10);
    if (!button_down) {
        TCCR1B |= _BV(ICES1); /* Wait for a press. */
    }

    TIFR1 = _BV(ICF1) | _BV(OCF1A) | _BV(OCF1B) | _BV(TOV1);
    TIMSK1 = _BV(ICIE1) | _BV(TOIE1);
    sei();

    while (1) {
        uint8_t code = 0;
        uint8_t length = 0;
        uint8_t ready = 0;

        /* Copy the character safely, then print outside the ISR. */
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

int main(void)
{
    part_e();
}
