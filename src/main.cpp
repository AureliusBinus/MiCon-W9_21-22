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
  // Clear registers first for a clean initialization
  UCSR0A = 0;
  UCSR0B = 0;
  UCSR0C = 0;

  // Set baud rate
  UBRR0H = (uint8_t)(MYUBRR >> 8);
  UBRR0L = (uint8_t)MYUBRR;

  // Enable transmitter, receiver and RX interrupt
  UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

  // Set frame format: 8 data bits, 1 stop bit, no parity
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void timer1_init()
{
  // Clear registers first
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  TCNT1 = 0; // Reset counter

  // Set CTC mode
  TCCR1B = (1 << WGM12);

  // Set compare match register for 1s delay with 16MHz clock and 1024 prescaler
  OCR1A = 15624;

  // Enable Timer1 compare interrupt
  TIMSK1 = (1 << OCIE1A);

  // Set prescaler 1024 and start timer
  TCCR1B |= (1 << CS12) | (1 << CS10);
}

// Timer0 for RX timeout detection
void timer0_init()
{
  // Clear registers first
  TCCR0A = 0;
  TCCR0B = 0;
  TIMSK0 = 0;
  TCNT0 = 0; // Reset counter

  // Set CTC mode
  TCCR0A = (1 << WGM01);

  // Set compare match register for ~16ms at 16MHz with 1024 prescaler
  OCR0A = 250;

  // Enable Timer0 compare interrupt
  TIMSK0 = (1 << OCIE0A);

  // Set prescaler 1024 and start timer
  TCCR0B = (1 << CS02) | (1 << CS00);
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