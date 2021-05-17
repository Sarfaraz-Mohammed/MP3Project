#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdint.h>
#include <stdio.h>

void ssp2_pins_init(void);

void adesto_cs(void);
void adesto_ds(void);

void ssp2__init(uint32_t max_clock_mhz);

uint8_t ssp2__exchange_byte_new(uint8_t data_out);