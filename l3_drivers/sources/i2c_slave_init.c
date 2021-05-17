#include "i2c_slave_init.h"

void i2c1__slave_init(uint8_t slave_address_to_respond_to) {
  const uint32_t i2c_speed = UINT32_C(400) * 1000;
  const uint16_t pin_function = (1 << 10);
  LPC_IOCON->P0_0 |= pin_function;
  LPC_IOCON->P0_1 |= pin_function;

  gpio__construct_with_function(0, 0, 3); // SDA at P0.0
  gpio__construct_with_function(0, 1, 3); // SCL at P0.1
  i2c__initialize(I2C__1, i2c_speed, clock__get_peripheral_clock_hz());

  LPC_I2C1->ADR0 |= (slave_address_to_respond_to << 0);
  LPC_I2C1->CONSET = 0x44;
}
