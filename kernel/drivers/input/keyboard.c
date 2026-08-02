#include "keyboard.h"
#include "../../arch/x86_64/idt.h"
#include "../../lib/printf.h"
#include <stdint.h>

#define KBD_DATA 0x60
#define KBD_STATUS 0x64

static const char sc_normal[128] = {
    0,
    0,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b',
    '\t',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '7',
    '8',
    '9',
    '-',
    '4',
    '5',
    '6',
    '+',
    '1',
    '2',
    '3',
    '0',
    '.',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static const char sc_shifted[128] = {
    0,
    0,
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    '\b',
    '\t',
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    '\n',
    0,
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '"',
    '~',
    0,
    '|',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '7',
    '8',
    '9',
    '-',
    '4',
    '5',
    '6',
    '+',
    '1',
    '2',
    '3',
    '0',
    '.',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static int shift = 0;
static int caps = 0;

static inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    __asm__ volatile("inb %1,%0" : "=a"(v) : "Nd"(port));
    return v;
}

static inline void outb(uint16_t port, uint8_t val) __attribute__((unused));
static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0,%1" ::"a"(val), "Nd"(port));
}

void keyboard_init(void)
{
    while (inb(KBD_STATUS) & 0x01)
        inb(KBD_DATA);

    outb(KBD_STATUS, 0xAD);
    for (volatile int i = 0; i < 10000; i++)
        ;
    outb(KBD_STATUS, 0xA7);
    for (volatile int i = 0; i < 10000; i++)
        ;

    // flush again
    while (inb(KBD_STATUS) & 0x01)
        inb(KBD_DATA);

    outb(KBD_STATUS, 0x20);
    while (!(inb(KBD_STATUS) & 0x01))
        ;
    uint8_t config = inb(KBD_DATA);
    config |= 0x01;
    config |= 0x02;
    config &= ~0x20;
    outb(KBD_STATUS, 0x60);
    while (inb(KBD_STATUS) & 0x02)
        ;
    outb(KBD_DATA, config);

    outb(KBD_STATUS, 0xAE);
    for (volatile int i = 0; i < 10000; i++)
        ;

    outb(KBD_STATUS, 0xA8);
    for (volatile int i = 0; i < 10000; i++)
        ;

    while (inb(KBD_STATUS) & 0x01)
        inb(KBD_DATA);
}

int keyboard_haschar(void)
{
    uint8_t status = inb(KBD_STATUS);
    // Data available AND not mouse data (bit 5 must be 0 for keyboard)
    return (status & 0x01) && !(status & 0x20);
}

char keyboard_getchar(void)
{
    while (1)
    {
        while (!(inb(KBD_STATUS) & 0x01))
            ;

        // Check AUX flag (bit 5) - skip if this is mouse data, not keyboard
        uint8_t status = inb(KBD_STATUS);
        if (status & 0x20)
        {
            // This is mouse data, skip it
            inb(KBD_DATA);
            continue;
        }

        uint8_t sc = inb(KBD_DATA);

        if (sc & 0x80)
        {
            uint8_t rel = sc & 0x7f;
            if (rel == 0x2a || rel == 0x36)
                shift = 0;
            continue;
        }

        if (sc == 0x2a || sc == 0x36)
        {
            shift = 1;
            continue;
        }
        if (sc == 0x3a)
        {
            caps = !caps;
            continue;
        }
        if (sc == 0x1d || sc == 0x38)
            continue;
        if (sc >= 128)
            continue;

        int upper = shift ^ caps;
        char c = upper ? sc_shifted[sc] : sc_normal[sc];
        if (c)
            return c;
    }
}