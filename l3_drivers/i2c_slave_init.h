#pragma once

#include "FreeRTOS.h"
#include "clock.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "i2c.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "sj2_cli.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void i2c1__slave_init(uint8_t slave_address_to_respond_to);