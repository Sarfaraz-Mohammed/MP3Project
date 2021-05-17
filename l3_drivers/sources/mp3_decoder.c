#include "mp3_decoder.h"

// DREQ
bool get_DREQ_high() { return LPC_GPIO2->PIN & (1 << 0) ? true : false; }

// CS
void high_CS() { LPC_GPIO2->SET |= (1 << 1); }
void low_CS() { LPC_GPIO2->CLR |= (1 << 1); }

// XDCS
void high_xdcs() { LPC_GPIO2->SET |= (1 << 2); }
void low_xdcs() { LPC_GPIO2->CLR |= (1 << 2); }

// RESET
void high_reset() { LPC_GPIO2->SET |= (1 << 4); }
void low_reset() { LPC_GPIO2->CLR |= (1 << 4); }

// Decoder SPI0 Initialization
static void decoder_ssp0__init(uint32_t SPI_clock) {
  SPI_clock = SPI_clock * 1000 * 1000;
  // Power On
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);

  // Set Control Registers
  LPC_SSP0->CR0 = 7;        // 8 Bit transfer
  LPC_SSP0->CR1 = (1 << 1); // Enable SSP Control

  // Set Prescalar Register
  const uint32_t CPU_clk = clock__get_core_clock_hz(); // 96 MHz
  for (uint8_t divider = 2; divider <= 254; divider += 2) {
    if ((CPU_clk / divider) <= SPI_clock) {
      break;
    }
    LPC_SSP0->CPSR = divider;
  }
}

// Clock Max
static void decoder_clock_max(uint32_t SPI_clock) { SPI_clock = SPI_clock * 1000 * 1000; }

// Pin Initialization
static void decoder_ssp0_PIN() {
  // SPI0
  gpio__construct_with_function(0, 15, GPIO__FUNCTION_2); // CLK
  gpio__construct_with_function(0, 17, GPIO__FUNCTION_2); // MISO
  gpio__construct_with_function(0, 18, GPIO__FUNCTION_2); // MOSI

  // GPIO
  gpio__construct_with_function(2, 0, GPIO__FUNCITON_0_IO_PIN);
  gpio2__set_as_input(0); // DREQ

  gpio__construct_with_function(2, 1, GPIO__FUNCITON_0_IO_PIN);
  gpio2__set_as_output(1); // CS

  gpio__construct_with_function(2, 2, GPIO__FUNCITON_0_IO_PIN);
  gpio2__set_as_output(2); // DCS

  gpio__construct_with_function(2, 4, GPIO__FUNCITON_0_IO_PIN);
  gpio2__set_as_output(4); // RESET
}

// Data Transfer
static uint8_t decoder_ssp0_transfer(uint8_t transfer_data) {

  LPC_SSP0->DR = transfer_data; // 16 bits of data

  // Status Register Busy
  while (LPC_SSP0->SR & (1 << 4)) {
    ; // Wait
  }
  return (uint8_t)(LPC_SSP0->DR & 0xFF); // Read the data
}

// Write to Decoder
void write_decoder_register(uint16_t reg_addr, uint8_t MSB, uint8_t LSB) {
  while (!get_DREQ_high()) {
    ; // Wait for HIGH
  }
  low_CS();

  // Transfer data byte using register address and WRITE OP Code
  decoder_ssp0_transfer(0x02);
  decoder_ssp0_transfer(reg_addr);

  // Write MSB and LSB
  decoder_ssp0_transfer(MSB);
  decoder_ssp0_transfer(LSB);

  while (!get_DREQ_high()) {
    ; // Wait
  }
  high_CS();
}

// Read from Decoder
uint16_t read_decoder_register(uint16_t reg_addr) {
  while (!get_DREQ_high()) {
    ; // wait
  }
  low_CS();

  // Read byte from register address with READ OP Code
  decoder_ssp0_transfer(0x03);
  decoder_ssp0_transfer(reg_addr);

  uint8_t first_byte = decoder_ssp0_transfer(0xFF);
  while (!get_DREQ_high()) {
    ; // wait for DREQ to go high indicating command is complete
  }
  uint8_t second_byte = decoder_ssp0_transfer(0xFF);
  while (!get_DREQ_high()) {
    ; // wait
  }
  high_CS();

  uint16_t value = 0;
  value |= ((first_byte << 8) | (second_byte << 0));
  return value;
}

// MP3 Data Byte sending
void send_data_decoder(uint8_t data) {
  low_xdcs();
  decoder_ssp0_transfer(data);
  high_xdcs();
}

// Decoder Initialization
void decoder_initialize() {
  decoder_ssp0_PIN();
  high_reset();

  // SPI0 with 1 MHz CLK
  decoder_ssp0__init(1);

  // Dummy Byte being sent
  decoder_ssp0_transfer(0xFF);
  high_CS();
  high_xdcs();

  // Set Volume, Bass, Treble
  write_decoder_register(SCI_VOL, 10, 10);
  set_bass(1);
  set_treble(1);

  // Decoder Status and Mode Setup
  uint16_t mp3_status = read_decoder_register(SCI_STATUS);
  int version = (mp3_status >> 4) & 0x000F;
  printf("VS1053 Version %d\n", version);

  uint16_t mp3_mode = read_decoder_register(SCI_MODE);
  printf("SCI_MODE = 0x%x\n", mp3_mode);

  // Increase Clock Multiplier
  write_decoder_register(SCI_CLOCKF, 0x60, 0x00);

  // SPI Speed for Bus
  decoder_clock_max(4);

  uint16_t mp3_clock = read_decoder_register(SCI_CLOCKF);
  printf("SCI_CLK = 0x%x\n", mp3_clock);
}

