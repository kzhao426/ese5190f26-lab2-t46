/* Part F: send a message as Morse code on PB1. */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PB1
#define UNIT_MS 100

static const char message[] = "penn ESE 5190 fall 2026";

static const char *const morse[] = {
    /* A-Z */
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",
    ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",
    "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
    /* 0-9 */
    "-----", ".----", "..---", "...--", "....-", ".....", "-....",
    "--...", "---..", "----."
};

static void wait_units(uint8_t units)
{
    while (units--) {
        _delay_ms(UNIT_MS);
    }
}

static void send_character(char c)
{
    if (c >= 'a' && c <= 'z') {
        c = (char)(c - 'a' + 'A');
    }
    const char *code;
    if (c >= 'A' && c <= 'Z') {
        code = morse[c - 'A'];
    } else if (c >= '0' && c <= '9') {
        code = morse[c - '0' + 26];
    } else {
        return;
    }

    while (*code) {
        PORTB |= _BV(LED_PIN);
        wait_units(*code == '.' ? 1 : 3);
        PORTB &= (uint8_t)~_BV(LED_PIN);
        code++;
        if (*code) {
            wait_units(1); /* Gap between symbols. */
        }
    }
}

static void send_message(const char *text)
{
    while (*text) {
        if (*text == ' ') {
            text++;
            continue;
        }
        send_character(*text++);
        /* Use a longer gap between words and message repeats. */
        wait_units(*text == ' ' || *text == '\0' ? 7 : 3);
    }
}

static void part_f(void)
{
    PORTB &= (uint8_t)~_BV(LED_PIN);
    DDRB |= _BV(LED_PIN);

    while (1) {
        send_message(message);
    }
}

int main(void)
{
    part_f();
}
