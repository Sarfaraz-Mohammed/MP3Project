#include "ssp2_lab.h"

void adesto_cs(void) { LPC_GPIO1->CLR = (1 << 10); } // activate -- active LOW
void adesto_ds(void) { LPC_GPIO1->SET = (1 << 10); } // deactivate -- HIGH

void ssp2_pins_init(void) {
  // reset bits of pins
  LPC_IOCON->P1_0 &= ~(0b111);  // SCK2
  LPC_IOCON->P1_1 &= ~(0b111);  // MOSI2
  LPC_IOCON->P1_4 &= ~(0b111);  // MISO2
  LPC_IOCON->P1_14 &= ~(0b111); // CS Flash

  // setting registers to function 100 for ssp2
  LPC_IOCON->P1_0 |= (0b100); // SCK2
  LPC_IOCON->P1_1 |= (0b100); // MOSI2
  LPC_IOCON->P1_4 |= (0b100); // MISO2
}

void ssp2__init(uint32_t max_clock_mhz) {
  ssp2_pins_init();
  LPC_SC->PCONP |= (1 << 20);                 // bit 20 to power on ssp2
  LPC_SSP2->CR0 = (0b111 << 0) | (0b00 << 4); // 8 bit data transfer for CR0 or SCR set to 0
  LPC_SSP2->CR1 = (0b1 << 1);                 // SSP enable
  LPC_SSP2->CPSR = 96 / max_clock_mhz; // prescalar register set to max clock mhz (PCLK / CPSDVSR) -> 96 / 24 = 4 MHz

  gpio1__set_as_output(10); // direction of CS set to output
  adesto_ds();              // CS set to HIGH to deactivate SPI signal
}

uint8_t ssp2__exchange_byte_new(uint8_t data_out) {
  LPC_SSP2->DR = data_out;

  // status register busy set to bit 4
  while (LPC_SSP2->SR & (1 << 4)) {
    ; // waiting for SSP to transfer all bits
  }
  return LPC_SSP2->DR;
}
