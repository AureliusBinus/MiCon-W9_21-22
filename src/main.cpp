#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

// F_CPU is already defined by Arduino framework
// Removed redundant #define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR ((F_CPU / 16 / BAUD) - 1)

volatile uint8_t tx_buffer[256];
volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;
volatile uint8_t timer_flag = 0;

// RX buffer for data reception
volatile uint8_t rx_buffer[256];
volatile uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0;
volatile uint8_t rx_timeout = 0;

void uart_init()
{
  // Correct register names for ATmega328P
  UBRR0H = (MYUBRR >> 8);
  UBRR0L = MYUBRR;
  UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data (no URSEL bit in ATmega328P)
}

void timer1_init()
{
  TCCR1B |= (1 << WGM12);              // CTC mode
  OCR1A = 15624;                       // 1s delay with 16MHz and prescaler 1024
  TIMSK1 |= (1 << OCIE1A);             // Enable Timer1 compare interrupt (corrected register)
  TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
}

// Timer0 for RX timeout detection
void timer0_init()
{
  TCCR0A |= (1 << WGM01);              // CTC mode (corrected register)
  OCR0A = 250;                         // ~16ms at 16MHz/1024 (corrected register)
  TIMSK0 |= (1 << OCIE0A);             // Enable Timer0 compare interrupt (corrected register)
  TCCR0B |= (1 << CS02) | (1 << CS00); // Prescaler 1024 (corrected register)
}

ISR(USART_RX_vect)
{
  // Store received byte in RX buffer
  rx_buffer[rx_head] = UDR0; // Corrected register
  rx_head = (rx_head + 1) % 256;

  // Echo character (also store in TX buffer)
  uint8_t next_tx_head = (tx_head + 1) % 256;
  if (next_tx_head != tx_tail)
  {
    tx_buffer[tx_head] = rx_buffer[(rx_head - 1) % 256];
    tx_head = next_tx_head;
    UCSR0B |= (1 << UDRIE0); // Enable TX interrupt (corrected register)
  }

  // Reset timeout timer
  TCNT0 = 0;
  TIMSK0 |= (1 << OCIE0A); // Enable Timer0 interrupt (corrected register)
}

ISR(USART_UDRE_vect)
{
  if (tx_tail != tx_head)
  {
    UDR0 = tx_buffer[tx_tail]; // Corrected register
    tx_tail = (tx_tail + 1) % 256;
  }
  else
  {
    UCSR0B &= ~(1 << UDRIE0); // Disable TX interrupt (corrected register)
  }
}

ISR(TIMER1_COMPA_vect)
{
  timer_flag = 1; // Set flag every second
}

ISR(TIMER0_COMPA_vect) // Corrected interrupt vector
{
  rx_timeout = 1;           // Set flag on timeout
  TIMSK0 &= ~(1 << OCIE0A); // Disable Timer0 interrupt (corrected register)
}

int main()
{
  uart_init();
  timer1_init();
  timer0_init(); // Initialize Timer0 for RX timeout detection
  sei();         // Enable global interrupts

  const char *message = "ATmega328P: Hello World!\r\n"; // Updated to match actual chip
  uint8_t msg_len = strlen(message);

  while (1)
  {
    // Check for timed out RX data (complete message received)
    if (rx_timeout && rx_head != rx_tail)
    {
      // Process received data
      // For this example, we've already echoed the characters in the RX ISR

      // Print information about received data
      char info_msg[64];
      uint8_t bytes_received = (rx_head >= rx_tail) ? (rx_head - rx_tail) : (256 + rx_head - rx_tail);
      sprintf(info_msg, "\r\nReceived %d bytes\r\n", bytes_received);

      // Add info message to TX buffer with timeout protection
      for (uint8_t i = 0; info_msg[i] != '\0'; i++)
      {
        uint8_t next_head = (tx_head + 1) % 256;
        uint8_t timeout_counter = 0;
        while (next_head == tx_tail)
        {
          // Add timeout protection to prevent hanging
          timeout_counter++;
          if (timeout_counter > 250)
            break;
        }

        // Only write to buffer if we didn't time out
        if (timeout_counter <= 250)
        {
          tx_buffer[tx_head] = info_msg[i];
          tx_head = next_head;
        }
      }

      // Reset RX buffer
      rx_tail = rx_head;
      rx_timeout = 0;

      // Enable TX interrupt to send data
      UCSR0B |= (1 << UDRIE0); // Corrected register
    }

    if (timer_flag)
    {
      timer_flag = 0;
      // Add message to TX buffer
      for (uint8_t i = 0; i < msg_len; i++)
      {
        uint8_t next_head = (tx_head + 1) % 256;
        uint8_t timeout_counter = 0;
        while (next_head == tx_tail)
        {
          // Add timeout protection to prevent hanging
          timeout_counter++;
          if (timeout_counter > 250)
            break;
        }

        // Only write to buffer if we didn't time out
        if (timeout_counter <= 250)
        {
          tx_buffer[tx_head] = message[i];
          tx_head = next_head;
        }
      }
      UCSR0B |= (1 << UDRIE0); // Trigger TX (corrected register)
    }
  }
}