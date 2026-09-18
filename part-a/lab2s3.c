/* Lab 2 Part A.2 (S3)
 * Button: PB0, external pull-down resistor, pressed = HIGH.
 * LEDs:   PB1, PB2, PB3, and PB4, active HIGH.
 * Each new button press advances to the next LED.
 */
#include <avr/io.h>
#include <util/delay.h>

#define LED_MASK ((uint8_t)(_BV(PB1) | _BV(PB2) | _BV(PB3) | _BV(PB4)))

int main(void)
{
    /* Configure PB0*/
    DDRB &= (uint8_t)~_BV(DDB0);
    PORTB &= (uint8_t)~_BV(PORTB0);

    /* Configure PB1-PB4 as outputs.*/
    PORTB = (uint8_t)((PORTB & (uint8_t)~LED_MASK) | _BV(PORTB1));
    DDRB |= LED_MASK;

    uint8_t led_index = 0;

    while (1) {
        /* A HIGH level means that the button has been pressed. */
        if (PINB & _BV(PINB0)) {
            _delay_ms(20); /* Debounce the press. */

            if (PINB & _BV(PINB0)) {
                led_index = (uint8_t)((led_index + 1U) & 0x03U);

                /* Clear all four LEDs, then turn on the selected one. */
                PORTB = (uint8_t)((PORTB & (uint8_t)~LED_MASK)
                                  | (uint8_t)(_BV(PB1) << led_index));

                /* Wait for release */
                while (PINB & _BV(PINB0)) {
                    /* Busy wait. */
                }
                _delay_ms(20); /* Debounce the release. */
            }
        }
    }
}
