#include "serial.h"

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" ::"a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void serial_init(void)
{
    outb(COM1 + 1, 0x00); // disable all interrupts
    outb(COM1 + 3, 0x80); // enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03); // divisor = 3 (lo byte) -> 38400 baud
    outb(COM1 + 1, 0x00); //           (hi byte)
    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // enable FIFO, clear them, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs disabled, RTS/DSR set (normal operation)
}

static int transmit_empty(void)
{
    return inb(COM1 + 5) & 0x20;
}

void serial_putchar(char c)
{
    if (c == '\n')
        serial_putchar('\r');

    while (!transmit_empty())
        ;
    outb(COM1, (uint8_t)c);
}

void serial_write(const char *s)
{
    while (s && *s)
        serial_putchar(*s++);
}
