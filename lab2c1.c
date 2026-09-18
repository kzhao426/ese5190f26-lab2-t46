/* Part B: use the Timer1 input capture interrupt to control the LED. */
#include <avr/io.h>
#include <avr/interrupt.h>

#define BUTTON_PIN PB0
#define LED_PIN    PB5

ISR(TIMER1_CAPT_vect)
{
    if (PINB & _BV(BUTTON_PIN)) {
        /* Button pressed: LED on, then wait for release. */
        PORTB |= _BV(LED_PIN);
        TCCR1B &= (uint8_t)~_BV(ICES1);
    } else {
        /* Button released: LED off, then wait for the next press. */
        PORTB &= (uint8_t)~_BV(LED_PIN);
        TCCR1B |= _BV(ICES1);
    }

    /* Clear the input capture flag. */
    TIFR1 = _BV(ICF1);
}

int main(void)
{
    cli();

    /* Set PB0 as the button input. */
    DDRB &= (uint8_t)~_BV(DDB0);
    PORTB &= (uint8_t)~_BV(PORTB0);

    /* Set PB5 as the LED output. */
    PORTB &= (uint8_t)~_BV(PORTB5);
    DDRB |= _BV(DDB5);

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    if (PINB & _BV(BUTTON_PIN)) {
        PORTB |= _BV(LED_PIN);
        /* The button is already pressed, so wait for a falling edge. */
        TCCR1B = _BV(CS11) | _BV(CS10); /* Timer1 clock = F_CPU / 64. */
    } else {
        /* The button is released, so wait for a rising edge. */
        TCCR1B = _BV(ICES1) | _BV(CS11) | _BV(CS10);
    }

    TIFR1 = _BV(ICF1);   /* Clear the flag before starting. */
    TIMSK1 = _BV(ICIE1); /* Enable the input capture interrupt. */
    sei();

    while (1) {
        /* The interrupt handles the button. */
    }
}
