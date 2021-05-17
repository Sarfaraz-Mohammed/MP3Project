#include "uart_lab.h"

static QueueHandle_t your_uart_rx_queue;

void uart_lab_pin__init(void) {
  // reset bits of pins
  LPC_IOCON->P2_8 &= ~(0b111);  // U2_TXD
  LPC_IOCON->P2_9 &= ~(0b111);  // U2_RXD
  LPC_IOCON->P4_28 &= ~(0b111); // U3_TXD
  LPC_IOCON->P4_29 &= ~(0b111); // U3_RXD

  // setting registers to function 001 for U2 pins
  LPC_IOCON->P2_8 |= (0b001); // U2_TXD
  LPC_IOCON->P2_9 |= (0b001); // U2_RXD

  // setting registers to function 010 for U3 pins
  LPC_IOCON->P4_28 |= (0b010); // U3_TXD
  LPC_IOCON->P4_29 |= (0b010); // U3_RXD
}

void uart_lab_init_baud_rate(uart_number_e uart, uint32_t baud_rate) {
  if (UART_2 == uart) {
    LPC_UART2->FDR = (0 << 0) | (1 << 4); // setting divaddval and mulval to disable fractional divider registers

    // Baud Rate Equation: 96 / (16 * (DLM * 256 + DLL))
    // Baud = 96 / (16 * (DLM << 8 | DLL))
    // Baud = 96 / (16 * value_16bit)

    const uint16_t divider_16bit = 96000000 / (16 * baud_rate);
    LPC_UART2->DLL = (divider_16bit & 0xFF);
    LPC_UART2->DLM = (divider_16bit >> 8) & 0xFF;
  } else {
    LPC_UART3->FDR = (0 << 0) | (1 << 4); // setting divaddval and mulval to disable fractional divider register
    const uint16_t divider_16bit = 96000000 / (16 * baud_rate);
    LPC_UART3->DLL = (divider_16bit & 0xFF);
    LPC_UART3->DLM = (divider_16bit >> 8) & 0xFF;
  }
}

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  uart_lab_pin__init();
  if (UART_2 == uart) {
    LPC_SC->PCONP |= (1 << 24); // power on UART2 using bit 24
    const uint8_t dlab_bit = (1 << 7);
    LPC_UART2->LCR = dlab_bit; // enabling Divsor Latch Access Bit using bit 7 for Baud rate
    uart_lab_init_baud_rate(uart, baud_rate);
    LPC_UART2->LCR &= ~dlab_bit; // disabling DLAB bit

    LPC_UART2->LCR = (0x3 << 0); // 8bit communication
  } else {
    LPC_SC->PCONP |= (1 << 25); // power on UART3 using bit 25
    const uint8_t dlab_bit = (1 << 7);
    LPC_UART3->LCR = dlab_bit; // enabling Divsor Latch Access Bit using bit 7 for Baud rate
    uart_lab_init_baud_rate(uart, baud_rate);
    LPC_UART3->LCR &= ~dlab_bit; // disabling DLAB bit

    LPC_UART3->LCR = (0x3 << 0); // 8bit communication
  }
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  bool status = false;
  if (UART_3 == uart) {
    const uint8_t receive_data_ready = (1 << 0); // setting Receive data ready to empty using bit 0
    // wait for receive data ready if not empty
    while (!(LPC_UART3->LSR) & (receive_data_ready)) {
      ;
    }
    printf("Setting RBR: %c\n", *input_byte);
    *input_byte = LPC_UART3->RBR; // reading byte from RBR and saving it to pointer
    status = true;
  } else {
    const uint8_t receive_data_ready = (1 << 0);
    // wait for receive data ready if not empty
    while (!(LPC_UART2->LSR) & (receive_data_ready)) {
      ;
    }
    printf("Setting RBR: %c\n", *input_byte);
    *input_byte = LPC_UART2->RBR;
    status = true;
  }
  return status;
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  bool status = false;

  if (UART_3 == uart) {
    const uint8_t transmit_hold_register = (1 << 5); // setting transmit hold register to empty using bit 5
    // wait for transmit_hold_register if not empty
    while (!(LPC_UART3->LSR) & (transmit_hold_register)) {
      ;
    }
    LPC_UART3->THR = output_byte; // copying output byte to THR register
    status = true;
  } else {
    const uint8_t transmit_hold_register = (1 << 5);

    while (!(LPC_UART2->LSR) & (transmit_hold_register)) {
      ;
    }
    LPC_UART2->THR = output_byte;
    status = true;
  }
  return status;
}

// Public function to get a char from the queue (this function should work without modification)
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}

/*******************************************************************************
 *
 *                     P R I V A T E    F U N C T I O N S
 *
 ******************************************************************************/

static void your_receive_interrupt(void) {
  // reading IIR register for UART and confirming interrupt
  // if no IIR interrupt is pending and receive data available
  if (((LPC_UART3->IIR >> 1) & 0xF) == 0x2) {
    // Reading LSR register to confirm empty and wait
    while (!(LPC_UART3->LSR & (1 << 0))) {
      ;
    }
  }
  const char byte = LPC_UART3->RBR;                   // reading rbr register
  xQueueSendFromISR(your_uart_rx_queue, &byte, NULL); // inputing data to RX queue
}

void uart__enable_receive_interrupt(uart_number_e uart_number) {
  if (UART_3 == uart_number) {
    NVIC_EnableIRQ(UART3_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, your_receive_interrupt, NULL);
    LPC_UART3->LCR &= ~(1 << 7);                         // disabling DLAB bit
    LPC_UART3->IER = (1 << 0) | (1 << 1) | (1 << 2);     // enabling UART receive interrupt through IER register
    your_uart_rx_queue = xQueueCreate(16, sizeof(char)); // TODO: Create your RX queue
  } else {
    NVIC_EnableIRQ(UART2_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, your_receive_interrupt, NULL);
    LPC_UART2->LCR &= ~(1 << 7); // disabling DLAB bit
    LPC_UART2->IER = (1 << 0) | (1 << 1) | (1 << 2);
    your_uart_rx_queue = xQueueCreate(16, sizeof(char));
  }
}
