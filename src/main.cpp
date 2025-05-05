#include <Arduino.h>

// Circular buffers for UART communication
volatile uint8_t tx_buffer[256];
volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;
volatile uint8_t timer_flag = 0;

// RX buffer for data reception
volatile uint8_t rx_buffer[256];
volatile uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0;
volatile uint8_t rx_timeout = 0;

void setup() {
  // Initialize UART
  Serial.begin(9600);
  
  // Timer1 for 1-second interval
  noInterrupts();
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  // Set compare match register for 1 Hz
  OCR1A = 15624; // = 16MHz/1024/1Hz - 1
  
  // CTC mode
  TCCR1B |= (1 << WGM12);
  
  // Prescaler 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);
  
  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  interrupts();
}

// Timer1 interrupt handler - triggers every 1 second
ISR(TIMER1_COMPA_vect) {
  timer_flag = 1;
}

void loop() {
  // 1. Send "Hello World" every second
  if (timer_flag) {
    timer_flag = 0;
    Serial.println("ATmega: Hello World!");
  }
  
  // 2. Echo back any received characters
  if (Serial.available()) {
    // Reset timeout when character received
    rx_timeout = 0;
    
    // Read and store character
    char c = Serial.read();
    rx_buffer[rx_head] = c;
    rx_head = (rx_head + 1) % 256;
    
    // Echo character back
    Serial.write(c);
    
    // Set timeout to detect end of message (handled by main loop)
    rx_timeout = 1;
  }
  
  // 3. Check for complete messages
  if (rx_timeout && rx_head != rx_tail) {
    // Print information about the received data
    uint8_t bytes_received = (rx_head >= rx_tail) ? (rx_head - rx_tail) : (256 + rx_head - rx_tail);
    Serial.print("\r\nReceived ");
    Serial.print(bytes_received);
    Serial.println(" bytes");
    
    // Reset RX buffer
    rx_tail = rx_head;
    rx_timeout = 0;
  }
}