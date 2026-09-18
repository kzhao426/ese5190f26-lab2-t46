/* Part C: measure button timing and print DOT, DASH, or SPACE. */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/atomic.h>

#include "uart.h"

#define BUTTON_PIN PB0

#define TIMER_PRESCALER 1024UL
#define TICKS_FOR_MS(ms) \
    ((uint16_t)(((F_CPU / TIMER_PRESCALER) * (ms)) / 1000UL))

#define DEBOUNCE_TICKS TICKS_FOR_MS(10)
#define DOT_MIN_TICKS  TICKS_FOR_MS(50)
#define DASH_MIN_TICKS TICKS_FOR_MS(200)
#define DASH_MAX_TICKS TICKS_FOR_MS(400)
#define SPACE_TICKS    TICKS_FOR_MS(400)

#define EVENT_DOT   _BV(0)
#define EVENT_DASH  _BV(1)
#define EVENT_SPACE _BV(2)

static volatile uint8_t button_down;
static volatile uint8_t events;

ISR(TIMER1_CAPT_vect)
{
    uint16_t elapsed = ICR1;

    if (button_down) {
        /* Ignore very short falling edges caused by button bounce. */
        if (elapsed < DOT_MIN_TICKS) {
            TIFR1 = _BV(ICF1);
            return;
        }

        if (elapsed < DASH_MIN_TICKS) {
            events |= EVENT_DOT;
        } else if (elapsed <= DASH_MAX_TICKS) {
            events |= EVENT_DASH;
        }

        button_down = 0;
        TCNT1 = 0;
        OCR1A = SPACE_TICKS;
        TIFR1 = _BV(OCF1A);
        TIMSK1 |= _BV(OCIE1A);
        TCCR1B |= _BV(ICES1); /* Capture a rising edge next. */
    } else {
        /* Ignore very short rising edges caused by button bounce. */
        if (elapsed < DEBOUNCE_TICKS) {
            TIFR1 = _BV(ICF1);
            return;
        }

        TIMSK1 &= (uint8_t)~_BV(OCIE1A);
        button_down = 1;
        TCNT1 = 0;
        TCCR1B &= (uint8_t)~_BV(ICES1); /* Capture a falling edge next. */
    }

    TIFR1 = _BV(ICF1);
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

int main(void)
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
    events = 0;

    /* Use Timer1 with a 1/1024 prescaler. */
    TCCR1B = _BV(CS12) | _BV(CS10);
    if (!button_down) {
        TCCR1B |= _BV(ICES1); /* Start by waiting for a button press. */
    }

    TIFR1 = _BV(ICF1) | _BV(OCF1A);
    TIMSK1 = _BV(ICIE1);
    sei();

    printf("Part C ready\r\n");

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
