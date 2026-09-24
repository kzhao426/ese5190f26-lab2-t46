/* Part C: measure button timing and print DOT, DASH, or SPACE. */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "uart.h"

#define BUTTON_PIN PB0

#define TIMER_PRESCALER 1024UL
#define TICKS_FOR_MS(ms) \
    ((uint16_t)(((F_CPU / TIMER_PRESCALER) * (ms)) / 1000UL))

#define DOT_MIN_TICKS  TICKS_FOR_MS(50)
#define DASH_MIN_TICKS TICKS_FOR_MS(200)
#define DASH_MAX_TICKS TICKS_FOR_MS(400)
#define SPACE_TICKS    TICKS_FOR_MS(400)

#define EVENT_DOT   _BV(0)
#define EVENT_DASH  _BV(1)
#define EVENT_SPACE _BV(2)

static volatile uint8_t button_down;
static volatile uint8_t overflowed;
static volatile uint8_t events;

ISR(TIMER1_OVF_vect)
{
    overflowed = 1;
}

ISR(TIMER1_CAPT_vect)
{
    /* Confirm the new level after switch bounce settles. */
    _delay_ms(10);
    uint8_t down = (PINB & _BV(BUTTON_PIN)) != 0;
    if (down == button_down) {
        TIFR1 = _BV(ICF1);
        return;
    }

    uint16_t elapsed = TCNT1;
    uint8_t too_long = overflowed || (TIFR1 & _BV(TOV1));

    if (button_down) {
        if (!too_long && elapsed >= DOT_MIN_TICKS && elapsed <= DASH_MAX_TICKS) {
            events |= elapsed < DASH_MIN_TICKS ? EVENT_DOT : EVENT_DASH;
        }

        TIMSK1 |= _BV(OCIE1A);
        TCCR1B |= _BV(ICES1); /* Capture a rising edge next. */
    } else {
        /* Preserve a space whose compare interrupt is still pending. */
        if ((TIMSK1 & _BV(OCIE1A)) && (too_long || elapsed >= SPACE_TICKS)) {
            events |= EVENT_SPACE;
        }

        TIMSK1 &= (uint8_t)~_BV(OCIE1A);
        TCCR1B &= (uint8_t)~_BV(ICES1); /* Capture a falling edge next. */
    }

    button_down = down;
    TCNT1 = 0;
    overflowed = 0;
    TIFR1 = _BV(ICF1) | _BV(OCF1A) | _BV(TOV1);
}

ISR(TIMER1_COMPA_vect)
{
    if (!button_down) {
        events |= EVENT_SPACE;
    }

    /* Print only one space for each release. */
    TIMSK1 &= (uint8_t)~_BV(OCIE1A);
    TIFR1 = _BV(OCF1A);
}

static void part_c(void)
{
    cli();

    /* PB0/ICP1 uses the external pull-down resistor. */
    DDRB &= (uint8_t)~_BV(DDB0);
    PORTB &= (uint8_t)~_BV(PORTB0);

    uart_init();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = SPACE_TICKS;

    button_down = (PINB & _BV(BUTTON_PIN)) != 0;
    overflowed = 0;
    events = 0;

    /* Use Timer1 with a 1/1024 prescaler. */
    TCCR1B = _BV(CS12) | _BV(CS10);
    if (!button_down) {
        TCCR1B |= _BV(ICES1); /* Start by waiting for a button press. */
    }

    TIFR1 = _BV(ICF1) | _BV(OCF1A) | _BV(TOV1);
    TIMSK1 = _BV(ICIE1) | _BV(TOIE1);
    sei();

    while (1) {
        uint8_t pending;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            pending = events;
            events = 0;
        }

        if (pending & EVENT_DOT) {
            printf("DOT\r\n");
        }
        if (pending & EVENT_DASH) {
            printf("DASH\r\n");
        }
        if (pending & EVENT_SPACE) {
            printf("SPACE\r\n");
        }
    }
}

int main(void)
{
    part_c();
}
