#include <Arduino.h>

#define DAC_BCK_PIN  PB3    // Pin 11 on Arduino Uno, Pin 1 on TM8211
#define DAC_WS_PIN   PB5    // Pin 13 on Arduino Uno, Pin 2 on TM8211
#define DAC_DATA_PIN PB4    // Pin 12 on Arduino Uno, Pin 3 on TM8211

#define DAC_BITS 16

#define INT16_MAX 0x7FFF
#define INT16_MIN 0x8000

unsigned int dac_bit_counter;
unsigned int data;

// Interrupt Timer initialisation
void init_timer() {
    cli();              // Disable interrupts during initialisation
    TCCR1A = 0;         // Reset Timer 1 Counter Control Register A
    TCCR1B = 0;         // Reset Timer 1 Counter Control Register B

    // OCR1A = (frequency_clock / (prescalar * frequency_interrupt)) - 1

    OCR1A = 7;          // Set Timer 1 Output Compare Register -  bounds

    TCCR1B |= (1 << WGM12); // Turn on CTC (clear timer on compare match) mode
    TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);  // Prescalar = 64

    TIMSK1 |= (1 << OCIE1A);        // Enable timer interrupt
    sei();                          // enable interrupts after initialisation
}

void setup() {
    // Setup Output Pins
    DDRB |= (1 << DAC_BCK_PIN) | (1 << DAC_WS_PIN) | (1 << DAC_DATA_PIN);

    dac_bit_counter = 0;
    data = 0;

    init_timer();
}

// control flow is given to interrupt service routine
void loop() {}

/**
 * Interrupt Service Routine, I2S DAC control
 */
ISR(TIMER1_COMPA_vect) {
    if (!(PORTB & (1 << DAC_BCK_PIN))) {
        if (!dac_bit_counter)
            PORTB ^= (1 << DAC_WS_PIN);

        boolean data_bit = data & (1 << DAC_BITS - 1 - dac_bit_counter);
        if (data_bit)

            PORTB |= (1 << DAC_DATA_PIN);
        else

            PORTB &= ~(1 << DAC_DATA_PIN);

        dac_bit_counter++;
        dac_bit_counter %= DAC_BITS;
        data += 16;
    }

    PORTB ^= (1 << DAC_BCK_PIN);
}
